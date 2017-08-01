#pragma once

#include "Array.h"
#include "Set.h"
#include "../Core/Template.h"
#include "../Core/TypeHash.h"
#include "Algorithm.h"

namespace EDX
{
	/** An initializer type for pairs that's passed to the pair set when adding a new pair. */
	template <typename KeyInitType, typename ValueInitType>
	class PairInitializer
	{
	public:
		typename RValueToLValueReference<KeyInitType  >::Type Key;
		typename RValueToLValueReference<ValueInitType>::Type Value;

		/** Initialization constructor. */
		__forceinline PairInitializer(KeyInitType InKey, ValueInitType InValue)
			: Key(InKey)
			, Value(InValue)
		{
		}
	};

	/** An initializer type for keys that's passed to the pair set when adding a new key. */
	template <typename KeyInitType>
	class KeyInitializer
	{
	public:
		typename RValueToLValueReference<KeyInitType>::Type Key;

		/** Initialization constructor. */
		__forceinline explicit KeyInitializer(KeyInitType InKey)
			: Key(InKey)
		{
		}
	};

	/** A key-value pair in the map. */
	template<typename KeyType, typename ValueType>
	class Pair
	{
	public:
		typedef typename TypeTraits<KeyType  >::ConstInitType KeyInitType;
		typedef typename TypeTraits<ValueType>::ConstInitType ValueInitType;

		KeyType   Key;
		ValueType Value;

		/** Initialization constructor. */
		template <typename InitKeyType, typename InitValueType>
		__forceinline Pair(const PairInitializer<InitKeyType, InitValueType>& InInitializer)
			: Key(StaticCast<InitKeyType  >(InInitializer.Key))
			, Value(StaticCast<InitValueType>(InInitializer.Value))
		{
			// The seemingly-pointless casts above are to enforce a move (i.e. equivalent to using Move) when
			// the initializers are themselves rvalue references.
		}

		/** Key initialization constructor. */
		template <typename InitKeyType>
		__forceinline explicit Pair(const KeyInitializer<InitKeyType>& InInitializer)
			: Key(StaticCast<InitKeyType>(InInitializer.Key))
			, Value()
		{
			// The seemingly-pointless cast above is to enforce a move (i.e. equivalent to using Move) when
			// the initializer is itself an rvalue reference.
		}

		__forceinline Pair() = default;
		__forceinline Pair(Pair&&) = default;
		__forceinline Pair(const Pair&) = default;
		__forceinline Pair& operator=(Pair&&) = default;
		__forceinline Pair& operator=(const Pair&) = default;

		///** Serializer. */
		//__forceinline friend FArchive& operator<<(FArchive& Ar, Pair& Pair)
		//{
		//	return Ar << Pair.Key << Pair.Value;
		//}

		// Comparison operators
		__forceinline bool operator==(const Pair& Other) const
		{
			return Key == Other.Key && Value == Other.Value;
		}

		__forceinline bool operator!=(const Pair& Other) const
		{
			return Key != Other.Key || Value != Other.Value;
		}

		/** Implicit conversion to pair initializer. */
		__forceinline operator PairInitializer<KeyInitType, ValueInitType>() const
		{
			return PairInitializer<KeyInitType, ValueInitType>(Key, Value);
		}
	};

	/** Defines how the map's pairs are hashed. */
	template<typename KeyType, typename ValueType, bool bInAllowDuplicateKeys>
	struct DefaultMapKeyFuncs : BaseKeyFuncs<Pair<KeyType, ValueType>, KeyType, bInAllowDuplicateKeys>
	{
		typedef typename TypeTraits<KeyType>::ConstPointerType KeyInitType;
		typedef const PairInitializer<typename TypeTraits<KeyType>::ConstInitType, typename TypeTraits<ValueType>::ConstInitType>& ElementInitType;

		static __forceinline KeyInitType GetSetKey(ElementInitType Element)
		{
			return Element.Key;
		}
		static __forceinline bool Matches(KeyInitType A, KeyInitType B)
		{
			return A == B;
		}
		static __forceinline uint32 GetKeyHash(KeyInitType Key)
		{
			return GetTypeHash(Key);
		}
	};

	/**
	* The base class of maps from keys to values.  Implemented using a Set of key-value pairs with a custom KeyFuncs,
	* with the same O(1) addition, removal, and finding.
	**/
	template <typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
	class MapBase
	{
		template <typename OtherKeyType, typename OtherValueType, typename OtherSetAllocator, typename OtherKeyFuncs>
		friend class MapBase;

		friend struct ContainerTraits<MapBase>;

	public:
		typedef typename TypeTraits<KeyType  >::ConstPointerType KeyConstPointerType;
		typedef typename TypeTraits<KeyType  >::ConstInitType    KeyInitType;
		typedef typename TypeTraits<ValueType>::ConstInitType    ValueInitType;

	protected:
		typedef Pair<KeyType, ValueType> PairType;

		MapBase() = default;
		MapBase(MapBase&&) = default;
		MapBase(const MapBase&) = default;
		MapBase& operator=(MapBase&&) = default;
		MapBase& operator=(const MapBase&) = default;

		/** Constructor for moving elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		MapBase(MapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
			: Pairs(Move(Other.Pairs))
		{
		}

		/** Constructor for copying elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		MapBase(const MapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
			: Pairs(Other.Pairs)
		{
		}

		/** Assignment operator for moving elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		MapBase& operator=(MapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
		{
			Pairs = Move(Other.Pairs);
			return *this;
		}

		/** Assignment operator for copying elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		MapBase& operator=(const MapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
		{
			Pairs = Other.Pairs;
			return *this;
		}

	public:
		// Legacy comparison operators.  Note that these also test whether the map's key-value pairs were added in the same order!
		friend bool LegacyCompareEqual(const MapBase& A, const MapBase& B)
		{
			return LegacyCompareEqual(A.Pairs, B.Pairs);
		}
		friend bool LegacyCompareNotEqual(const MapBase& A, const MapBase& B)
		{
			return LegacyCompareNotEqual(A.Pairs, B.Pairs);
		}

		/**
		* Compare this map with another for equality. Does not make any assumptions about Key order.
		* NOTE: this might be a candidate for operator== but it was decided to make it an explicit function
		*  since it can potentially be quite slow.
		*
		* @param Other The other map to compare against
		* @returns True if both this and Other contain the same keys with values that compare ==
		*/
		bool OrderIndependentCompareEqual(const MapBase& Other) const
		{
			// first check counts (they should be the same obviously)
			if (Size() != Other.Size())
			{
				return false;
			}

			// since we know the counts are the same, we can just iterate one map and check for existence in the other
			for (typename PairSetType::ConstIterator It(Pairs); It; ++It)
			{
				const ValueType* BVal = Other.Find(It->Key);
				if (BVal == nullptr)
				{
					return false;
				}
				if (!(*BVal == It->Value))
				{
					return false;
				}
			}

			// all fields in A match B and A and B's counts are the same (so there can be no fields in B not in A)
			return true;
		}

		/**
		* Removes all elements from the map, potentially leaving space allocated for an expected number of elements about to be added.
		* @param ExpectedNumElements - The number of elements about to be added to the set.
		*/
		__forceinline void Clear(int32 ExpectedNumElements = 0)
		{
			Pairs.Clear(ExpectedNumElements);
		}

		/** Efficiently empties out the map but preserves all allocations and capacities */
		__forceinline void Reset()
		{
			Pairs.Reset();
		}

		/** Shrinks the pair set to avoid slack. */
		__forceinline void Shrink()
		{
			Pairs.Shrink();
		}

		/** Compacts the pair set to remove holes */
		__forceinline void Compact()
		{
			Pairs.Compact();
		}

		/** Compacts the pair set to remove holes. Does not change the iteration order of the elements. */
		__forceinline void CompactStable()
		{
			Pairs.CompactStable();
		}

		/** Preallocates enough memory to contain Number elements */
		__forceinline void Reserve(int32 Number)
		{
			Pairs.Reserve(Number);
		}

		/** @return The number of elements in the map. */
		__forceinline int32 Size() const
		{
			return Pairs.Size();
		}

		/**
		* Returns the unique keys contained within this map
		* @param	OutKeys	- Upon return, contains the set of unique keys in this map.
		* @return The number of unique keys in the map.
		*/
		template<typename Allocator> int32 GetKeys(Array<KeyType, Allocator>& OutKeys) const
		{
			Set<KeyType> VisitedKeys;
			for (typename PairSetType::ConstIterator It(Pairs); It; ++It)
			{
				if (!VisitedKeys.Contains(It->Key))
				{
					OutKeys.Add(It->Key);
					VisitedKeys.Add(It->Key);
				}
			}
			return OutKeys.Size();
		}

		/**
		* Helper function to return the amount of memory allocated by this container
		* @return number of bytes allocated by this container
		*/
		__forceinline uint32 GetAllocatedSize() const
		{
			return Pairs.GetAllocatedSize();
		}

		///** Tracks the container's memory use through an archive. */
		//__forceinline void CountBytes(FArchive& Ar)
		//{
		//	Pairs.CountBytes(Ar);
		//}

		/**
		* Sets the value associated with a key.
		*
		* @param InKey - The key to associate the value with.
		* @param InValue - The value to associate with the key.
		* @return A reference to the value as stored in the map.  The reference is only valid until the next change to any key in the map.
		*/
		__forceinline ValueType& Add(const KeyType&  InKey, const ValueType&  InValue) { return Emplace(InKey, InValue); }
		__forceinline ValueType& Add(const KeyType&  InKey, ValueType&& InValue) { return Emplace(InKey, Move(InValue)); }
		__forceinline ValueType& Add(KeyType&& InKey, const ValueType&  InValue) { return Emplace(Move(InKey), InValue); }
		__forceinline ValueType& Add(KeyType&& InKey, ValueType&& InValue) { return Emplace(Move(InKey), Move(InValue)); }

		/**
		* Sets a default value associated with a key.
		*
		* @param InKey - The key to associate the value with.
		* @return A reference to the value as stored in the map.  The reference is only valid until the next change to any key in the map.
		*/
		__forceinline ValueType& Add(const KeyType&  InKey) { return Emplace(InKey); }
		__forceinline ValueType& Add(KeyType&& InKey) { return Emplace(Move(InKey)); }

		/**
		* Sets the value associated with a key.
		*
		* @param InKey - The key to associate the value with.
		* @param InValue - The value to associate with the key.
		* @return A reference to the value as stored in the map.  The reference is only valid until the next change to any key in the map.
		*/
		template <typename InitKeyType, typename InitValueType>
		ValueType& Emplace(InitKeyType&& InKey, InitValueType&& InValue)
		{
			const SetElementId PairId = Pairs.Emplace(PairInitializer<InitKeyType&&, InitValueType&&>(Forward<InitKeyType>(InKey), Forward<InitValueType>(InValue)));

			return Pairs[PairId].Value;
		}

		/**
		* Sets a default value associated with a key.
		*
		* @param InKey - The key to associate the value with.
		* @return A reference to the value as stored in the map.  The reference is only valid until the next change to any key in the map.
		*/
		template <typename InitKeyType>
		ValueType& Emplace(InitKeyType&& InKey)
		{
			const SetElementId PairId = Pairs.Emplace(KeyInitializer<InitKeyType&&>(Forward<InitKeyType>(InKey)));

			return Pairs[PairId].Value;
		}

		/**
		* Removes all value associations for a key.
		* @param InKey - The key to remove associated values for.
		* @return The number of values that were associated with the key.
		*/
		__forceinline int32 Remove(KeyConstPointerType InKey)
		{
			const int32 NumRemovedPairs = Pairs.Remove(InKey);
			return NumRemovedPairs;
		}

		/**
		* Returns the key associated with the specified value.  The time taken is O(N) in the number of pairs.
		* @param	Value - The value to search for
		* @return	A pointer to the key associated with the specified value, or nullptr if the value isn't contained in this map.  The pointer
		*			is only valid until the next change to any key in the map.
		*/
		const KeyType* FindKey(ValueInitType Value) const
		{
			for (typename PairSetType::ConstIterator PairIt(Pairs); PairIt; ++PairIt)
			{
				if (PairIt->Value == Value)
				{
					return &PairIt->Key;
				}
			}
			return nullptr;
		}

		/**
		* Returns the value associated with a specified key.
		* @param	Key - The key to search for.
		* @return	A pointer to the value associated with the specified key, or nullptr if the key isn't contained in this map.  The pointer
		*			is only valid until the next change to any key in the map.
		*/
		__forceinline ValueType* Find(KeyConstPointerType Key)
		{
			if (auto* Pair = Pairs.Find(Key))
			{
				return &Pair->Value;
			}

			return nullptr;
		}
		__forceinline const ValueType* Find(KeyConstPointerType Key) const
		{
			return const_cast<MapBase*>(this)->Find(Key);
		}

	private:
		/**
		* Returns the value associated with a specified key, or if none exists,
		* adds a value using the default constructor.
		* @param	Key - The key to search for.
		* @return	A reference to the value associated with the specified key.
		*/
		template <typename ArgType>
		__forceinline ValueType& FindOrAddImpl(ArgType&& Arg)
		{
			if (auto* Pair = Pairs.Find(Arg))
				return Pair->Value;

			return Add(Forward<ArgType>(Arg));
		}

	public:
		/**
		* Returns the value associated with a specified key, or if none exists,
		* adds a value using the default constructor.
		* @param	Key - The key to search for.
		* @return	A reference to the value associated with the specified key.
		*/
		__forceinline ValueType& FindOrAdd(const KeyType&  Key) { return FindOrAddImpl(Key); }
		__forceinline ValueType& FindOrAdd(KeyType&& Key) { return FindOrAddImpl(Move(Key)); }

		/**
		* Returns the value associated with a specified key, or if none exists,
		* adds a value using the key as the constructor parameter.
		* @param	Key - The key to search for.
		* @return	A reference to the value associated with the specified key.
		*/
		//@todo  merge - this prevents FConfigCacheIni from compiling
		/*ValueType& FindOrAddKey(KeyInitType Key)
		{
		Pair* Pair = Pairs.Find(Key);
		if( Pair )
		{
		return Pair->Value;
		}
		else
		{
		return Set(Key, ValueType(Key));
		}
		}*/

		/**
		* Returns a reference to the value associated with a specified key.
		* @param	Key - The key to search for.
		* @return	The value associated with the specified key, or triggers an assertion if the key does not exist.
		*/
		__forceinline const ValueType& FindChecked(KeyConstPointerType Key) const
		{
			const auto* Pair = Pairs.Find(Key);
			Assert(Pair != nullptr);
			return Pair->Value;
		}

		/**
		* Returns a reference to the value associated with a specified key.
		* @param	Key - The key to search for.
		* @return	The value associated with the specified key, or triggers an assertion if the key does not exist.
		*/
		__forceinline ValueType& FindChecked(KeyConstPointerType Key)
		{
			auto* Pair = Pairs.Find(Key);
			Assert(Pair != nullptr);
			return Pair->Value;
		}

		/**
		* Returns the value associated with a specified key.
		* @param	Key - The key to search for.
		* @return	The value associated with the specified key, or the default value for the ValueType if the key isn't contained in this map.
		*/
		__forceinline ValueType FindRef(KeyConstPointerType Key) const
		{
			if (const auto* Pair = Pairs.Find(Key))
			{
				return Pair->Value;
			}

			return ValueType();
		}

		/**
		* Checks if map contains the specified key.
		* @param Key - The key to check for.
		* @return true if the map contains the key.
		*/
		__forceinline bool Contains(KeyConstPointerType Key) const
		{
			return Pairs.Contains(Key);
		}

		/**
		* Generates an array from the keys in this map.
		*/
		template<typename Allocator> void GenerateKeyArray(Array<KeyType, Allocator>& OutArray) const
		{
			OutArray.Clear(Pairs.Size());
			for (typename PairSetType::ConstIterator PairIt(Pairs); PairIt; ++PairIt)
			{
				new(OutArray) KeyType(PairIt->Key);
			}
		}

		/**
		* Generates an array from the values in this map.
		*/
		template<typename Allocator> void GenerateValueArray(Array<ValueType, Allocator>& OutArray) const
		{
			OutArray.Clear(Pairs.Size());
			for (typename PairSetType::ConstIterator PairIt(Pairs); PairIt; ++PairIt)
			{
				new(OutArray) ValueType(PairIt->Value);
			}
		}

		///** Serializer. */
		//__forceinline friend FArchive& operator<<(FArchive& Ar, MapBase& Map)
		//{
		//	return Ar << Map.Pairs;
		//}

		///**
		//* Describes the map's contents through an output device.
		//* @param Ar - The output device to describe the map's contents through.
		//*/
		//void Dump(FOutputDevice& Ar)
		//{
		//	Pairs.Dump(Ar);
		//}

	protected:

		typedef Set<PairType, KeyFuncs, SetAllocator> PairSetType;

		/** The base of MapBase iterators. */
		template<bool bConst>
		class BaseIterator
		{
		public:
			typedef typename ChooseClass<bConst, typename PairSetType::ConstIterator, typename PairSetType::Iterator>::Result PairItType;
		private:
			typedef typename ChooseClass<bConst, const MapBase, MapBase>::Result MapType;
			typedef typename ChooseClass<bConst, const KeyType, KeyType>::Result ItKeyType;
			typedef typename ChooseClass<bConst, const ValueType, ValueType>::Result ItValueType;
			typedef typename ChooseClass<bConst, const typename PairSetType::ElementType, typename PairSetType::ElementType>::Result PairType;

		protected:
			__forceinline BaseIterator(const PairItType& InElementIt)
				: PairIt(InElementIt)
			{
			}

		public:
			__forceinline BaseIterator& operator++()
			{
				++PairIt;
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			__forceinline explicit operator bool() const
			{
				return !!PairIt;
			}
			/** inverse of the "bool" operator */
			__forceinline bool operator !() const
			{
				return !(bool)*this;
			}

			__forceinline friend bool operator==(const BaseIterator& Lhs, const BaseIterator& Rhs) { return Lhs.PairIt == Rhs.PairIt; }
			__forceinline friend bool operator!=(const BaseIterator& Lhs, const BaseIterator& Rhs) { return Lhs.PairIt != Rhs.PairIt; }

			__forceinline ItKeyType&   Key()   const { return PairIt->Key; }
			__forceinline ItValueType& Value() const { return PairIt->Value; }

			__forceinline PairType& operator* () const { return  *PairIt; }
			__forceinline PairType* operator->() const { return &*PairIt; }

		protected:
			PairItType PairIt;
		};

		/** The base type of iterators that iterate over the values associated with a specified key. */
		template<bool bConst>
		class BaseKeyIterator
		{
		private:
			typedef typename ChooseClass<bConst, typename PairSetType::ConstKeyIterator, typename PairSetType::KeyIterator>::Result SetItType;
			typedef typename ChooseClass<bConst, const KeyType, KeyType>::Result ItKeyType;
			typedef typename ChooseClass<bConst, const ValueType, ValueType>::Result ItValueType;

		public:
			/** Initialization constructor. */
			__forceinline BaseKeyIterator(const SetItType& InSetIt)
				: SetIt(InSetIt)
			{
			}

			__forceinline BaseKeyIterator& operator++()
			{
				++SetIt;
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			__forceinline explicit operator bool() const
			{
				return !!SetIt;
			}

			/** inverse of the "bool" operator */
			__forceinline bool operator !() const
			{
				return !(bool)*this;
			}

			__forceinline ItKeyType&   Key() const { return SetIt->Key; }
			__forceinline ItValueType& Value() const { return SetIt->Value; }

		protected:
			SetItType SetIt;
		};

		/** A set of the key-value pairs in the map. */
		PairSetType Pairs;

	public:

		/** Map iterator. */
		class Iterator : public BaseIterator<false>
		{
		public:

			/** Initialization constructor. */
			__forceinline Iterator(MapBase& InMap, bool bInRequiresRehashOnRemoval = false)
				: BaseIterator<false>(begin(InMap.Pairs))
				, Map(InMap)
				, bElementsHaveBeenRemoved(false)
				, bRequiresRehashOnRemoval(bInRequiresRehashOnRemoval)
			{
			}

			/** Initialization constructor. */
			__forceinline Iterator(MapBase& InMap, const typename BaseIterator<false>::PairItType& InPairIt)
				: BaseIterator<false>(InPairIt)
				, Map(InMap)
				, bElementsHaveBeenRemoved(false)
				, bRequiresRehashOnRemoval(false)
			{
			}

			/** Destructor. */
			__forceinline ~Iterator()
			{
				if (bElementsHaveBeenRemoved && bRequiresRehashOnRemoval)
				{
					Map.Pairs.Relax();
				}
			}

			/** Removes the current pair from the map. */
			__forceinline void RemoveCurrent()
			{
				BaseIterator<false>::PairIt.RemoveCurrent();
				bElementsHaveBeenRemoved = true;
			}

		private:
			MapBase& Map;
			bool      bElementsHaveBeenRemoved;
			bool      bRequiresRehashOnRemoval;
		};

		/** Const map iterator. */
		class ConstIterator : public BaseIterator<true>
		{
		public:
			__forceinline ConstIterator(const MapBase& InMap)
				: BaseIterator<true>(begin(InMap.Pairs))
			{
			}

			__forceinline ConstIterator(const typename BaseIterator<true>::PairItType& InPairIt)
				: BaseIterator<true>(InPairIt)
			{
			}
		};

		/** Iterates over values associated with a specified key in a const map. */
		class ConstKeyIterator : public BaseKeyIterator<true>
		{
		public:
			__forceinline ConstKeyIterator(const MapBase& InMap, KeyInitType InKey)
				: BaseKeyIterator<true>(typename PairSetType::ConstKeyIterator(InMap.Pairs, InKey))
			{}
		};

		/** Iterates over values associated with a specified key in a map. */
		class KeyIterator : public BaseKeyIterator<false>
		{
		public:
			__forceinline KeyIterator(MapBase& InMap, KeyInitType InKey)
				: BaseKeyIterator<false>(typename PairSetType::KeyIterator(InMap.Pairs, InKey))
			{}

			/** Removes the current key-value pair from the map. */
			__forceinline void RemoveCurrent()
			{
				BaseKeyIterator<false>::SetIt.RemoveCurrent();
			}
		};

		/** Creates an iterator over all the pairs in this map */
		__forceinline Iterator CreateIterator()
		{
			return Iterator(*this);
		}

		/** Creates a const iterator over all the pairs in this map */
		__forceinline ConstIterator CreateConstIterator() const
		{
			return ConstIterator(*this);
		}

		/** Creates an iterator over the values associated with a specified key in a map */
		__forceinline KeyIterator CreateKeyIterator(KeyInitType InKey)
		{
			return KeyIterator(*this, InKey);
		}

		/** Creates a const iterator over the values associated with a specified key in a map */
		__forceinline ConstKeyIterator CreateConstKeyIterator(KeyInitType InKey) const
		{
			return ConstKeyIterator(*this, InKey);
		}

	private:
		/**
		* DO NOT USE DIRECTLY
		* STL-like iterators to enable range-based for loop support.
		*/
		__forceinline friend Iterator      begin(MapBase& MapBase) { return Iterator(MapBase, begin(MapBase.Pairs)); }
		__forceinline friend ConstIterator begin(const MapBase& MapBase) { return ConstIterator(begin(MapBase.Pairs)); }
		__forceinline friend Iterator      end(MapBase& MapBase) { return Iterator(MapBase, end(MapBase.Pairs)); }
		__forceinline friend ConstIterator end(const MapBase& MapBase) { return ConstIterator(end(MapBase.Pairs)); }
	};

	/** The base type of sortable maps. */
	template <typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
	class SortableMapBase : public MapBase<KeyType, ValueType, SetAllocator, KeyFuncs>
	{
		friend struct ContainerTraits<SortableMapBase>;

	protected:
		typedef MapBase<KeyType, ValueType, SetAllocator, KeyFuncs> Super;

		SortableMapBase() = default;
		SortableMapBase(SortableMapBase&&) = default;
		SortableMapBase(const SortableMapBase&) = default;
		SortableMapBase& operator=(SortableMapBase&&) = default;
		SortableMapBase& operator=(const SortableMapBase&) = default;

		/** Constructor for moving elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		SortableMapBase(SortableMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
			: Super(Move(Other))
		{
		}

		/** Constructor for copying elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		SortableMapBase(const SortableMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
			: Super(Other)
		{
		}

		/** Assignment operator for moving elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		SortableMapBase& operator=(SortableMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
		{
			(Super&)*this = Move(Other);
			return *this;
		}

		/** Assignment operator for copying elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		SortableMapBase& operator=(const SortableMapBase<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
		{
			(Super&)*this = Other;
			return *this;
		}

	public:
		/**
		* Sorts the pairs array using each pair's Key as the sort criteria, then rebuilds the map's hash.
		* Invoked using "MyMapVar.KeySort( PREDICATE_CLASS() );"
		*/
		template<typename PREDICATE_CLASS>
		__forceinline void KeySort(const PREDICATE_CLASS& Predicate)
		{
			Super::Pairs.Sort(KeyComparisonClass<PREDICATE_CLASS>(Predicate));
		}

		/**
		* Sorts the pairs array using each pair's Value as the sort criteria, then rebuilds the map's hash.
		* Invoked using "MyMapVar.ValueSort( PREDICATE_CLASS() );"
		*/
		template<typename PREDICATE_CLASS>
		__forceinline void ValueSort(const PREDICATE_CLASS& Predicate)
		{
			Super::Pairs.Sort(ValueComparisonClass<PREDICATE_CLASS>(Predicate));
		}

	private:

		/** Extracts the pair's key from the map's pair structure and passes it to the user provided comparison class. */
		template<typename PREDICATE_CLASS>
		class KeyComparisonClass
		{
			DereferenceWrapper< KeyType, PREDICATE_CLASS> Predicate;

		public:

			__forceinline KeyComparisonClass(const PREDICATE_CLASS& InPredicate)
				: Predicate(InPredicate)
			{}

			__forceinline bool operator()(const typename Super::PairType& A, const typename Super::PairType& B) const
			{
				return Predicate(A.Key, B.Key);
			}
		};

		/** Extracts the pair's value from the map's pair structure and passes it to the user provided comparison class. */
		template<typename PREDICATE_CLASS>
		class ValueComparisonClass
		{
			DereferenceWrapper< ValueType, PREDICATE_CLASS> Predicate;

		public:

			__forceinline ValueComparisonClass(const PREDICATE_CLASS& InPredicate)
				: Predicate(InPredicate)
			{}

			__forceinline bool operator()(const typename Super::PairType& A, const typename Super::PairType& B) const
			{
				return Predicate(A.Value, B.Value);
			}
		};
	};

	/** A MapBase specialization that only allows a single value associated with each key.*/
	template<typename KeyType, typename ValueType, typename SetAllocator = DefaultSetAllocator, typename KeyFuncs = DefaultMapKeyFuncs<KeyType, ValueType, false>>
	class Map : public SortableMapBase<KeyType, ValueType, SetAllocator, KeyFuncs>
	{
		friend struct ContainerTraits<Map>;

		static_assert(!KeyFuncs::bAllowDuplicateKeys, "Map cannot be instantiated with a KeyFuncs which allows duplicate keys");

	public:
		typedef SortableMapBase<KeyType, ValueType, SetAllocator, KeyFuncs> Super;
		typedef typename Super::KeyInitType KeyInitType;
		typedef typename Super::KeyConstPointerType KeyConstPointerType;

		Map() = default;
		Map(Map&&) = default;
		Map(const Map&) = default;
		Map& operator=(Map&&) = default;
		Map& operator=(const Map&) = default;

		/** Constructor for moving elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		Map(Map<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
			: Super(Move(Other))
		{
		}

		/** Constructor for copying elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		Map(const Map<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
			: Super(Other)
		{
		}

		/** Assignment operator for moving elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		Map& operator=(Map<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
		{
			(Super&)*this = Move(Other);
			return *this;
		}

		/** Assignment operator for copying elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		Map& operator=(const Map<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
		{
			(Super&)*this = Other;
			return *this;
		}

		/**
		* Removes the pair with the specified key and copies the value that was removed to the ref parameter
		* @param Key - the key to search for
		* @param OutRemovedValue - if found, the value that was removed (not modified if the key was not found)
		* @return whether or not the key was found
		*/
		__forceinline bool RemoveAndCopyValue(KeyInitType Key, ValueType& OutRemovedValue)
		{
			const SetElementId PairId = Super::Pairs.FindId(Key);
			if (!PairId.IsValidId())
				return false;

			OutRemovedValue = Move(Super::Pairs[PairId].Value);
			Super::Pairs.Remove(PairId);
			return true;
		}

		/**
		* Finds a pair with the specified key, removes it from the map, and returns the value part of the pair.
		* If no pair was found, an exception is thrown.
		* @param Key - the key to search for
		* @return whether or not the key was found
		*/
		__forceinline ValueType FindAndRemoveChecked(KeyConstPointerType Key)
		{
			const SetElementId PairId = Super::Pairs.FindId(Key);
			Assert(PairId.IsValidId());
			ValueType Result = Move(Super::Pairs[PairId].Value);
			Super::Pairs.Remove(PairId);
			return Result;
		}

		/**
		* Move all items from another map into our map (if any keys are in both, the value from the other map wins) and empty the other map.
		* @param OtherMap - The other map of items to move the elements from.
		*/
		template<typename OtherSetAllocator>
		void Append(Map<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& OtherMap)
		{
			this->Reserve(this->Size() + OtherMap.Size());
			for (auto& Pair : OtherMap)
			{
				this->Add(Move(Pair.Key), Move(Pair.Value));
			}

			OtherMap.Reset();
		}

		/**
		* Add all items from another map to our map (if any keys are in both, the value from the other map wins)
		* @param OtherMap - The other map of items to add.
		*/
		template<typename OtherSetAllocator>
		void Append(const Map<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& OtherMap)
		{
			this->Reserve(this->Size() + OtherMap.Size());
			for (auto& Pair : OtherMap)
			{
				this->Add(Pair.Key, Pair.Value);
			}
		}

		__forceinline       ValueType& operator[](KeyConstPointerType Key) { return this->FindChecked(Key); }
		__forceinline const ValueType& operator[](KeyConstPointerType Key) const { return this->FindChecked(Key); }
	};

	/** A MapBase specialization that allows multiple values to be associated with each key. */
	template<typename KeyType, typename ValueType, typename SetAllocator = DefaultSetAllocator, typename KeyFuncs = DefaultMapKeyFuncs<KeyType, ValueType, true>>
	class MultiMap : public SortableMapBase<KeyType, ValueType, SetAllocator, KeyFuncs>
	{
		friend struct ContainerTraits<MultiMap>;

		static_assert(KeyFuncs::bAllowDuplicateKeys, "MultiMap cannot be instantiated with a KeyFuncs which disallows duplicate keys");

	public:
		typedef SortableMapBase<KeyType, ValueType, SetAllocator, KeyFuncs> Super;
		typedef typename Super::KeyConstPointerType KeyConstPointerType;
		typedef typename Super::KeyInitType KeyInitType;
		typedef typename Super::ValueInitType ValueInitType;

		MultiMap() = default;
		MultiMap(MultiMap&&) = default;
		MultiMap(const MultiMap&) = default;
		MultiMap& operator=(MultiMap&&) = default;
		MultiMap& operator=(const MultiMap&) = default;


		/** Constructor for moving elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		MultiMap(MultiMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
			: Super(Move(Other))
		{
		}

		/** Constructor for copying elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		MultiMap(const MultiMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
			: Super(Other)
		{
		}

		/** Assignment operator for moving elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		MultiMap& operator=(MultiMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>&& Other)
		{
			(Super&)*this = Move(Other);
			return *this;
		}

		/** Assignment operator for copying elements from a Map with a different SetAllocator */
		template<typename OtherSetAllocator>
		MultiMap& operator=(const MultiMap<KeyType, ValueType, OtherSetAllocator, KeyFuncs>& Other)
		{
			(Super&)*this = Other;
			return *this;
		}

		/**
		* Finds all values associated with the specified key.
		* @param Key - The key to find associated values for.
		* @param OutValues - Upon return, contains the values associated with the key.
		* @param bMaintainOrder - true if the Values array should be in the same order as the map's pairs.
		*/
		template<typename Allocator> void MultiFind(KeyInitType Key, Array<ValueType, Allocator>& OutValues, bool bMaintainOrder = false) const
		{
			for (typename Super::PairSetType::ConstKeyIterator It(Super::Pairs, Key); It; ++It)
			{
				new(OutValues) ValueType(It->Value);
			}

			if (bMaintainOrder)
			{
				Algorithm::Reverse(OutValues);
			}
		}

		/**
		* Finds all values associated with the specified key.
		* @param Key - The key to find associated values for.
		* @param OutValues - Upon return, contains pointers to the values associated with the key.
		*					Pointers are only valid until the next change to any key in the map.
		* @param bMaintainOrder - true if the Values array should be in the same order as the map's pairs.
		*/
		template<typename Allocator> void MultiFindPointer(KeyInitType Key, Array<const ValueType*, Allocator>& OutValues, bool bMaintainOrder = false) const
		{
			for (typename Super::PairSetType::ConstKeyIterator It(Super::Pairs, Key); It; ++It)
			{
				new(OutValues) const ValueType*(&It->Value);
			}

			if (bMaintainOrder)
			{
				Algorithm::Reverse(OutValues);
			}
		}

		/**
		* Adds a key-value association to the map.  The association doesn't replace any of the key's existing associations.
		* However, if both the key and value match an existing association in the map, no new association is made and the existing association's
		* value is returned.
		* @param InKey - The key to associate.
		* @param InValue - The value to associate.
		* @return A reference to the value as stored in the map; the reference is only valid until the next change to any key in the map.
		*/
		__forceinline ValueType& AddUnique(const KeyType&  InKey, const ValueType&  InValue) { return EmplaceUnique(InKey, InValue); }
		__forceinline ValueType& AddUnique(const KeyType&  InKey, ValueType&& InValue) { return EmplaceUnique(InKey, Move(InValue)); }
		__forceinline ValueType& AddUnique(KeyType&& InKey, const ValueType&  InValue) { return EmplaceUnique(Move(InKey), InValue); }
		__forceinline ValueType& AddUnique(KeyType&& InKey, ValueType&& InValue) { return EmplaceUnique(Move(InKey), Move(InValue)); }

		/**
		* Adds a key-value association to the map.  The association doesn't replace any of the key's existing associations.
		* However, if both the key and value match an existing association in the map, no new association is made and the existing association's
		* value is returned.
		* @param InKey - The key to associate.
		* @param InValue - The value to associate.
		* @return A reference to the value as stored in the map; the reference is only valid until the next change to any key in the map.
		*/
		template <typename InitKeyType, typename InitValueType>
		ValueType& EmplaceUnique(InitKeyType&& InKey, InitValueType&& InValue)
		{
			if (ValueType* Found = FindPair(InKey, InValue))
			{
				return *Found;
			}

			// If there's no existing association with the same key and value, create one.
			return Super::Add(Forward<InitKeyType>(InKey), Forward<InitValueType>(InValue));
		}

		/**
		* Removes all value associations for a key.
		* @param InKey - The key to remove associated values for.
		* @return The number of values that were associated with the key.
		*/
		__forceinline int32 Remove(KeyConstPointerType InKey)
		{
			return Super::Remove(InKey);
		}

		/**
		* Removes associations between the specified key and value from the map.
		* @param InKey - The key part of the pair to remove.
		* @param InValue - The value part of the pair to remove.
		* @return The number of associations removed.
		*/
		int32 Remove(KeyInitType InKey, ValueInitType InValue)
		{
			// Iterate over pairs with a matching key.
			int32 NumRemovedPairs = 0;
			for (typename Super::PairSetType::KeyIterator It(Super::Pairs, InKey); It; ++It)
			{
				// If this pair has a matching value as well, remove it.
				if (It->Value == InValue)
				{
					It.RemoveCurrent();
					++NumRemovedPairs;
				}
			}
			return NumRemovedPairs;
		}

		/**
		* Removes the first association between the specified key and value from the map.
		* @param InKey - The key part of the pair to remove.
		* @param InValue - The value part of the pair to remove.
		* @return The number of associations removed.
		*/
		int32 RemoveSingle(KeyInitType InKey, ValueInitType InValue)
		{
			// Iterate over pairs with a matching key.
			int32 NumRemovedPairs = 0;
			for (typename Super::PairSetType::KeyIterator It(Super::Pairs, InKey); It; ++It)
			{
				// If this pair has a matching value as well, remove it.
				if (It->Value == InValue)
				{
					It.RemoveCurrent();
					++NumRemovedPairs;

					// We were asked to remove only the first association, so bail out.
					break;
				}
			}
			return NumRemovedPairs;
		}

		/**
		* Finds an association between a specified key and value. (const)
		* @param Key - The key to find.
		* @param Value - The value to find.
		* @return If the map contains a matching association, a pointer to the value in the map is returned.  Otherwise nullptr is returned.
		*			The pointer is only valid as long as the map isn't changed.
		*/
		__forceinline const ValueType* FindPair(KeyInitType Key, ValueInitType Value) const
		{
			return const_cast<MultiMap*>(this)->FindPair(Key, Value);
		}

		/**
		* Finds an association between a specified key and value.
		* @param Key - The key to find.
		* @param Value - The value to find.
		* @return If the map contains a matching association, a pointer to the value in the map is returned.  Otherwise nullptr is returned.
		*			The pointer is only valid as long as the map isn't changed.
		*/
		ValueType* FindPair(KeyInitType Key, ValueInitType Value)
		{
			// Iterate over pairs with a matching key.
			for (typename Super::PairSetType::KeyIterator It(Super::Pairs, Key); It; ++It)
			{
				// If the pair's value matches, return a pointer to it.
				if (It->Value == Value)
				{
					return &It->Value;
				}
			}

			return nullptr;
		}

		/** Returns the number of values within this map associated with the specified key */
		int32 Num(KeyInitType Key) const
		{
			// Iterate over pairs with a matching key.
			int32 NumMatchingPairs = 0;
			for (typename Super::PairSetType::ConstKeyIterator It(Super::Pairs, Key); It; ++It)
			{
				++NumMatchingPairs;
			}
			return NumMatchingPairs;
		}

		// Since we implement an overloaded Size() function in MultiMap, we need to reimplement MapBase::Num to make it visible.
		__forceinline int32 Size() const
		{
			return Super::Size();
		}
	};


	template <typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
	struct ContainerTraits<Map<KeyType, ValueType, SetAllocator, KeyFuncs>> : public ContainerTraitsBase<Map<KeyType, ValueType, SetAllocator, KeyFuncs>>
	{
		enum { MoveWillEmptyContainer = ContainerTraits<typename Map<KeyType, ValueType, SetAllocator, KeyFuncs>::PairSetType>::MoveWillEmptyContainer };
	};

	template <typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
	struct ContainerTraits<MultiMap<KeyType, ValueType, SetAllocator, KeyFuncs>> : public ContainerTraitsBase<MultiMap<KeyType, ValueType, SetAllocator, KeyFuncs>>
	{
		enum { MoveWillEmptyContainer = ContainerTraits<typename MultiMap<KeyType, ValueType, SetAllocator, KeyFuncs>::PairSetType>::MoveWillEmptyContainer };
	};

}