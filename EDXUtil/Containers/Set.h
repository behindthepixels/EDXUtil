#pragma once

#include "../Core/Types.h"
#include "SparseArray.h"
#include "../Core/Template.h"
#include "../Core/TypeHash.h"
#include "../Core/Sorting.h"
#include "../Core/Misc.h"

namespace EDX
{
	/**
	* The base KeyFuncs type with some useful definitions for all KeyFuncs; meant to be derived from instead of used directly.
	* bInAllowDuplicateKeys=true is slightly faster because it allows the Set to skip validating that
	* there isn't already a duplicate entry in the Set.
	*/
	template<typename ElementType, typename InKeyType, bool bInAllowDuplicateKeys = false>
	struct BaseKeyFuncs
	{
		typedef InKeyType KeyType;
		typedef typename CallTraits<InKeyType>::ParamType KeyInitType;
		typedef typename CallTraits<ElementType>::ParamType ElementInitType;

		enum { bAllowDuplicateKeys = bInAllowDuplicateKeys };
	};

	/**
	* A default implementation of the KeyFuncs used by Set which uses the element as a key.
	*/
	template<typename ElementType, bool bInAllowDuplicateKeys = false>
	struct DefaultKeyFuncs : BaseKeyFuncs<ElementType, ElementType, bInAllowDuplicateKeys>
	{
		typedef typename CallTraits<ElementType>::ParamType KeyInitType;
		typedef typename CallTraits<ElementType>::ParamType ElementInitType;

		/**
		* @return The key used to index the given element.
		*/
		static __forceinline KeyInitType GetSetKey(ElementInitType Element)
		{
			return Element;
		}

		/**
		* @return True if the keys match.
		*/
		static __forceinline bool Matches(KeyInitType A, KeyInitType B)
		{
			return A == B;
		}

		/** Calculates a hash index for a key. */
		static __forceinline uint32 GetKeyHash(KeyInitType Key)
		{
			return GetTypeHash(Key);
		}
	};

	// Forward declaration.
	template<
		typename InElementType,
		typename KeyFuncs = DefaultKeyFuncs<InElementType>,
		typename Allocator = DefaultSetAllocator>
	class Set;

	/** Either NULL or an identifier for an element of a set. */
	class SetElementId
	{
	public:

		template<typename, typename, typename>
		friend class Set;

		/** Default constructor. */
		__forceinline SetElementId() :
			Index(INDEX_NONE)
		{}

		/** @return a boolean value representing whether the id is NULL. */
		__forceinline bool IsValidId() const
		{
			return Index != INDEX_NONE;
		}

		/** Comparison operator. */
		__forceinline friend bool operator==(const SetElementId& A, const SetElementId& B)
		{
			return A.Index == B.Index;
		}

		__forceinline int32 AsInteger() const
		{
			return Index;
		}

		__forceinline static SetElementId FromInteger(int32 Integer)
		{
			return SetElementId(Integer);
		}

	private:

		/** The index of the element in the set's element array. */
		int32 Index;

		/** Initialization constructor. */
		__forceinline SetElementId(int32 InIndex) :
			Index(InIndex)
		{}

		/** Implicit conversion to the element index. */
		__forceinline operator int32() const
		{
			return Index;
		}
	};

	/** An element in the set. */
	template <typename InElementType>
	class SetElement
	{
	public:
		typedef InElementType ElementType;

		/** The element's value. */
		ElementType Value;

		/** The id of the next element in the same hash bucket. */
		mutable SetElementId HashNextId;

		/** The hash bucket that the element is currently linked to. */
		mutable int32 HashIndex;

		/** Default constructor. */
		__forceinline SetElement()
		{}

		/** Initialization constructor. */
		template <typename InitType> __forceinline SetElement(const InitType&  InValue) : Value(InValue) {}
		template <typename InitType> __forceinline SetElement(InitType&& InValue) : Value(Move(InValue)) {}

		/** Copy/move constructors */
		__forceinline SetElement(const SetElement&  Rhs) : Value(Rhs.Value), HashNextId(Rhs.HashNextId), HashIndex(Rhs.HashIndex) {}
		__forceinline SetElement(SetElement&& Rhs) : Value(Move(Rhs.Value)), HashNextId(Move(Rhs.HashNextId)), HashIndex(Rhs.HashIndex) {}

		/** Copy/move assignment */
		__forceinline SetElement& operator=(const SetElement&  Rhs) { Value = Rhs.Value; HashNextId = Rhs.HashNextId; HashIndex = Rhs.HashIndex; return *this; }
		__forceinline SetElement& operator=(SetElement&& Rhs) { Value = Move(Rhs.Value); HashNextId = Move(Rhs.HashNextId); HashIndex = Rhs.HashIndex; return *this; }

		///** Serializer. */
		//__forceinline friend FArchive& operator<<(FArchive& Ar, SetElement& Element)
		//{
		//	return Ar << Element.Value;
		//}

		// Comparison operators
		__forceinline bool operator==(const SetElement& Other) const
		{
			return Value == Other.Value;
		}
		__forceinline bool operator!=(const SetElement& Other) const
		{
			return Value != Other.Value;
		}
	};

	/**
	* A set with an optional KeyFuncs parameters for customizing how the elements are compared and searched.
	* E.g. You can specify a mapping from elements to keys if you want to find elements by specifying a subset of
	* the element type.  It uses a SparseArray of the elements, and also links the elements into a hash with a
	* number of buckets proportional to the number of elements.  Addition, removal, and finding are O(1).
	*
	**/
	template<
		typename InElementType,
		typename KeyFuncs /*= DefaultKeyFuncs<ElementType>*/,
		typename Allocator /*= DefaultSetAllocator*/
	>
		class Set
	{
		friend struct ContainerTraits<Set>;

		typedef typename KeyFuncs::KeyInitType     KeyInitType;
		typedef typename KeyFuncs::ElementInitType ElementInitType;

		typedef SetElement<InElementType> SetElementType;

	public:
		typedef InElementType ElementType;

		/** Initialization constructor. */
		__forceinline Set()
			: HashSize(0)
		{}

		/** Copy constructor. */
		__forceinline Set(const Set& Copy)
			: HashSize(0)
		{
			*this = Copy;
		}

		__forceinline explicit Set(const Array<ElementType>& InArray)
			: HashSize(0)
		{
			Append(InArray);
		}

		__forceinline explicit Set(Array<ElementType>&& InArray)
			: HashSize(0)
		{
			Append(Move(InArray));
		}

		/** Destructor. */
		__forceinline ~Set()
		{
			HashSize = 0;
		}

		/** Assignment operator. */
		Set& operator=(const Set& Copy)
		{
			if (this != &Copy)
			{
				Clear(Copy.Size());
				for (ConstIterator CopyIt(Copy); CopyIt; ++CopyIt)
				{
					Add(*CopyIt);
				}
			}
			return *this;
		}

	private:
		template <typename SetType>
		static __forceinline typename EnableIf<ContainerTraits<SetType>::MoveWillEmptyContainer>::Type MoveOrCopy(SetType& ToSet, SetType& FromSet)
		{
			ToSet.Elements = (ElementArrayType&&)FromSet.Elements;

			ToSet.Hash.MoveToEmpty(FromSet.Hash);

			ToSet.HashSize = FromSet.HashSize;
			FromSet.HashSize = 0;
		}

		template <typename SetType>
		static __forceinline typename EnableIf<!ContainerTraits<SetType>::MoveWillEmptyContainer>::Type MoveOrCopy(SetType& ToSet, SetType& FromSet)
		{
			ToSet = FromSet;
		}

	public:
		/** Move constructor. */
		Set(Set&& Other)
			: HashSize(0)
		{
			MoveOrCopy(*this, Other);
		}

		/** Move assignment operator. */
		Set& operator=(Set&& Other)
		{
			if (this != &Other)
			{
				MoveOrCopy(*this, Other);
			}

			return *this;
		}

		/** Constructor for moving elements from a Set with a different SetAllocator */
		template<typename OtherAllocator>
		Set(Set<ElementType, KeyFuncs, OtherAllocator>&& Other)
			: HashSize(0)
		{
			Append(Move(Other));
		}

		/** Constructor for copying elements from a Set with a different SetAllocator */
		template<typename OtherAllocator>
		Set(const Set<ElementType, KeyFuncs, OtherAllocator>& Other)
			: HashSize(0)
		{
			Append(Other);
		}

		/** Assignment operator for moving elements from a Set with a different SetAllocator */
		template<typename OtherAllocator>
		Set& operator=(Set<ElementType, KeyFuncs, OtherAllocator>&& Other)
		{
			Reset();
			Append(Move(Other));
			return *this;
		}

		/** Assignment operator for copying elements from a Set with a different SetAllocator */
		template<typename OtherAllocator>
		Set& operator=(const Set<ElementType, KeyFuncs, OtherAllocator>& Other)
		{
			Reset();
			Append(Other);
			return *this;
		}

		/**
		* Removes all elements from the set, potentially leaving space allocated for an expected number of elements about to be added.
		* @param ExpectedNumElements - The number of elements about to be added to the set.
		*/
		void Clear(int32 ExpectedNumElements = 0)
		{
			// Empty the elements array, and reallocate it for the expected number of elements.
			Elements.Clear(ExpectedNumElements);

			// Resize the hash to the desired size for the expected number of elements.
			if (!ConditionalRehash(ExpectedNumElements, true))
			{
				// If the hash was already the desired size, clear the references to the elements that have now been removed.
				for (int32 HashIndex = 0, LocalHashSize = HashSize; HashIndex < LocalHashSize; ++HashIndex)
				{
					GetTypedHash(HashIndex) = SetElementId();
				}
			}
		}

		/** Efficiently empties out the set but preserves all allocations and capacities */
		void Reset()
		{
			// Reset the elements array.
			Elements.Reset();

			// Clear the references to the elements that have now been removed.
			for (int32 HashIndex = 0, LocalHashSize = HashSize; HashIndex < LocalHashSize; ++HashIndex)
			{
				GetTypedHash(HashIndex) = SetElementId();
			}
		}

		/** Shrinks the set's element storage to avoid slack. */
		__forceinline void Shrink()
		{
			Elements.Shrink();
			Relax();
		}

		/** Compacts the allocated elements into a contiguous range. */
		__forceinline void Compact()
		{
			if (Elements.Compact())
			{
				Rehash();
			}
		}

		/** Compacts the allocated elements into a contiguous range. Does not change the iteration order of the elements. */
		__forceinline void CompactStable()
		{
			if (Elements.CompactStable())
			{
				Rehash();
			}
		}

		/** Preallocates enough memory to contain Number elements */
		__forceinline void Reserve(int32 Number)
		{
			// makes sense only when Number > Elements.Size() since SparseArray::Reserve 
			// does any work only if that's the case
			if (Number > Elements.Size())
			{
				Elements.Reserve(Number);
			}
		}

		/** Relaxes the set's hash to a size strictly bounded by the number of elements in the set. */
		__forceinline void Relax()
		{
			ConditionalRehash(Elements.Size(), true);
		}

		/**
		* Helper function to return the amount of memory allocated by this container
		* @return number of bytes allocated by this container
		*/
		__forceinline uint32 GetAllocatedSize(void) const
		{
			return Elements.GetAllocatedSize() + (HashSize * sizeof(SetElementId));
		}

		///** Tracks the container's memory use through an archive. */
		//__forceinline void CountBytes(FArchive& Ar)
		//{
		//	Elements.CountBytes(Ar);
		//	Ar.CountBytes(HashSize * sizeof(int32), HashSize * sizeof(SetElementId));
		//}

		/** @return the number of elements. */
		__forceinline int32 Size() const
		{
			return Elements.Size();
		}

		/**
		* Checks whether an element id is valid.
		* @param Id - The element id to check.
		* @return true if the element identifier refers to a valid element in this set.
		*/
		__forceinline bool IsValidId(SetElementId Id) const
		{
			return	Id.IsValidId() &&
				Id >= 0 &&
				Id < Elements.GetMaxIndex() &&
				Elements.IsAllocated(Id);
		}

		/** Accesses the identified element's value. */
		__forceinline ElementType& operator[](SetElementId Id)
		{
			return Elements[Id].Value;
		}

		/** Accesses the identified element's value. */
		__forceinline const ElementType& operator[](SetElementId Id) const
		{
			return Elements[Id].Value;
		}

		/**
		* Adds an element to the set.
		*
		* @param	InElement					Element to add to set
		* @param	bIsAlreadyInSetPtr	[out]	Optional pointer to bool that will be set depending on whether element is already in set
		* @return	A pointer to the element stored in the set.
		*/
		__forceinline SetElementId Add(const InElementType&  InElement, bool* bIsAlreadyInSetPtr = NULL) { return Emplace(InElement, bIsAlreadyInSetPtr); }
		__forceinline SetElementId Add(InElementType&& InElement, bool* bIsAlreadyInSetPtr = NULL) { return Emplace(Move(InElement), bIsAlreadyInSetPtr); }

		/**
		* Adds an element to the set.
		*
		* @param	Args						The argument(s) to be forwarded to the set element's constructor.
		* @param	bIsAlreadyInSetPtr	[out]	Optional pointer to bool that will be set depending on whether element is already in set
		* @return	A pointer to the element stored in the set.
		*/
		template <typename ArgsType>
		SetElementId Emplace(ArgsType&& Args, bool* bIsAlreadyInSetPtr = NULL)
		{
			// Create a new element.
			SparseArrayAllocationInfo ElementAllocation = Elements.AddUninitialized();
			SetElementId ElementId(ElementAllocation.Index);
			auto& Element = *new(ElementAllocation) SetElementType(Forward<ArgsType>(Args));

			bool bIsAlreadyInSet = false;
			if (!KeyFuncs::bAllowDuplicateKeys)
			{
				// If the set doesn't allow duplicate keys, check for an existing element with the same key as the element being added.

				// Don't bother searching for a duplicate if this is the first element we're adding
				if (Elements.Size() != 1)
				{
					SetElementId ExistingId = FindId(KeyFuncs::GetSetKey(Element.Value));
					bIsAlreadyInSet = ExistingId.IsValidId();
					if (bIsAlreadyInSet)
					{
						// If there's an existing element with the same key as the new element, replace the existing element with the new element.
						MoveByRelocate(Elements[ExistingId].Value, Element.Value);

						// Then remove the new element.
						Elements.RemoveAtUninitialized(ElementId);

						// Then point the return value at the replaced element.
						ElementId = ExistingId;
					}
				}
			}

			if (!bIsAlreadyInSet)
			{
				// Check if the hash needs to be resized.
				if (!ConditionalRehash(Elements.Size()))
				{
					// If the rehash didn't add the new element to the hash, add it.
					HashElement(ElementId, Element);
				}
			}

			if (bIsAlreadyInSetPtr)
			{
				*bIsAlreadyInSetPtr = bIsAlreadyInSet;
			}

			return ElementId;
		}

		template<typename ArrayAllocator>
		void Append(const Array<ElementType, ArrayAllocator>& InElements)
		{
			Reserve(Elements.Size() + InElements.Size());
			for (auto& Element : InElements)
			{
				Add(Element);
			}
		}

		template<typename ArrayAllocator>
		void Append(Array<ElementType, ArrayAllocator>&& InElements)
		{
			Reserve(Elements.Size() + InElements.Size());
			for (auto& Element : InElements)
			{
				Add(Move(Element));
			}
			InElements.Reset();
		}

		/**
		* Add all items from another set to our set (union without creating a new set)
		* @param OtherSet - The other set of items to add.
		*/
		template<typename OtherAllocator>
		void Append(const Set<ElementType, KeyFuncs, OtherAllocator>& OtherSet)
		{
			Reserve(Elements.Size() + OtherSet.Size());
			for (auto& Element : OtherSet)
			{
				Add(Element);
			}
		}

		template<typename OtherAllocator>
		void Append(Set<ElementType, KeyFuncs, OtherAllocator>&& OtherSet)
		{
			Reserve(Elements.Size() + OtherSet.Size());
			for (auto& Element : OtherSet)
			{
				Add(Move(Element));
			}
			OtherSet.Reset();
		}

		/**
		* Removes an element from the set.
		* @param Element - A pointer to the element in the set, as returned by Add or Find.
		*/
		void Remove(SetElementId ElementId)
		{
			if (Elements.Size())
			{
				const auto& ElementBeingRemoved = Elements[ElementId];

				// Remove the element from the hash.
				for (SetElementId* NextElementId = &GetTypedHash(ElementBeingRemoved.HashIndex);
					NextElementId->IsValidId();
					NextElementId = &Elements[*NextElementId].HashNextId)
				{
					if (*NextElementId == ElementId)
					{
						*NextElementId = ElementBeingRemoved.HashNextId;
						break;
					}
				}
			}

			// Remove the element from the elements array.
			Elements.RemoveAt(ElementId);
		}

		/**
		* Finds an element with the given key in the set.
		* @param Key - The key to search for.
		* @return The id of the set element matching the given key, or the NULL id if none matches.
		*/
		SetElementId FindId(KeyInitType Key) const
		{
			if (Elements.Size())
			{
				for (SetElementId ElementId = GetTypedHash(KeyFuncs::GetKeyHash(Key));
					ElementId.IsValidId();
					ElementId = Elements[ElementId].HashNextId)
				{
					if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Elements[ElementId].Value), Key))
					{
						// Return the first match, regardless of whether the set has multiple matches for the key or not.
						return ElementId;
					}
				}
			}
			return SetElementId();
		}

		/**
		* Finds an element with the given key in the set.
		* @param Key - The key to search for.
		* @return A pointer to an element with the given key.  If no element in the set has the given key, this will return NULL.
		*/
		__forceinline ElementType* Find(KeyInitType Key)
		{
			SetElementId ElementId = FindId(Key);
			if (ElementId.IsValidId())
			{
				return &Elements[ElementId].Value;
			}
			else
			{
				return NULL;
			}
		}

		/**
		* Finds an element with the given key in the set.
		* @param Key - The key to search for.
		* @return A const pointer to an element with the given key.  If no element in the set has the given key, this will return NULL.
		*/
		__forceinline const ElementType* Find(KeyInitType Key) const
		{
			SetElementId ElementId = FindId(Key);
			if (ElementId.IsValidId())
			{
				return &Elements[ElementId].Value;
			}
			else
			{
				return NULL;
			}
		}

		/**
		* Removes all elements from the set matching the specified key.
		* @param Key - The key to match elements against.
		* @return The number of elements removed.
		*/
		int32 Remove(KeyInitType Key)
		{
			int32 NumRemovedElements = 0;

			if (Elements.Size())
			{
				SetElementId* NextElementId = &GetTypedHash(KeyFuncs::GetKeyHash(Key));
				while (NextElementId->IsValidId())
				{
					auto& Element = Elements[*NextElementId];
					if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Element.Value), Key))
					{
						// This element matches the key, remove it from the set.  Note that Remove sets *NextElementId to point to the next
						// element after the removed element in the hash bucket.
						Remove(*NextElementId);
						NumRemovedElements++;

						if (!KeyFuncs::bAllowDuplicateKeys)
						{
							// If the hash disallows duplicate keys, we're done removing after the first matched key.
							break;
						}
					}
					else
					{
						NextElementId = &Element.HashNextId;
					}
				}
			}

			return NumRemovedElements;
		}

		/**
		* Checks if the element contains an element with the given key.
		* @param Key - The key to check for.
		* @return true if the set contains an element with the given key.
		*/
		__forceinline bool Contains(KeyInitType Key) const
		{
			return FindId(Key).IsValidId();
		}

		/**
		* Sorts the set's elements using the provided comparison class.
		*/
		template <typename PREDICATE_CLASS>
		void Sort(const PREDICATE_CLASS& Predicate)
		{
			// Sort the elements according to the provided comparison class.
			Elements.Sort(ElementCompareClass< PREDICATE_CLASS >(Predicate));

			// Rehash.
			Rehash();
		}

		///** Serializer. */
		//friend FArchive& operator<<(FArchive& Ar, Set& Set)
		//{
		//	// Load the set's new elements.
		//	Ar << Set.Elements;

		//	if (Ar.IsLoading())
		//	{
		//		// Free the old hash.
		//		Set.Hash.ResizeAllocation(0, 0, sizeof(SetElementId));
		//		Set.HashSize = 0;

		//		// Hash the newly loaded elements.
		//		Set.ConditionalRehash(Set.Elements.Size());
		//	}

		//	return Ar;
		//}

		///**
		//* Describes the set's contents through an output device.
		//* @param Ar - The output device to describe the set's contents through.
		//*/
		//void Dump(FOutputDevice& Ar)
		//{
		//	Ar.Logf(EDX_TEXT("Set: %i elements, %i hash slots"), Elements.Size(), HashSize);
		//	for (int32 HashIndex = 0, LocalHashSize = HashSize; HashIndex < LocalHashSize; ++HashIndex)
		//	{
		//		// Count the number of elements in this hash bucket.
		//		int32 NumElementsInBucket = 0;
		//		for (SetElementId ElementId = GetTypedHash(HashIndex);
		//			ElementId.IsValidId();
		//			ElementId = Elements[ElementId].HashNextId)
		//		{
		//			NumElementsInBucket++;
		//		}

		//		Ar.Logf(EDX_TEXT("   Hash[%i] = %i"), HashIndex, NumElementsInBucket);
		//	}
		//}

		bool VerifyHashElementsKey(KeyInitType Key)
		{
			bool bResult = true;
			if (Elements.Size())
			{
				// iterate over all elements for the hash entry of the given key 
				// and verify that the ids are valid
				SetElementId ElementId = GetTypedHash(KeyFuncs::GetKeyHash(Key));
				while (ElementId.IsValidId())
				{
					if (!IsValidId(ElementId))
					{
						bResult = false;
						break;
					}
					ElementId = Elements[ElementId].HashNextId;
				}
			}
			return bResult;
		}

		//void DumpHashElements(FOutputDevice& Ar)
		//{
		//	for (int32 HashIndex = 0, LocalHashSize = HashSize; HashIndex < LocalHashSize; ++HashIndex)
		//	{
		//		Ar.Logf(EDX_TEXT("   Hash[%i]"), HashIndex);

		//		// iterate over all elements for the all hash entries 
		//		// and dump info for all elements
		//		SetElementId ElementId = GetTypedHash(HashIndex);
		//		while (ElementId.IsValidId())
		//		{
		//			if (!IsValidId(ElementId))
		//			{
		//				Ar.Logf(EDX_TEXT("		!!INVALID!! ElementId = %d"), ElementId.Index);
		//			}
		//			else
		//			{
		//				Ar.Logf(EDX_TEXT("		VALID ElementId = %d"), ElementId.Index);
		//			}
		//			ElementId = Elements[ElementId].HashNextId;
		//		}
		//	}
		//}

		// Legacy comparison operators.  Note that these also test whether the set's elements were added in the same order!
		friend bool LegacyCompareEqual(const Set& A, const Set& B)
		{
			return A.Elements == B.Elements;
		}
		friend bool LegacyCompareNotEqual(const Set& A, const Set& B)
		{
			return A.Elements != B.Elements;
		}

		/** @return the intersection of two sets. (A AND B)*/
		Set Intersect(const Set& OtherSet) const
		{
			const bool bOtherSmaller = (Size() > OtherSet.Size());
			const Set& A = (bOtherSmaller ? OtherSet : *this);
			const Set& B = (bOtherSmaller ? *this : OtherSet);

			Set Result;
			Result.Reserve(A.Size()); // Worst case is everything in smaller is in larger

			for (ConstIterator SetIt(A); SetIt; ++SetIt)
			{
				if (B.Contains(KeyFuncs::GetSetKey(*SetIt)))
				{
					Result.Add(*SetIt);
				}
			}
			return Result;
		}

		/** @return the union of two sets. (A OR B)*/
		Set Union(const Set& OtherSet) const
		{
			Set Result;
			Result.Reserve(Size() + OtherSet.Size()); // Worst case is 2 totally unique Sets

			for (ConstIterator SetIt(*this); SetIt; ++SetIt)
			{
				Result.Add(*SetIt);
			}
			for (ConstIterator SetIt(OtherSet); SetIt; ++SetIt)
			{
				Result.Add(*SetIt);
			}
			return Result;
		}

		/** @return the complement of two sets. (A not in B where A is this and B is Other)*/
		Set Difference(const Set& OtherSet) const
		{
			Set Result;
			Result.Reserve(Size()); // Worst case is no elements of this are in Other

			for (ConstIterator SetIt(*this); SetIt; ++SetIt)
			{
				if (!OtherSet.Contains(KeyFuncs::GetSetKey(*SetIt)))
				{
					Result.Add(*SetIt);
				}
			}
			return Result;
		}

		/**
		* Determine whether the specified set is entirely included within this set
		*
		* @param OtherSet	Set to check
		*
		* @return True if the other set is entirely included in this set, false if it is not
		*/
		bool Includes(const Set<ElementType, KeyFuncs, Allocator>& OtherSet) const
		{
			bool bIncludesSet = true;
			if (OtherSet.Size() <= Size())
			{
				for (ConstIterator OtherSetIt(OtherSet); OtherSetIt; ++OtherSetIt)
				{
					if (!Contains(KeyFuncs::GetSetKey(*OtherSetIt)))
					{
						bIncludesSet = false;
						break;
					}
				}
			}
			else
			{
				// Not possible to include if it is bigger than us
				bIncludesSet = false;
			}
			return bIncludesSet;
		}

		/** @return a Array of the elements */
		Array<ElementType> Array() const
		{
			Array<ElementType> Result;
			Result.Reserve(Size());
			for (ConstIterator SetIt(*this); SetIt; ++SetIt)
			{
				Result.Add(*SetIt);
			}
			return Result;
		}

		/**
		* Checks that the specified address is not part of an element within the container.  Used for implementations
		* to check that reference arguments aren't going to be invalidated by possible reallocation.
		*
		* @param Addr The address to check.
		*/
		__forceinline void CheckAddress(const ElementType* Addr) const
		{
			Elements.CheckAddress(Addr);
		}

	private:
		/** Extracts the element value from the set's element structure and passes it to the user provided comparison class. */
		template <typename PREDICATE_CLASS>
		class ElementCompareClass
		{
			DereferenceWrapper< ElementType, PREDICATE_CLASS > Predicate;

		public:
			__forceinline ElementCompareClass(const PREDICATE_CLASS& InPredicate)
				: Predicate(InPredicate)
			{}

			__forceinline bool operator()(const SetElementType& A, const SetElementType& B) const
			{
				return Predicate(A.Value, B.Value);
			}
		};

		typedef SparseArray<SetElementType, typename Allocator::SparseArrayAllocator>     ElementArrayType;
		typedef typename Allocator::HashAllocator::template ForElementType<SetElementId> HashType;

		ElementArrayType Elements;

		mutable HashType Hash;
		mutable int32    HashSize;

		__forceinline SetElementId& GetTypedHash(int32 HashIndex) const
		{
			return ((SetElementId*)Hash.GetAllocation())[HashIndex & (HashSize - 1)];
		}

		/**
		* Accesses an element in the set.
		* This is needed because the iterator classes aren't friends of SetElementId and so can't access the element index.
		*/
		__forceinline const SetElementType& GetInternalElement(SetElementId Id) const
		{
			return Elements[Id];
		}
		__forceinline SetElementType& GetInternalElement(SetElementId Id)
		{
			return Elements[Id];
		}

		/**
		* Translates an element index into an element ID.
		* This is needed because the iterator classes aren't friends of SetElementId and so can't access the SetElementId private constructor.
		*/
		static __forceinline SetElementId IndexToId(int32 Index)
		{
			return SetElementId(Index);
		}

		/** Adds an element to the hash. */
		__forceinline void HashElement(SetElementId ElementId, const SetElementType& Element) const
		{
			// Compute the hash bucket the element goes in.
			Element.HashIndex = KeyFuncs::GetKeyHash(KeyFuncs::GetSetKey(Element.Value)) & (HashSize - 1);

			// Link the element into the hash bucket.
			Element.HashNextId = GetTypedHash(Element.HashIndex);
			GetTypedHash(Element.HashIndex) = ElementId;
		}

		/**
		* Checks if the hash has an appropriate number of buckets, and if not resizes it.
		* @param NumHashedElements - The number of elements to size the hash for.
		* @param bAllowShrinking - true if the hash is allowed to shrink.
		* @return true if the set was rehashed.
		*/
		bool ConditionalRehash(int32 NumHashedElements, bool bAllowShrinking = false) const
		{
			// Calculate the desired hash size for the specified number of elements.
			const int32 DesiredHashSize = Allocator::GetNumberOfHashBuckets(NumHashedElements);

			// If the hash hasn't been created yet, or is smaller than the desired hash size, rehash.
			if (NumHashedElements > 0 &&
				(!HashSize ||
					HashSize < DesiredHashSize ||
					(HashSize > DesiredHashSize && bAllowShrinking)))
			{
				HashSize = DesiredHashSize;
				Rehash();
				return true;
			}
			else
			{
				return false;
			}
		}

		/** Resizes the hash. */
		void Rehash() const
		{
			// Free the old hash.
			Hash.ResizeAllocation(0, 0, sizeof(SetElementId));

			int32 LocalHashSize = HashSize;
			if (LocalHashSize)
			{
				// Allocate the new hash.
				Assert(!(LocalHashSize & (HashSize - 1)));
				Hash.ResizeAllocation(0, LocalHashSize, sizeof(SetElementId));
				for (int32 HashIndex = 0; HashIndex < LocalHashSize; ++HashIndex)
				{
					GetTypedHash(HashIndex) = SetElementId();
				}

				// Add the existing elements to the new hash.
				for (typename ElementArrayType::ConstIterator ElementIt(Elements); ElementIt; ++ElementIt)
				{
					HashElement(SetElementId(ElementIt.GetIndex()), *ElementIt);
				}
			}
		}

		/** The base type of whole set iterators. */
		template<bool bConst>
		class BaseIterator
		{
		private:
			friend class Set;

			typedef typename ChooseClass<bConst, const ElementType, ElementType>::Result ItElementType;

		public:
			typedef typename ChooseClass<bConst, typename ElementArrayType::ConstIterator, typename ElementArrayType::Iterator>::Result ElementItType;

			__forceinline BaseIterator(const ElementItType& InElementIt)
				: ElementIt(InElementIt)
			{
			}

			/** Advances the iterator to the next element. */
			__forceinline BaseIterator& operator++()
			{
				++ElementIt;
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			__forceinline explicit operator bool() const
			{
				return !!ElementIt;
			}
			/** inverse of the "bool" operator */
			__forceinline bool operator !() const
			{
				return !(bool)*this;
			}

			// Accessors.
			__forceinline SetElementId GetId() const
			{
				return Set::IndexToId(ElementIt.GetIndex());
			}
			__forceinline ItElementType* operator->() const
			{
				return &ElementIt->Value;
			}
			__forceinline ItElementType& operator*() const
			{
				return ElementIt->Value;
			}

			__forceinline friend bool operator==(const BaseIterator& Lhs, const BaseIterator& Rhs) { return Lhs.ElementIt == Rhs.ElementIt; }
			__forceinline friend bool operator!=(const BaseIterator& Lhs, const BaseIterator& Rhs) { return Lhs.ElementIt != Rhs.ElementIt; }

			ElementItType ElementIt;
		};

		/** The base type of whole set iterators. */
		template<bool bConst>
		class BaseKeyIterator
		{
		private:
			typedef typename ChooseClass<bConst, const Set, Set>::Result SetType;
			typedef typename ChooseClass<bConst, const ElementType, ElementType>::Result ItElementType;

		public:
			/** Initialization constructor. */
			__forceinline BaseKeyIterator(SetType& InSet, KeyInitType InKey)
				: Set(InSet)
				, Key(InKey)
				, Id()
			{
				// The set's hash needs to be initialized to find the elements with the specified key.
				Set.ConditionalRehash(Set.Elements.Size());
				if (Set.HashSize)
				{
					NextId = Set.GetTypedHash(KeyFuncs::GetKeyHash(Key));
					++(*this);
				}
			}

			/** Advances the iterator to the next element. */
			__forceinline BaseKeyIterator& operator++()
			{
				Id = NextId;

				while (Id.IsValidId())
				{
					NextId = Set.GetInternalElement(Id).HashNextId;
					Assert(Id != NextId);

					if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Set[Id]), Key))
					{
						break;
					}

					Id = NextId;
				}
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			__forceinline explicit operator bool() const
			{
				return Id.IsValidId();
			}
			/** inverse of the "bool" operator */
			__forceinline bool operator !() const
			{
				return !(bool)*this;
			}

			// Accessors.
			__forceinline ItElementType* operator->() const
			{
				return &Set[Id];
			}
			__forceinline ItElementType& operator*() const
			{
				return Set[Id];
			}

		protected:
			SetType& Set;
			typename TypeTraits<typename KeyFuncs::KeyType>::ConstPointerType Key;
			SetElementId Id;
			SetElementId NextId;
		};

	public:

		/** Used to iterate over the elements of a const Set. */
		class ConstIterator : public BaseIterator<true>
		{
			friend class Set;

		public:
			__forceinline ConstIterator(const typename BaseIterator<true>::ElementItType& InElementId)
				: BaseIterator<true>(InElementId)
			{
			}

			__forceinline ConstIterator(const Set& InSet)
				: BaseIterator<true>(begin(InSet.Elements))
			{
			}
		};

		/** Used to iterate over the elements of a Set. */
		class Iterator : public BaseIterator<false>
		{
			friend class Set;

		public:
			__forceinline Iterator(Set& InSet, const typename BaseIterator<false>::ElementItType& InElementId)
				: BaseIterator<false>(InElementId)
				, Set(InSet)
			{
			}

			__forceinline Iterator(Set& InSet)
				: BaseIterator<false>(begin(InSet.Elements))
				, Set(InSet)
			{
			}

			/** Removes the current element from the set. */
			__forceinline void RemoveCurrent()
			{
				Set.Remove(BaseIterator<false>::GetId());
			}

		private:
			Set& Set;
		};

		/** Used to iterate over the elements of a const Set. */
		class ConstKeyIterator : public BaseKeyIterator<true>
		{
		public:
			__forceinline ConstKeyIterator(const Set& InSet, KeyInitType InKey) :
				BaseKeyIterator<true>(InSet, InKey)
			{}
		};

		/** Used to iterate over the elements of a Set. */
		class KeyIterator : public BaseKeyIterator<false>
		{
		public:
			__forceinline KeyIterator(Set& InSet, KeyInitType InKey)
				: BaseKeyIterator<false>(InSet, InKey)
				, Set(InSet)
			{}

			/** Removes the current element from the set. */
			__forceinline void RemoveCurrent()
			{
				Set.Remove(BaseKeyIterator<false>::Id);
				BaseKeyIterator<false>::Id = SetElementId();
			}
		private:
			Set& Set;
		};

		/** Creates an iterator for the contents of this set */
		__forceinline Iterator CreateIterator()
		{
			return Iterator(*this);
		}

		/** Creates a const iterator for the contents of this set */
		__forceinline ConstIterator CreateConstIterator() const
		{
			return ConstIterator(*this);
		}

	private:
		/**
		* DO NOT USE DIRECTLY
		* STL-like iterators to enable range-based for loop support.
		*/
		__forceinline friend Iterator      begin(Set& Set) { return Iterator(Set, begin(Set.Elements)); }
		__forceinline friend ConstIterator begin(const Set& Set) { return ConstIterator(begin(Set.Elements)); }
		__forceinline friend Iterator      end(Set& Set) { return Iterator(Set, end(Set.Elements)); }
		__forceinline friend ConstIterator end(const Set& Set) { return ConstIterator(end(Set.Elements)); }
	};

	template<typename ElementType, typename KeyFuncs, typename Allocator>
	struct ContainerTraits<Set<ElementType, KeyFuncs, Allocator> > : public ContainerTraitsBase<Set<ElementType, KeyFuncs, Allocator> >
	{
		enum {
			MoveWillEmptyContainer =
			ContainerTraits<typename Set<ElementType, KeyFuncs, Allocator>::ElementArrayType>::MoveWillEmptyContainer &&
			AllocatorTraits<typename Allocator::HashAllocator>::SupportsMove
		};
	};

}