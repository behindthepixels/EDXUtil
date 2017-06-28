#pragma once

#include <initializer_list>

#include "AllocationPolicies.h"
#include "../Core/Template.h"
#include "../Core/Memory.h"

#define DEBUG_HEAP 0

#ifndef _DEBUG
#define TARRAY_RANGED_FOR_CHECKS 0
#else
#define TARRAY_RANGED_FOR_CHECKS 1
#endif


namespace EDX
{
	/**
	* Generic iterator which can operate on types that expose the following:
	* - A type called ElementType representing the contained type.
	* - A method IndexType Size() const that returns the number of items in the container.
	* - A method bool IsValidIndex(IndexType index) which returns whether a given index is valid in the container.
	* - A method T& operator\[\](IndexType index) which returns a reference to a contained object by index.
	*/
	template< typename ContainerType, typename ElementType, typename IndexType>
	class IndexedContainerIterator
	{
	public:
		IndexedContainerIterator(ContainerType& InContainer, IndexType StartIndex = 0)
			: Container(InContainer)
			, Index(StartIndex)
		{
		}

		/** Advances iterator to the next element in the container. */
		IndexedContainerIterator& operator++()
		{
			++Index;
			return *this;
		}
		IndexedContainerIterator operator++(int)
		{
			IndexedContainerIterator Tmp(*this);
			++Index;
			return Tmp;
		}

		/** Moves iterator to the previous element in the container. */
		IndexedContainerIterator& operator--()
		{
			--Index;
			return *this;
		}
		IndexedContainerIterator operator--(int)
		{
			IndexedContainerIterator Tmp(*this);
			--Index;
			return Tmp;
		}

		/** iterator arithmetic support */
		IndexedContainerIterator& operator+=(int32 Offset)
		{
			Index += Offset;
			return *this;
		}

		IndexedContainerIterator operator+(int32 Offset) const
		{
			IndexedContainerIterator Tmp(*this);
			return Tmp += Offset;
		}

		IndexedContainerIterator& operator-=(int32 Offset)
		{
			return *this += -Offset;
		}

		IndexedContainerIterator operator-(int32 Offset) const
		{
			IndexedContainerIterator Tmp(*this);
			return Tmp -= Offset;
		}

		/** @name Element access */
		//@{
		ElementType& operator* () const
		{
			return Container[Index];
		}

		ElementType* operator-> () const
		{
			return &Container[Index];
		}
		//@}

		/** conversion to "bool" returning true if the iterator has not reached the last element. */
		__forceinline explicit operator bool() const
		{
			return Container.IsValidIndex(Index);
		}
		/** inverse of the "bool" operator */
		__forceinline bool operator !() const
		{
			return !(bool)*this;
		}

		/** Returns an index to the current element. */
		IndexType GetIndex() const
		{
			return Index;
		}

		/** Resets the iterator to the first element. */
		void Reset()
		{
			Index = 0;
		}

		__forceinline friend bool operator==(const IndexedContainerIterator& Lhs, const IndexedContainerIterator& Rhs) { return &Lhs.Container == &Rhs.Container && Lhs.Index == Rhs.Index; }
		__forceinline friend bool operator!=(const IndexedContainerIterator& Lhs, const IndexedContainerIterator& Rhs) { return &Lhs.Container != &Rhs.Container || Lhs.Index != Rhs.Index; }

	private:

		ContainerType& Container;
		IndexType      Index;
	};


	/** operator + */
	template <typename ContainerType, typename ElementType, typename IndexType>
	__forceinline IndexedContainerIterator<ContainerType, ElementType, IndexType> operator+(int32 Offset, IndexedContainerIterator<ContainerType, ElementType, IndexType> RHS)
	{
		return RHS + Offset;
	}


#if TARRAY_RANGED_FOR_CHECKS
	/**
	* Pointer-like iterator type for ranged-for loops which checks that the
	* container hasn't been resized during iteration.
	*/
	template <typename ElementType>
	struct CheckedPointerIterator
	{
		// This iterator type only supports the minimal functionality needed to support
		// C++ ranged-for syntax.  For example, it does not provide post-increment ++ nor ==.
		//
		// We do add an operator-- to help String implementation

		explicit CheckedPointerIterator(const int32& InNum, ElementType* InPtr)
			: Ptr(InPtr)
			, CurrentNum(InNum)
			, InitialNum(InNum)
		{
		}

		__forceinline ElementType& operator*() const
		{
			return *Ptr;
		}

		__forceinline CheckedPointerIterator& operator++()
		{
			++Ptr;
			return *this;
		}

		__forceinline CheckedPointerIterator& operator--()
		{
			--Ptr;
			return *this;
		}

	private:
		ElementType* Ptr;
		const int32& CurrentNum;
		int32        InitialNum;

		friend bool operator!=(const CheckedPointerIterator& Lhs, const CheckedPointerIterator& Rhs)
		{
			// We only need to do the check in this operator, because no other operator will be
			// called until after this one returns.
			//
			// Also, we should only need to check one side of this comparison - if the other iterator isn't
			// even from the same array then the compiler has generated bad code.
			Assert(Lhs.CurrentNum == Lhs.InitialNum);
			return Lhs.Ptr != Rhs.Ptr;
		}
	};
#endif


	/**
	* ReversePredicateWrapper class used by implicit heaps.
	* This is similar to DereferenceWrapper from Sorting.h except it reverses the comparison at the same time
	*/
	template <typename ElementType, typename PREDICATE_CLASS>
	class ReversePredicateWrapper
	{
		const PREDICATE_CLASS& Predicate;
	public:
		ReversePredicateWrapper(const PREDICATE_CLASS& InPredicate)
			: Predicate(InPredicate)
		{}

		__forceinline bool operator()(ElementType& A, ElementType& B) const { return Predicate(B, A); }
		__forceinline bool operator()(const ElementType& A, const ElementType& B) const { return Predicate(B, A); }
	};


	/**
	* Partially specialized version of the above.
	*/
	template <typename ElementType, typename PREDICATE_CLASS>
	class ReversePredicateWrapper<ElementType*, PREDICATE_CLASS>
	{
		const PREDICATE_CLASS& Predicate;
	public:
		ReversePredicateWrapper(const PREDICATE_CLASS& InPredicate)
			: Predicate(InPredicate)
		{}

		__forceinline bool operator()(ElementType* A, ElementType* B) const
		{
			Assert(A != nullptr);
			Assert(B != nullptr);
			return Predicate(*B, *A);
		}
		__forceinline bool operator()(const ElementType* A, const ElementType* B) const
		{
			Assert(A != nullptr);
			Assert(B != nullptr);
			return Predicate(*B, *A);
		}
	};


	namespace Array_Private
	{
		template <typename FromArrayType, typename ToArrayType>
		struct CanMoveTArrayPointersBetweenArrayTypes
		{
			typedef typename FromArrayType::Allocator   FromAllocatorType;
			typedef typename ToArrayType::Allocator   ToAllocatorType;
			typedef typename FromArrayType::ElementType FromElementType;
			typedef typename ToArrayType::ElementType ToElementType;

			enum
			{
				Value =
				AreTypesEqual<FromAllocatorType, ToAllocatorType>::Value && // Allocators must be equal
				ContainerTraits<FromArrayType>::MoveWillEmptyContainer &&   // A move must be allowed to leave the source array empty
				(
					AreTypesEqual         <ToElementType, FromElementType>::Value || // The element type of the container must be the same, or...
					IsBitwiseConstructible<ToElementType, FromElementType>::Value    // ... the element type of the source container must be bitwise constructible from the element type in the destination container
				)
			};
		};
	}


	/**
	* Templated dynamic array
	*
	* A dynamically sized array of typed elements.  Makes the assumption that your elements are relocate-able;
	* i.e. that they can be transparently moved to new memory without a copy constructor.  The main implication
	* is that pointers to elements in the Array may be invalidated by adding or removing other elements to the array.
	* Removal of elements is O(N) and invalidates the indices of subsequent elements.
	*
	* Caution: as noted below some methods are not safe for element types that require constructors.
	*
	**/
	template<typename InElementType, typename InAllocator = DefaultAllocator>
	class Array
	{
		template <typename OtherInElementType, typename OtherAllocator>
		friend class Array;

	public:

		typedef InElementType ElementType;
		typedef InAllocator   Allocator;

		/**
		* Constructor, initializes element number counters.
		*/
		__forceinline Array()
			: mSize(0)
			, mCapacity(0)
		{}

		/**
		* Initializer list constructor
		*/
		Array(std::initializer_list<InElementType> InitList)
		{
			// This is not strictly legal, as std::initializer_list's iterators are not guaranteed to be pointers, but
			// this appears to be the case on all of our implementations.  Also, if it's not true on a new implementation,
			// it will fail to compile rather than behave badly.
			CopyToEmpty(InitList.begin(), (int32)InitList.size(), 0, 0);
		}

		/**
		* Copy constructor with changed allocator. Use the common routine to perform the copy.
		*
		* @param Other The source array to copy.
		*/
		template <typename OtherElementType, typename OtherAllocator>
		__forceinline explicit Array(const Array<OtherElementType, OtherAllocator>& Other)
		{
			CopyToEmpty(Other, 0, 0);
		}

		/**
		* Copy constructor. Use the common routine to perform the copy.
		*
		* @param Other The source array to copy.
		*/
		__forceinline Array(const Array& Other)
		{
			CopyToEmpty(Other, 0, 0);
		}

		/**
		* Copy constructor. Use the common routine to perform the copy.
		*
		* @param Other The source array to copy.
		* @param ExtraSlack Tells how much extra memory should be preallocated
		*                   at the end of the array in the number of elements.
		*/
		__forceinline Array(const Array& Other, int32 ExtraSlack)
		{
			CopyToEmpty(Other, 0, ExtraSlack);
		}

		/**
		* Initializer list assignment operator. First deletes all currently contained elements
		* and then copies from initializer list.
		*
		* @param InitList The initializer_list to copy from.
		*/
		__forceinline Array& operator=(std::initializer_list<InElementType> InitList)
		{
			DestructItems(Data(), mSize);
			// This is not strictly legal, as std::initializer_list's iterators are not guaranteed to be pointers, but
			// this appears to be the case on all of our implementations.  Also, if it's not true on a new implementation,
			// it will fail to compile rather than behave badly.
			CopyToEmpty(InitList.begin(), (int32)InitList.size(), mCapacity, 0);
			return *this;
		}

		/**
		* Assignment operator. First deletes all currently contained elements
		* and then copies from other array.
		*
		* Allocator changing version.
		*
		* @param Other The source array to assign from.
		*/
		template<typename OtherAllocator>
		Array& operator=(const Array<ElementType, OtherAllocator>& Other)
		{
			DestructItems(Data(), mSize);
			CopyToEmpty(Other, mCapacity, 0);
			return *this;
		}

		/**
		* Assignment operator. First deletes all currently contained elements
		* and then copies from other array.
		*
		* @param Other The source array to assign from.
		*/
		Array& operator=(const Array& Other)
		{
			if (this != &Other)
			{
				DestructItems(Data(), mSize);
				CopyToEmpty(Other, mCapacity, 0);
			}
			return *this;
		}

	private:

		/**
		* Moves or copies array. Depends on the array type traits.
		*
		* This override moves.
		*
		* @param ToArray Array to move into.
		* @param FromArray Array to move from.
		*/
		template <typename FromArrayType, typename ToArrayType>
		static __forceinline typename EnableIf<Array_Private::CanMoveTArrayPointersBetweenArrayTypes<FromArrayType, ToArrayType>::Value>::Type MoveOrCopy(ToArrayType& ToArray, FromArrayType& FromArray, int32 PrevMax)
		{
			ToArray.AllocatorInstance.MoveToEmpty(FromArray.AllocatorInstance);

			ToArray.mSize = FromArray.mSize;
			ToArray.mCapacity = FromArray.mCapacity;
			FromArray.mSize = 0;
			FromArray.mCapacity = 0;
		}

		/**
		* Moves or copies array. Depends on the array type traits.
		*
		* This override copies.
		*
		* @param ToArray Array to move into.
		* @param FromArray Array to move from.
		* @param ExtraSlack Tells how much extra memory should be preallocated
		*                   at the end of the array in the number of elements.
		*/
		template <typename FromArrayType, typename ToArrayType>
		static __forceinline typename EnableIf<!Array_Private::CanMoveTArrayPointersBetweenArrayTypes<FromArrayType, ToArrayType>::Value>::Type MoveOrCopy(ToArrayType& ToArray, FromArrayType& FromArray, int32 PrevMax)
		{
			ToArray.CopyToEmpty(FromArray, PrevMax, 0);
		}

		/**
		* Moves or copies array. Depends on the array type traits.
		*
		* This override moves.
		*
		* @param ToArray Array to move into.
		* @param FromArray Array to move from.
		* @param ExtraSlack Tells how much extra memory should be preallocated
		*                   at the end of the array in the number of elements.
		*/
		template <typename FromArrayType, typename ToArrayType>
		static __forceinline typename EnableIf<Array_Private::CanMoveTArrayPointersBetweenArrayTypes<FromArrayType, ToArrayType>::Value>::Type MoveOrCopyWithSlack(ToArrayType& ToArray, FromArrayType& FromArray, int32 PrevMax, int32 ExtraSlack)
		{
			MoveOrCopy(ToArray, FromArray, PrevMax);

			ToArray.Reserve(ToArray.mSize + ExtraSlack);
		}

		/**
		* Moves or copies array. Depends on the array type traits.
		*
		* This override copies.
		*
		* @param ToArray Array to move into.
		* @param FromArray Array to move from.
		* @param ExtraSlack Tells how much extra memory should be preallocated
		*                   at the end of the array in the number of elements.
		*/
		template <typename FromArrayType, typename ToArrayType>
		static __forceinline typename EnableIf<!Array_Private::CanMoveTArrayPointersBetweenArrayTypes<FromArrayType, ToArrayType>::Value>::Type MoveOrCopyWithSlack(ToArrayType& ToArray, FromArrayType& FromArray, int32 PrevMax, int32 ExtraSlack)
		{
			ToArray.CopyToEmpty(FromArray, PrevMax, ExtraSlack);
		}

	public:
		/**
		* Move constructor.
		*
		* @param Other Array to move from.
		*/
		__forceinline Array(Array&& Other)
		{
			MoveOrCopy(*this, Other, 0);
		}

		/**
		* Move constructor.
		*
		* @param Other Array to move from.
		*/
		template <typename OtherElementType, typename OtherAllocator>
		__forceinline explicit Array(Array<OtherElementType, OtherAllocator>&& Other)
		{
			MoveOrCopy(*this, Other, 0);
		}

		/**
		* Move constructor.
		*
		* @param Other Array to move from.
		* @param ExtraSlack Tells how much extra memory should be preallocated
		*                   at the end of the array in the number of elements.
		*/
		template <typename OtherElementType>
		Array(Array<OtherElementType, Allocator>&& Other, int32 ExtraSlack)
		{
			// We don't implement move semantics for general OtherAllocators, as there's no way
			// to tell if they're compatible with the current one.  Probably going to be a pretty
			// rare requirement anyway.

			MoveOrCopyWithSlack(*this, Other, 0, ExtraSlack);
		}

		/**
		* Move assignment operator.
		*
		* @param Other Array to assign and move from.
		*/
		Array& operator=(Array&& Other)
		{
			if (this != &Other)
			{
				DestructItems(Data(), mSize);
				MoveOrCopy(*this, Other, mCapacity);
			}
			return *this;
		}

		/** Destructor. */
		~Array()
		{
			DestructItems(Data(), mSize);

#if defined(_MSC_VER) && !defined(__clang__)	// Relies on MSVC-specific lazy template instantiation to support arrays of incomplete types
			// ensure that DebugGet gets instantiated.
			//@todo it would be nice if we had a cleaner solution for DebugGet
			volatile const ElementType* Dummy = &DebugGet(0);
#endif
		}

		/**
		* Helper function for returning a typed pointer to the first array entry.
		*
		* @returns Pointer to first array entry or nullptr if mCapacity == 0.
		*/
		__forceinline ElementType* Data()
		{
			return (ElementType*)AllocatorInstance.GetAllocation();
		}

		/**
		* Helper function for returning a typed pointer to the first array entry.
		*
		* @returns Pointer to first array entry or nullptr if mCapacity == 0.
		*/
		__forceinline const ElementType* Data() const
		{
			return (const ElementType*)AllocatorInstance.GetAllocation();
		}

		/**
		* Helper function returning the size of the inner type.
		*
		* @returns Size in bytes of array type.
		*/
		__forceinline uint32 GetTypeSize() const
		{
			return sizeof(ElementType);
		}

		/**
		* Helper function to return the amount of memory allocated by this
		* container.
		*
		* @returns Number of bytes allocated by this container.
		*/
		__forceinline uint32 GetAllocatedSize(void) const
		{
			return AllocatorInstance.GetAllocatedSize(mCapacity, sizeof(ElementType));
		}

		/**
		* Returns the amount of slack in this array in elements.
		*
		* @see Num, Shrink
		*/
		__forceinline int32 GetSlack() const
		{
			return mCapacity - mSize;
		}

		/**
		* Checks array invariants: if array size is greater than zero and less
		* than maximum.
		*/
		__forceinline void CheckInvariants() const
		{
			Assert((mSize >= 0) & (mCapacity >= mSize)); // & for one branch
		}

		/**
		* Checks if index is in array range.
		*
		* @param Index Index to check.
		*/
		__forceinline void RangeCheck(int32 Index) const
		{
			CheckInvariants();

			// Template property, branch will be optimized out
			if (Allocator::RequireRangeCheck)
			{
				Assertf((Index >= 0) & (Index < mSize), EDX_TEXT("Array index out of bounds: %i from an array of size %i"), Index, mSize); // & for one branch
			}
		}

		/**
		* Tests if index is valid, i.e. greater than or equal to zero, and less than the number of elements in the array.
		*
		* @param Index Index to test.
		* @returns True if index is valid. False otherwise.
		*/
		__forceinline bool IsValidIndex(int32 Index) const
		{
			return Index >= 0 && Index < mSize;
		}

		/**
		* Returns number of elements in array.
		*
		* @returns Number of elements in array.
		* @see GetSlack
		*/
		__forceinline int32 Size() const
		{
			return mSize;
		}

		/**
		* Returns maximum number of elements in array.
		*
		* @returns Maximum number of elements in array.
		* @see GetSlack
		*/
		__forceinline int32 Capacity() const
		{
			return mCapacity;
		}

		/**
		* Returns whether the array is empty.
		*
		* @returns whether the array is empty.
		*/
		__forceinline bool Empty() const
		{
			return mSize > 0;
		}

		/**
		* Array bracket operator. Returns reference to element at give index.
		*
		* @returns Reference to indexed element.
		*/
		__forceinline ElementType& operator[](int32 Index)
		{
			RangeCheck(Index);
			return Data()[Index];
		}

		/**
		* Array bracket operator. Returns reference to element at give index.
		*
		* Const version of the above.
		*
		* @returns Reference to indexed element.
		*/
		__forceinline const ElementType& operator[](int32 Index) const
		{
			RangeCheck(Index);
			return Data()[Index];
		}

		/**
		* Pops element from the array.
		*
		* @param bAllowShrinking If this call allows shrinking of the array during element remove.
		* @returns Popped element.
		*/
		__forceinline ElementType Pop(bool bAllowShrinking = true)
		{
			RangeCheck(0);
			ElementType Result = Move(Data()[mSize - 1]);
			RemoveAt(mSize - 1, 1, bAllowShrinking);
			return Result;
		}

		/**
		* Pushes element into the array.
		*
		* @param Item Item to push.
		*/
		__forceinline void Push(ElementType&& Item)
		{
			Add(Move(Item));
		}

		/**
		* Pushes element into the array.
		*
		* Const ref version of the above.
		*
		* @param Item Item to push.
		* @see Pop, Top
		*/
		__forceinline void Push(const ElementType& Item)
		{
			Add(Item);
		}

		/**
		* Returns the top element, i.e. the last one.
		*
		* @returns Reference to the top element.
		* @see Pop, Push
		*/
		__forceinline ElementType& Top()
		{
			return Last();
		}

		/**
		* Returns the top element, i.e. the last one.
		*
		* Const version of the above.
		*
		* @returns Reference to the top element.
		* @see Pop, Push
		*/
		__forceinline const ElementType& Top() const
		{
			return Last();
		}

		/**
		* Returns n-th last element from the array.
		*
		* @param IndexFromTheEnd (Optional) Index from the end of array (default = 0).
		* @returns Reference to n-th last element from the array.
		*/
		__forceinline ElementType& Last(int32 IndexFromTheEnd = 0)
		{
			RangeCheck(mSize - IndexFromTheEnd - 1);
			return Data()[mSize - IndexFromTheEnd - 1];
		}

		/**
		* Returns n-th last element from the array.
		*
		* Const version of the above.
		*
		* @param IndexFromTheEnd (Optional) Index from the end of array (default = 0).
		* @returns Reference to n-th last element from the array.
		*/
		__forceinline const ElementType& Last(int32 IndexFromTheEnd = 0) const
		{
			RangeCheck(mSize - IndexFromTheEnd - 1);
			return Data()[mSize - IndexFromTheEnd - 1];
		}

		/**
		* Shrinks the array's used memory to smallest possible to store elements currently in it.
		*
		* @see Slack
		*/
		__forceinline void Shrink()
		{
			CheckInvariants();
			if (mCapacity != mSize)
			{
				ResizeTo(mSize);
			}
		}

		/**
		* Finds element within the array.
		*
		* @param Item Item to look for.
		* @param Index Will contain the found index.
		* @returns True if found. False otherwise.
		* @see FindLast, FindLastByPredicate
		*/
		__forceinline bool Find(const ElementType& Item, int32& Index) const
		{
			Index = this->Find(Item);
			return Index != INDEX_NONE;
		}

		/**
		* Finds element within the array.
		*
		* @param Item Item to look for.
		* @returns Index of the found element. INDEX_NONE otherwise.
		* @see FindLast, FindLastByPredicate
		*/
		int32 Find(const ElementType& Item) const
		{
			const ElementType* __restrict Start = Data();
			for (const ElementType* __restrict Data = Start, *__restrict DataEnd = Data + mSize; Data != DataEnd; ++Data)
			{
				if (*Data == Item)
				{
					return static_cast<int32>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		/**
		* Finds element within the array starting from the end.
		*
		* @param Item Item to look for.
		* @param Index Output parameter. Found index.
		* @returns True if found. False otherwise.
		* @see Find, FindLastByPredicate
		*/
		__forceinline bool FindLast(const ElementType& Item, int32& Index) const
		{
			Index = this->FindLast(Item);
			return Index != INDEX_NONE;
		}

		/**
		* Finds element within the array starting from the end.
		*
		* @param Item Item to look for.
		* @returns Index of the found element. INDEX_NONE otherwise.
		*/
		int32 FindLast(const ElementType& Item) const
		{
			for (const ElementType* __restrict Start = Data(), *__restrict Data = Start + mSize; Data != Start; )
			{
				--Data;
				if (*Data == Item)
				{
					return static_cast<int32>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		/**
		* Finds element within the array starting from StartIndex and going backwards. Uses predicate to match element.
		*
		* @param Pred Predicate taking array element and returns true if element matches search criteria, false otherwise.
		* @param StartIndex Index of element from which to start searching.
		* @returns Index of the found element. INDEX_NONE otherwise.
		*/
		template <typename Predicate>
		int32 FindLastByPredicate(Predicate Pred, int32 StartIndex) const
		{
			Assert(StartIndex >= 0 && StartIndex <= this->Size());
			for (const ElementType* __restrict Start = Data(), *__restrict Data = Start + StartIndex; Data != Start; )
			{
				--Data;
				if (Pred(*Data))
				{
					return static_cast<int32>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		/**
		* Finds element within the array starting from the end. Uses predicate to match element.
		*
		* @param Pred Predicate taking array element and returns true if element matches search criteria, false otherwise.
		* @returns Index of the found element. INDEX_NONE otherwise.
		*/
		template <typename Predicate>
		__forceinline int32 FindLastByPredicate(Predicate Pred) const
		{
			return FindLastByPredicate(Pred, mSize);
		}

		/**
		* Finds an item by key (assuming the ElementType overloads operator== for
		* the comparison).
		*
		* @param Key The key to search by.
		* @returns Index to the first matching element, or INDEX_NONE if none is found.
		*/
		template <typename KeyType>
		int32 IndexOfByKey(const KeyType& Key) const
		{
			const ElementType* __restrict Start = Data();
			for (const ElementType* __restrict Data = Start, *__restrict DataEnd = Start + mSize; Data != DataEnd; ++Data)
			{
				if (*Data == Key)
				{
					return static_cast<int32>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		/**
		* Finds an item by predicate.
		*
		* @param Pred The predicate to match.
		* @returns Index to the first matching element, or INDEX_NONE if none is found.
		*/
		template <typename Predicate>
		int32 IndexOfByPredicate(Predicate Pred) const
		{
			const ElementType* __restrict Start = Data();
			for (const ElementType* __restrict Data = Start, *__restrict DataEnd = Start + mSize; Data != DataEnd; ++Data)
			{
				if (Pred(*Data))
				{
					return static_cast<int32>(Data - Start);
				}
			}
			return INDEX_NONE;
		}

		/**
		* Finds an item by key (assuming the ElementType overloads operator== for
		* the comparison).
		*
		* @param Key The key to search by.
		* @returns Pointer to the first matching element, or nullptr if none is found.
		* @see Find
		*/
		template <typename KeyType>
		__forceinline const ElementType* FindByKey(const KeyType& Key) const
		{
			return const_cast<Array*>(this)->FindByKey(Key);
		}

		/**
		* Finds an item by key (assuming the ElementType overloads operator== for
		* the comparison). Time Complexity: O(n), starts iteration from the beginning so better performance if Key is in the front
		*
		* @param Key The key to search by.
		* @returns Pointer to the first matching element, or nullptr if none is found.
		* @see Find
		*/
		template <typename KeyType>
		ElementType* FindByKey(const KeyType& Key)
		{
			for (ElementType* __restrict Data = Data(), *__restrict DataEnd = Data + mSize; Data != DataEnd; ++Data)
			{
				if (*Data == Key)
				{
					return Data;
				}
			}

			return nullptr;
		}

		/**
		* Finds an element which matches a predicate functor.
		*
		* @param Pred The functor to apply to each element.
		* @returns Pointer to the first element for which the predicate returns true, or nullptr if none is found.
		* @see FilterByPredicate, ContainsByPredicate
		*/
		template <typename Predicate>
		__forceinline const ElementType* FindByPredicate(Predicate Pred) const
		{
			return const_cast<Array*>(this)->FindByPredicate(Pred);
		}

		/**
		* Finds an element which matches a predicate functor.
		*
		* @param Pred The functor to apply to each element. true, or nullptr if none is found.
		* @see FilterByPredicate, ContainsByPredicate
		*/
		template <typename Predicate>
		ElementType* FindByPredicate(Predicate Pred)
		{
			for (ElementType* __restrict IterData = Data(), *__restrict DataEnd = IterData + mSize; IterData != DataEnd; ++IterData)
			{
				if (Pred(*IterData))
				{
					return IterData;
				}
			}

			return nullptr;
		}

		/**
		* Filters the elements in the array based on a predicate functor.
		*
		* @param Pred The functor to apply to each element.
		* @returns Array with the same type as this object which contains
		*          the subset of elements for which the functor returns true.
		* @see FindByPredicate, ContainsByPredicate
		*/
		template <typename Predicate>
		Array<ElementType> FilterByPredicate(Predicate Pred) const
		{
			Array<ElementType> FilterResults;
			for (const ElementType* __restrict IterData = Data(), *__restrict DataEnd = IterData + mSize; IterData != DataEnd; ++IterData)
			{
				if (Pred(*IterData))
				{
					FilterResults.Add(*IterData);
				}
			}
			return FilterResults;
		}

		/**
		* Checks if this array contains the element.
		*
		* @returns	True if found. False otherwise.
		* @see ContainsByPredicate, FilterByPredicate, FindByPredicate
		*/
		template <typename ComparisonType>
		bool Contains(const ComparisonType& Item) const
		{
			for (const ElementType* __restrict IterData = Data(), *__restrict DataEnd = IterData + mSize; IterData != DataEnd; ++IterData)
			{
				if (*IterData == Item)
				{
					return true;
				}
			}
			return false;
		}

		/**
		* Checks if this array contains element for which the predicate is true.
		*
		* @param Predicate to use
		* @returns	True if found. False otherwise.
		* @see Contains, Find
		*/
		template <typename Predicate>
		__forceinline bool ContainsByPredicate(Predicate Pred) const
		{
			return FindByPredicate(Pred) != nullptr;
		}

		/**
		* Equality operator.
		*
		* @param OtherArray Array to compare.
		* @returns True if this array is the same as OtherArray. False otherwise.
		*/
		bool operator==(const Array& OtherArray) const
		{
			int32 Count = Size();

			return Count == OtherArray.Size() && CompareItems(Data(), OtherArray.Data(), Count);
		}

		/**
		* Inequality operator.
		*
		* @param OtherArray Array to compare.
		* @returns True if this array is NOT the same as OtherArray. False otherwise.
		*/
		__forceinline bool operator!=(const Array& OtherArray) const
		{
			return !(*this == OtherArray);
		}

		///**
		//* Serialization operator.
		//*
		//* @param Ar Archive to serialize the array with.
		//* @param A Array to serialize.
		//* @returns Passing the given archive.
		//*/
		//friend FArchive& operator<<(FArchive& Ar, Array& A)
		//{
		//	A.CountBytes(Ar);
		//	if (sizeof(ElementType) == 1)
		//	{
		//		// Serialize simple bytes which require no construction or destruction.
		//		Ar << A.mSize;
		//		check(A.mSize >= 0);
		//		if ((A.mSize || A.mCapacity) && Ar.IsLoading())
		//		{
		//			A.ResizeForCopy(A.mSize, A.mCapacity);
		//		}
		//		Ar.Serialize(A.Data(), A.Size());
		//	}
		//	else if (Ar.IsLoading())
		//	{
		//		// Load array.
		//		int32 NewNum;
		//		Ar << NewNum;
		//		A.Clear(NewNum);
		//		for (int32 i = 0; i < NewNum; i++)
		//		{
		//			Ar << *::new(A)ElementType;
		//		}
		//	}
		//	else
		//	{
		//		// Save array.
		//		Ar << A.mSize;
		//		for (int32 i = 0; i < A.mSize; i++)
		//		{
		//			Ar << A[i];
		//		}
		//	}
		//	return Ar;
		//}

		///**
		//* Bulk serialize array as a single memory blob when loading. Uses regular serialization code for saving
		//* and doesn't serialize at all otherwise (e.g. transient, garbage collection, ...).
		//*
		//* Requirements:
		//*   - T's << operator needs to serialize ALL member variables in the SAME order they are layed out in memory.
		//*   - T's << operator can NOT perform any fixup operations. This limitation can be lifted by manually copying
		//*     the code after the BulkSerialize call.
		//*   - T can NOT contain any member variables requiring constructor calls or pointers
		//*   - sizeof(ElementType) must be equal to the sum of sizes of it's member variables.
		//*        - e.g. use pragma pack (push,1)/ (pop) to ensure alignment
		//*        - match up uint8/ WORDs so everything always end up being properly aligned
		//*   - Code can not rely on serialization of T if neither ArIsLoading nor ArIsSaving is true.
		//*   - Can only be called platforms that either have the same endianness as the one the content was saved with
		//*     or had the endian conversion occur in a cooking process like e.g. for consoles.
		//*
		//* Notes:
		//*   - it is safe to call BulkSerialize on TTransArrays
		//*
		//* IMPORTANT:
		//*   - This is Overridden in XeD3dResourceArray.h Please make certain changes are propogated accordingly
		//*
		//* @param Ar	FArchive to bulk serialize this Array to/from
		//*/
		//void BulkSerialize(FArchive& Ar, bool bForcePerElementSerialization = false)
		//{
		//	int32 ElementSize = sizeof(ElementType);
		//	// Serialize element size to detect mismatch across platforms.
		//	int32 SerializedElementSize = ElementSize;
		//	Ar << SerializedElementSize;

		//	if (bForcePerElementSerialization
		//		|| (Ar.IsSaving()			// if we are saving, we always do the ordinary serialize as a way to make sure it matches up with bulk serialization
		//			&& !Ar.IsCooking()			// but cooking and transacting is performance critical, so we skip that
		//			&& !Ar.IsTransacting())
		//		|| Ar.IsByteSwapping()		// if we are byteswapping, we need to do that per-element
		//		)
		//	{
		//		Ar << *this;
		//	}
		//	else
		//	{
		//		CountBytes(Ar);
		//		if (Ar.IsLoading())
		//		{
		//			// Basic sanity checking to ensure that sizes match.
		//			checkf(SerializedElementSize == 0 || SerializedElementSize == ElementSize, EDX_TEXT("Expected %i, Got: %i"), ElementSize, SerializedElementSize);
		//			// Serialize the number of elements, block allocate the right amount of memory and deserialize
		//			// the data as a giant memory blob in a single call to Serialize. Please see the function header
		//			// for detailed documentation on limitations and implications.
		//			int32 NewmSize;
		//			Ar << NewmSize;
		//			Clear(NewmSize);
		//			AddUninitialized(NewmSize);
		//			Ar.Serialize(Data(), NewmSize * SerializedElementSize);
		//		}
		//		else if (Ar.IsSaving())
		//		{
		//			int32 ArrayCount = Size();
		//			Ar << ArrayCount;
		//			Ar.Serialize(Data(), ArrayCount * SerializedElementSize);
		//		}
		//	}
		//}

		///**
		//* Count bytes needed to serialize this array.
		//*
		//* @param Ar Archive to count for.
		//*/
		//void CountBytes(FArchive& Ar)
		//{
		//	Ar.CountBytes(mSize * sizeof(ElementType), mCapacity * sizeof(ElementType));
		//}

		/**
		* Adds a given number of uninitialized elements into the array.
		*
		* Caution, AddUninitialized() will create elements without calling
		* the constructor and this is not appropriate for element types that
		* require a constructor to function properly.
		*
		* @param Count Number of elements to add.
		* @returns Number of elements in array before addition.
		*/
		__forceinline int32 AddUninitialized(int32 Count = 1)
		{
			CheckInvariants();
			Assert(Count >= 0);

			const int32 OldNum = mSize;
			if ((mSize += Count) > mCapacity)
			{
				ResizeGrow(OldNum);
			}
			return OldNum;
		}

		/**
		* Inserts a given number of uninitialized elements into the array at given
		* location.
		*
		* Caution, InsertUninitialized() will create elements without calling the
		* constructor and this is not appropriate for element types that require
		* a constructor to function properly.
		*
		* @param Index Tells where to insert the new elements.
		* @param Count Number of elements to add.
		*/
		void InsertUninitialized(int32 Index, int32 Count = 1)
		{
			CheckInvariants();
			Assert((Count >= 0) & (Index >= 0) & (Index <= mSize));

			const int32 OldNum = mSize;
			if ((mSize += Count) > mCapacity)
			{
				ResizeGrow(OldNum);
			}
			ElementType* OffsetData = Data() + Index;
			RelocateConstructItems<ElementType>(OffsetData + Count, OffsetData, OldNum - Index);
		}

		/**
		* Inserts a given number of zeroed elements into the array at given
		* location.
		*
		* Caution, InsertZeroed() will create elements without calling the
		* constructor and this is not appropriate for element types that require
		* a constructor to function properly.
		*
		* @param Index Tells where to insert the new elements.
		* @param Count Number of elements to add.
		* @see Insert, InsertUninitialized
		*/
		void InsertZeroed(int32 Index, int32 Count = 1)
		{
			InsertUninitialized(Index, Count);
			Memory::Memzero((uint8*)AllocatorInstance.GetAllocation() + Index * sizeof(ElementType), Count * sizeof(ElementType));
		}

		/**
		* Inserts given elements into the array at given location.
		*
		* @param Items Array of elements to insert.
		* @param InIndex Tells where to insert the new elements.
		* @returns Location at which the item was inserted.
		*/
		int32 Insert(std::initializer_list<ElementType> InitList, const int32 InIndex)
		{
			InsertUninitialized(InIndex, (int32)InitList.size());

			int32 Index = InIndex;
			for (const ElementType& Element : InitList)
			{
				new (Data() + Index++) ElementType(Element);
			}
			return InIndex;
		}

		/**
		* Inserts given elements into the array at given location.
		*
		* @param Items Array of elements to insert.
		* @param InIndex Tells where to insert the new elements.
		* @returns Location at which the item was inserted.
		*/
		int32 Insert(const Array<ElementType>& Items, const int32 InIndex)
		{
			Assert(this != &Items);
			InsertUninitialized(InIndex, Items.Size());
			int32 Index = InIndex;
			for (auto It = Items.CreateConstIterator(); It; ++It)
			{
				RangeCheck(Index);
				new(Data() + Index++) ElementType(Move(*It));
			}
			return InIndex;
		}

		/**
		* Inserts a raw array of elements at a particular index in the Array.
		*
		* @param Ptr A pointer to an array of elements to add.
		* @param Count The number of elements to insert from Ptr.
		* @param Index The index to insert the elements at.
		* @return The index of the first element inserted.
		* @see Add, Remove
		*/
		int32 Insert(const ElementType* Ptr, int32 Count, int32 Index)
		{
			Assert(Ptr != nullptr);

			InsertUninitialized(Index, Count);
			ConstructItems<ElementType>(Data() + Index, Ptr, Count);

			return Index;
		}

		/**
		* Checks that the specified address is not part of an element within the
		* container. Used for implementations to check that reference arguments
		* aren't going to be invalidated by possible reallocation.
		*
		* @param Addr The address to check.
		* @see Add, Remove
		*/
		__forceinline void CheckAddress(const ElementType* Addr) const
		{
			Assertf(Addr < Data() || Addr >= (Data() + mCapacity), EDX_TEXT("Attempting to add a container element (0x%08x) which already comes from the container (0x%08x, mCapacity: %d)!"), Addr, Data(), mCapacity);
		}

		/**
		* Inserts a given element into the array at given location. Move semantics
		* version.
		*
		* @param Item The element to insert.
		* @param Index Tells where to insert the new elements.
		* @returns Location at which the insert was done.
		* @see Add, Remove
		*/
		int32 Insert(ElementType&& Item, int32 Index)
		{
			CheckAddress(&Item);

			// construct a copy in place at Index (this new operator will insert at 
			// Index, then construct that memory with Item)
			InsertUninitialized(Index, 1);
			new(Data() + Index) ElementType(Move(Item));
			return Index;
		}

		/**
		* Inserts a given element into the array at given location.
		*
		* @param Item The element to insert.
		* @param Index Tells where to insert the new elements.
		* @returns Location at which the insert was done.
		* @see Add, Remove
		*/
		int32 Insert(const ElementType& Item, int32 Index)
		{
			CheckAddress(&Item);

			// construct a copy in place at Index (this new operator will insert at 
			// Index, then construct that memory with Item)
			InsertUninitialized(Index, 1);
			new(Data() + Index) ElementType(Item);
			return Index;
		}

		/**
		* Removes an element (or elements) at given location optionally shrinking
		* the array.
		*
		* @param Index Location in array of the element to remove.
		* @param Count (Optional) Number of elements to remove. Default is 1.
		* @param bAllowShrinking (Optional) Tells if this call can shrink array if suitable after remove. Default is true.
		*/
		void RemoveAt(int32 Index, int32 Count = 1, bool bAllowShrinking = true)
		{
			if (Count)
			{
				CheckInvariants();
				Assert((Count >= 0) & (Index >= 0) & (Index + Count <= mSize));

				DestructItems(Data() + Index, Count);

				// Skip memmove in the common case that there is nothing to move.
				int32 NumToMove = mSize - Index - Count;
				if (NumToMove)
				{
					Memory::Memmove
					(
						(uint8*)AllocatorInstance.GetAllocation() + (Index) * sizeof(ElementType),
						(uint8*)AllocatorInstance.GetAllocation() + (Index + Count) * sizeof(ElementType),
						NumToMove * sizeof(ElementType)
					);
				}
				mSize -= Count;

				if (bAllowShrinking)
				{
					ResizeShrink();
				}
			}
		}

		/**
		* Removes an element (or elements) at given location optionally shrinking
		* the array.
		*
		* This version is much more efficient than RemoveAt (O(Count) instead of
		* O(mSize)), but does not preserve the order.
		*
		* @param Index Location in array of the element to remove.
		* @param Count (Optional) Number of elements to remove. Default is 1.
		* @param bAllowShrinking (Optional) Tells if this call can shrink array if
		*                        suitable after remove. Default is true.
		*/
		void RemoveAtSwap(int32 Index, int32 Count = 1, bool bAllowShrinking = true)
		{
			if (Count)
			{
				CheckInvariants();
				Assert((Count >= 0) & (Index >= 0) & (Index + Count <= mSize));

				DestructItems(Data() + Index, Count);

				// Replace the elements in the hole created by the removal with elements from the end of the array, so the range of indices used by the array is contiguous.
				const int32 NumElementsInHole = Count;
				const int32 NumElementsAfterHole = mSize - (Index + Count);
				const int32 NumElementsToMoveIntoHole = Math::Min(NumElementsInHole, NumElementsAfterHole);
				if (NumElementsToMoveIntoHole)
				{
					Memory::Memcpy(
						(uint8*)AllocatorInstance.GetAllocation() + (Index) * sizeof(ElementType),
						(uint8*)AllocatorInstance.GetAllocation() + (mSize - NumElementsToMoveIntoHole) * sizeof(ElementType),
						NumElementsToMoveIntoHole * sizeof(ElementType)
					);
				}
				mSize -= Count;

				if (bAllowShrinking)
				{
					ResizeShrink();
				}
			}
		}

		/**
		* Same as empty, but doesn't change memory allocations, unless the new size is larger than
		* the current array. It calls the destructors on held items if needed and then zeros the mSize.
		*
		* @param NewSize The expected usage size after calling this function.
		*/
		void Reset(int32 NewSize = 0)
		{
			// If we have space to hold the excepted size, then don't reallocate
			if (NewSize <= mCapacity)
			{
				DestructItems(Data(), mSize);
				mSize = 0;
			}
			else
			{
				Clear(NewSize);
			}
		}

		/**
		* Empties the array. It calls the destructors on held items if needed.
		*
		* @param Slack (Optional) The expected usage size after empty operation. Default is 0.
		*/
		void Clear(int32 Slack = 0)
		{
			DestructItems(Data(), mSize);

			Assert(Slack >= 0);
			mSize = 0;

			if (mCapacity != Slack)
			{
				ResizeTo(Slack);
			}
		}

		/**
		* Resizes array to given number of elements.
		*
		* @param NewNum New size of the array.
		* @param bAllowShrinking Tell if this function can shrink the memory in-use if suitable.
		*/
		void Resize(int32 NewNum, bool bAllowShrinking = true)
		{
			if (NewNum > Size())
			{
				const int32 Diff = NewNum - mSize;
				const int32 Index = AddUninitialized(Diff);
				DefaultConstructItems<ElementType>((uint8*)AllocatorInstance.GetAllocation() + Index * sizeof(ElementType), Diff);
			}
			else if (NewNum < Size())
			{
				RemoveAt(NewNum, Size() - NewNum, bAllowShrinking);
			}
		}

		/**
		* Resizes array to given number of elements. New elements will be zeroed.
		*
		* @param NewNum New size of the array.
		*/
		void ResizeZeroed(int32 NewNum, bool bAllowShrinking = true)
		{
			if (NewNum > Size())
			{
				AddZeroed(NewNum - Size());
			}
			else if (NewNum < Size())
			{
				RemoveAt(NewNum, Size() - NewNum, bAllowShrinking);
			}
		}

		/**
		* Resizes array to given number of elements. New elements will be uninitialized.
		*
		* @param NewNum New size of the array.
		*/
		void ResizeUninitialized(int32 NewNum, bool bAllowShrinking = true)
		{
			if (NewNum > Size())
			{
				AddUninitialized(NewNum - Size());
			}
			else if (NewNum < Size())
			{
				RemoveAt(NewNum, Size() - NewNum, bAllowShrinking);
			}
		}

		/**
		* Appends the specified array to this array.
		*
		* Allocator changing version.
		*
		* @param Source The array to append.
		* @see Add, Insert
		*/
		template <typename OtherElementType, typename OtherAllocator>
		void Append(const Array<OtherElementType, OtherAllocator>& Source)
		{
			Assert((void*)this != (void*)&Source);

			int32 SourceCount = Source.Size();

			// Do nothing if the source is empty.
			if (!SourceCount)
			{
				return;
			}

			// Allocate memory for the new elements.
			Reserve(mSize + SourceCount);
			ConstructItems<ElementType>(Data() + mSize, Source.Data(), SourceCount);

			mSize += SourceCount;
		}

		/**
		* Appends the specified array to this array.
		*
		* @param Source The array to append.
		* @see Add, Insert
		*/
		template <typename OtherElementType, typename OtherAllocator>
		void Append(Array<OtherElementType, OtherAllocator>&& Source)
		{
			Assert((void*)this != (void*)&Source);

			int32 SourceCount = Source.Size();

			// Do nothing if the source is empty.
			if (!SourceCount)
			{
				return;
			}

			// Allocate memory for the new elements.
			Reserve(mSize + SourceCount);
			RelocateConstructItems<ElementType>(Data() + mSize, Source.Data(), SourceCount);
			Source.mSize = 0;

			mSize += SourceCount;
		}

		/**
		* Adds a raw array of elements to the end of the Array.
		*
		* @param Ptr   A pointer to an array of elements to add.
		* @param Count The number of elements to insert from Ptr.
		* @see Add, Insert
		*/
		void Append(const ElementType* Ptr, int32 Count)
		{
			Assert(Ptr != nullptr);

			int32 Pos = AddUninitialized(Count);
			ConstructItems<ElementType>(Data() + Pos, Ptr, Count);
		}

		/**
		* Adds an initializer list of elements to the end of the TArray.
		*
		* @param InitList The initializer list of elements to add.
		* @see Add, Insert
		*/
		FORCEINLINE void Append(std::initializer_list<ElementType> InitList)
		{
			int32 Count = (int32)InitList.size();

			int32 Pos = AddUninitialized(Count);
			ConstructItems<ElementType>(Data() + Pos, InitList.begin(), Count);
		}


		/**
		* Appends the specified array to this array.
		* Cannot append to self.
		*
		* Move semantics version.
		*
		* @param Other The array to append.
		*/
		Array& operator+=(Array&& Other)
		{
			Append(Move(Other));
			return *this;
		}

		/**
		* Appends the specified array to this array.
		* Cannot append to self.
		*
		* @param Other The array to append.
		*/
		Array& operator+=(const Array& Other)
		{
			Append(Other);
			return *this;
		}

		/**
		* Adds a new item to the end of the array, possibly reallocating the whole array to fit.
		*
		* @param Item	The item to add
		* @return		Index to the new item
		*/
		template <typename... ArgsType>
		__forceinline int32 Emplace(ArgsType&&... Args)
		{
			const int32 Index = AddUninitialized(1);
			new(Data() + Index) ElementType(Forward<ArgsType>(Args)...);
			return Index;
		}

		/**
		* Adds a new item to the end of the array, possibly reallocating the whole array to fit.
		*
		* Move semantics version.
		*
		* @param Item The item to add
		* @return Index to the new item
		* @see AddDefaulted, AddUnique, AddZeroed, Append, Insert
		*/
		__forceinline int32 Add(ElementType&& Item) { CheckAddress(&Item); return Emplace(Move(Item)); }

		/**
		* Adds a new item to the end of the array, possibly reallocating the whole array to fit.
		*
		* @param Item The item to add
		* @return Index to the new item
		* @see AddDefaulted, AddUnique, AddZeroed, Append, Insert
		*/
		__forceinline int32 Add(const ElementType& Item) { CheckAddress(&Item); return Emplace(Item); }

		/**
		* Adds new items to the end of the array, possibly reallocating the whole
		* array to fit. The new items will be zeroed.
		*
		* Caution, AddZeroed() will create elements without calling the
		* constructor and this is not appropriate for element types that require
		* a constructor to function properly.
		*
		* @param  Count  The number of new items to add.
		* @return Index to the first of the new items.
		* @see Add, AddDefaulted, AddUnique, Append, Insert
		*/
		int32 AddZeroed(int32 Count = 1)
		{
			const int32 Index = AddUninitialized(Count);
			Memory::Memzero((uint8*)AllocatorInstance.GetAllocation() + Index * sizeof(ElementType), Count * sizeof(ElementType));
			return Index;
		}

		/**
		* Adds new items to the end of the array, possibly reallocating the whole
		* array to fit. The new items will be default-constructed.
		*
		* @param  Count  The number of new items to add.
		* @return Index to the first of the new items.
		* @see Add, AddZeroed, AddUnique, Append, Insert
		*/
		int32 AddDefaulted(int32 Count = 1)
		{
			const int32 Index = AddUninitialized(Count);
			DefaultConstructItems<ElementType>((uint8*)AllocatorInstance.GetAllocation() + Index * sizeof(ElementType), Count);
			return Index;
		}

	private:

		/**
		* Adds unique element to array if it doesn't exist.
		*
		* @param Args Item to add.
		* @returns Index of the element in the array.
		*/
		template <typename ArgsType>
		int32 AddUniqueImpl(ArgsType&& Args)
		{
			int32 Index;
			if (Find(Args, Index))
			{
				return Index;
			}

			return Add(Forward<ArgsType>(Args));
		}

	public:

		/**
		* Adds unique element to array if it doesn't exist.
		*
		* Move semantics version.
		*
		* @param Args Item to add.
		* @returns Index of the element in the array.
		* @see Add, AddDefaulted, AddZeroed, Append, Insert
		*/
		__forceinline int32 AddUnique(ElementType&& Item) { return AddUniqueImpl(Move(Item)); }

		/**
		* Adds unique element to array if it doesn't exist.
		*
		* @param Args Item to add.
		* @returns Index of the element in the array.
		* @see Add, AddDefaulted, AddZeroed, Append, Insert
		*/
		__forceinline int32 AddUnique(const ElementType& Item) { return AddUniqueImpl(Item); }

		/**
		* Reserves memory such that the array can contain at least Number elements.
		*
		* @param Number The number of elements that the array should be able to contain after allocation.
		* @see Shrink
		*/
		__forceinline void Reserve(int32 Number)
		{
			if (Number > mCapacity)
			{
				ResizeTo(Number);
			}
		}

		/**
		* Sets the size of the array, filling it with the given element.
		*
		* @param Element The element to fill array with.
		* @param Number The number of elements that the array should be able to contain after allocation.
		*/
		void Init(const ElementType& Element, int32 Number)
		{
			Clear(Number);
			for (int32 Index = 0; Index < Number; ++Index)
			{
				new(*this) ElementType(Element);
			}
		}

		/**
		* Removes the first occurrence of the specified item in the array,
		* maintaining order but not indices.
		*
		* @param Item The item to remove.
		* @returns The number of items removed. For RemoveSingleItem, this is always either 0 or 1.
		* @see Add, Insert, Remove, RemoveAll, RemoveAllSwap
		*/
		int32 RemoveSingle(const ElementType& Item)
		{
			int32 Index = Find(Item);
			if (Index == INDEX_NONE)
			{
				return 0;
			}

			auto* RemovePtr = Data() + Index;

			// Destruct items that match the specified Item.
			DestructItems(RemovePtr, 1);
			const int32 NextIndex = Index + 1;
			RelocateConstructItems<ElementType>(RemovePtr, RemovePtr + 1, mSize - (Index + 1));

			// Update the array count
			--mSize;

			// Removed one item
			return 1;
		}

		/**
		* Removes as many instances of Item as there are in the array, maintaining
		* order but not indices.
		*
		* @param Item Item to remove from array.
		* @returns Number of removed elements.
		* @see Add, Insert, RemoveAll, RemoveAllSwap, RemoveSingle, RemoveSwap
		*/
		int32 Remove(const ElementType& Item)
		{
			CheckAddress(&Item);

			// Element is non-const to preserve compatibility with existing code with a non-const operator==() member function
			return RemoveAll([&Item](ElementType& Element) { return Element == Item; });
		}

		/**
		* Remove all instances that match the predicate, maintaining order but not indices
		* Optimized to work with runs of matches/non-matches
		*
		* @param Predicate Predicate class instance
		* @returns Number of removed elements.
		* @see Add, Insert, RemoveAllSwap, RemoveSingle, RemoveSwap
		*/
		template <class PREDICATE_CLASS>
		int32 RemoveAll(const PREDICATE_CLASS& Predicate)
		{
			const int32 OriginalNum = mSize;
			if (!OriginalNum)
			{
				return 0; // nothing to do, loop assumes one item so need to deal with this edge case here
			}

			int32 WriteIndex = 0;
			int32 ReadIndex = 0;
			bool NotMatch = !Predicate(Data()[ReadIndex]); // use a ! to guarantee it can't be anything other than zero or one
			do
			{
				int32 RunStartIndex = ReadIndex++;
				while (ReadIndex < OriginalNum && NotMatch == !Predicate(Data()[ReadIndex]))
				{
					ReadIndex++;
				}
				int32 RunLength = ReadIndex - RunStartIndex;
				Assert(RunLength > 0);
				if (NotMatch)
				{
					// this was a non-matching run, we need to move it
					if (WriteIndex != RunStartIndex)
					{
						Memory::Memmove(&Data()[WriteIndex], &Data()[RunStartIndex], sizeof(ElementType)* RunLength);
					}
					WriteIndex += RunLength;
				}
				else
				{
					// this was a matching run, delete it
					DestructItems(Data() + RunStartIndex, RunLength);
				}
				NotMatch = !NotMatch;
				} while (ReadIndex < OriginalNum);

				mSize = WriteIndex;
				return OriginalNum - mSize;
			}

		/**
		* Remove all instances that match the predicate
		*
		* @param Predicate Predicate class instance
		* @see Remove, RemoveSingle, RemoveSingleSwap, RemoveSwap
		*/
		template <class PREDICATE_CLASS>
		void RemoveAllSwap(const PREDICATE_CLASS& Predicate, bool bAllowShrinking = true)
		{
			for (int32 ItemIndex = 0; ItemIndex < Size();)
			{
				if (Predicate((*this)[ItemIndex]))
				{
					RemoveAtSwap(ItemIndex, 1, bAllowShrinking);
				}
				else
				{
					++ItemIndex;
				}
			}
		}

		/**
		* Removes the first occurrence of the specified item in the array. This version is much more efficient
		* O(Count) instead of O(mSize), but does not preserve the order
		*
		* @param Item The item to remove
		*
		* @returns The number of items removed. For RemoveSingleItem, this is always either 0 or 1.
		* @see Add, Insert, Remove, RemoveAll, RemoveAllSwap, RemoveSwap
		*/
		int32 RemoveSingleSwap(const ElementType& Item, bool bAllowShrinking = true)
		{
			int32 Index = Find(Item);
			if (Index == INDEX_NONE)
			{
				return 0;
			}

			RemoveAtSwap(Index, 1, bAllowShrinking);

			// Removed one item
			return 1;
		}

		/**
		* Removes item from the array.
		*
		* This version is much more efficient, because it uses RemoveAtSwap
		* internally which is O(Count) instead of RemoveAt which is O(mSize),
		* but does not preserve the order.
		*
		* @returns Number of elements removed.
		* @see Add, Insert, Remove, RemoveAll, RemoveAllSwap
		*/
		int32 RemoveSwap(const ElementType& Item)
		{
			CheckAddress(&Item);

			const int32 OriginalNum = mSize;
			for (int32 Index = 0; Index < mSize; Index++)
			{
				if ((*this)[Index] == Item)
				{
					RemoveAtSwap(Index--);
				}
			}
			return OriginalNum - mSize;
		}

		/**
		* Element-wise array memory swap.
		*
		* @param FirstIndexToSwap Position of the first element to swap.
		* @param SecondIndexToSwap Position of the second element to swap.
		*/
		__forceinline void SwapMemory(int32 FirstIndexToSwap, int32 SecondIndexToSwap)
		{
			Memory::Memswap(
				(uint8*)AllocatorInstance.GetAllocation() + (sizeof(ElementType)*FirstIndexToSwap),
				(uint8*)AllocatorInstance.GetAllocation() + (sizeof(ElementType)*SecondIndexToSwap),
				sizeof(ElementType)
			);
		}

		/**
		* Element-wise array element swap.
		*
		* This version is doing more sanity checks than SwapMemory.
		*
		* @param FirstIndexToSwap Position of the first element to swap.
		* @param SecondIndexToSwap Position of the second element to swap.
		*/
		__forceinline void Swap(int32 FirstIndexToSwap, int32 SecondIndexToSwap)
		{
			Assert((FirstIndexToSwap >= 0) && (SecondIndexToSwap >= 0));
			Assert((mSize > FirstIndexToSwap) && (mSize > SecondIndexToSwap));
			if (FirstIndexToSwap != SecondIndexToSwap)
			{
				SwapMemory(FirstIndexToSwap, SecondIndexToSwap);
			}
		}

		/**
		* Searches for the first entry of the specified type, will only work with
		* Array<UObject*>. Optionally return the item's index, and can specify
		* the start index.
		*
		* @param Item (Optional output) If it's not null, then it will point to
		*             the found element. Untouched if element hasn't been found.
		* @param ItemIndex (Optional output) If it's not null, then it will be set
		*             to the position of found element in the array. Untouched if
		*             element hasn't been found.
		* @param StartIndex (Optional) Index in array at which the function should
		*             start to look for element.
		* @returns True if element was found. False otherwise.
		*/
		template<typename SearchType>
		bool FindItemByClass(SearchType **Item = nullptr, int32 *ItemIndex = nullptr, int32 StartIndex = 0) const
		{
			UClass* SearchClass = SearchType::StaticClass();
			for (int32 Idx = StartIndex; Idx < mSize; Idx++)
			{
				if ((*this)[Idx] != nullptr && (*this)[Idx]->IsA(SearchClass))
				{
					if (Item != nullptr)
					{
						*Item = (SearchType*)((*this)[Idx]);
					}
					if (ItemIndex != nullptr)
					{
						*ItemIndex = Idx;
					}
					return true;
				}
			}
			return false;
		}

		// Iterators
		typedef IndexedContainerIterator<      Array, ElementType, int32> Iterator;
		typedef IndexedContainerIterator<const Array, const ElementType, int32> ConstIterator;

		/**
		* Creates an iterator for the contents of this array
		*
		* @returns The iterator.
		*/
		Iterator CreateIterator()
		{
			return Iterator(*this);
		}

		/**
		* Creates a const iterator for the contents of this array
		*
		* @returns The const iterator.
		*/
		ConstIterator CreateConstIterator() const
		{
			return ConstIterator(*this);
		}

#if TARRAY_RANGED_FOR_CHECKS
		typedef CheckedPointerIterator<      ElementType> RangedForIteratorType;
		typedef CheckedPointerIterator<const ElementType> RangedForConstIteratorType;
#else
		typedef       ElementType* RangedForIteratorType;
		typedef const ElementType* RangedForConstIteratorType;
#endif

	private:

		/**
		* DO NOT USE DIRECTLY
		* STL-like iterators to enable range-based for loop support.
		*/
#if TARRAY_RANGED_FOR_CHECKS
		__forceinline friend RangedForIteratorType      begin(Array& Array) { return RangedForIteratorType(Array.mSize, Array.Data()); }
		__forceinline friend RangedForConstIteratorType begin(const Array& Array) { return RangedForConstIteratorType(Array.mSize, Array.Data()); }
		__forceinline friend RangedForIteratorType      end(Array& Array) { return RangedForIteratorType(Array.mSize, Array.Data() + Array.Size()); }
		__forceinline friend RangedForConstIteratorType end(const Array& Array) { return RangedForConstIteratorType(Array.mSize, Array.Data() + Array.Size()); }
#else
		__forceinline friend RangedForIteratorType      begin(Array& Array) { return Array.Data(); }
		__forceinline friend RangedForConstIteratorType begin(const Array& Array) { return Array.Data(); }
		__forceinline friend RangedForIteratorType      end(Array& Array) { return Array.Data() + Array.Size(); }
		__forceinline friend RangedForConstIteratorType end(const Array& Array) { return Array.Data() + Array.Size(); }
#endif

	public:

		/**
		* Sorts the array assuming < operator is defined for the item type.
		*/
		void Sort()
		{
			::Sort(Data(), Size());
		}

		/**
		* Sorts the array using user define predicate class.
		*
		* @param Predicate Predicate class instance.
		*/
		template <class PREDICATE_CLASS>
		void Sort(const PREDICATE_CLASS& Predicate)
		{
			::Sort(Data(), Size(), Predicate);
		}

		/**
		* Stable sorts the array assuming < operator is defined for the item type.
		*
		* Stable sort is slower than non-stable algorithm.
		*/
		void StableSort()
		{
			::StableSort(Data(), Size());
		}

		/**
		* Stable sorts the array using user defined predicate class.
		*
		* Stable sort is slower than non-stable algorithm.
		*
		* @param Predicate Predicate class instance
		*/
		template <class PREDICATE_CLASS>
		void StableSort(const PREDICATE_CLASS& Predicate)
		{
			::StableSort(Data(), Size(), Predicate);
		}

#if defined(_MSC_VER) && !defined(__clang__)	// Relies on MSVC-specific lazy template instantiation to support arrays of incomplete types
	private:
		/**
		* Helper function that can be used inside the debuggers watch window to debug TArrays. E.g. "*Class->Defaults.DebugGet(5)".
		*
		* @param Index Position to get.
		* @returns Reference to the element at given position.
		*/
		__declspec(noinline) const ElementType& DebugGet(int32 Index) const
		{
			return Data()[Index];
		}
#endif

	private:

		__declspec(noinline) void ResizeGrow(int32 OldNum)
		{
			mCapacity = AllocatorInstance.CalculateSlackGrow(mSize, mCapacity, sizeof(ElementType));
			AllocatorInstance.ResizeAllocation(OldNum, mCapacity, sizeof(ElementType));
		}
		__declspec(noinline) void ResizeShrink()
		{
			const int32 NewmCapacity = AllocatorInstance.CalculateSlackShrink(mSize, mCapacity, sizeof(ElementType));
			if (NewmCapacity != mCapacity)
			{
				mCapacity = NewmCapacity;
				Assert(mCapacity >= mSize);
				AllocatorInstance.ResizeAllocation(mSize, mCapacity, sizeof(ElementType));
			}
		}
		__declspec(noinline) void ResizeTo(int32 NewMax)
		{
			if (NewMax)
			{
				NewMax = AllocatorInstance.CalculateSlackReserve(NewMax, sizeof(ElementType));
			}
			if (NewMax != mCapacity)
			{
				mCapacity = NewMax;
				AllocatorInstance.ResizeAllocation(mSize, mCapacity, sizeof(ElementType));
			}
		}
		__declspec(noinline) void ResizeForCopy(int32 NewMax, int32 PrevMax)
		{
			if (NewMax)
			{
				NewMax = AllocatorInstance.CalculateSlackReserve(NewMax, sizeof(ElementType));
			}
			if (NewMax != PrevMax)
			{
				AllocatorInstance.ResizeAllocation(0, NewMax, sizeof(ElementType));
			}
			mCapacity = NewMax;
		}


		/**
		* Copies data from one array into this array. Uses the fast path if the
		* data in question does not need a constructor.
		*
		* @param Source The source array to copy
		* @param PrevMax The previous allocated size
		* @param ExtraSlack Additional amount of memory to allocate at
		*                   the end of the buffer. Counted in elements. Zero by
		*                   default.
		*/
		template <typename OtherElementType, typename OtherAllocator>
		void CopyToEmpty(const Array<OtherElementType, OtherAllocator>& Source, int32 PrevMax, int32 ExtraSlack)
		{
			Assert(ExtraSlack >= 0);
			mSize = Source.Size();
			if (mSize || ExtraSlack || PrevMax)
			{
				ResizeForCopy(mSize + ExtraSlack, PrevMax);
				ConstructItems<ElementType>(Data(), Source.Data(), mSize);
			}
			else
			{
				mCapacity = 0;
			}
		}

		template <typename OtherElementType>
		void CopyToEmpty(const OtherElementType* OtherData, int32 OtherNum, int32 PrevMax, int32 ExtraSlack)
		{
			Assert(ExtraSlack >= 0);
			mSize = OtherNum;
			if (OtherNum || ExtraSlack || PrevMax)
			{
				ResizeForCopy(OtherNum + ExtraSlack, PrevMax);
				ConstructItems<ElementType>(Data(), OtherData, OtherNum);
			}
			else
			{
				mCapacity = 0;
			}
		}

	protected:

		typedef typename ChooseClass<
			Allocator::NeedsElementType,
			typename Allocator::template ForElementType<ElementType>,
			typename Allocator::ForAnyElementType
		>::Result ElementAllocatorType;

		ElementAllocatorType AllocatorInstance;
		int32	  mSize;
		int32	  mCapacity;

		/**
		* Implicit heaps
		*/

	public:

		/**
		* Builds an implicit heap from the array.
		*
		* @param Predicate Predicate class instance.
		*/
		template <class PREDICATE_CLASS>
		void Heapify(const PREDICATE_CLASS& Predicate)
		{
			DereferenceWrapper< ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			for (int32 Index = HeapGetParentIndex(Size() - 1); Index >= 0; Index--)
			{
				SiftDown(Index, Size(), PredicateWrapper);
			}

#if DEBUG_HEAP
			VerifyHeap(PredicateWrapper);
#endif
		}

		/**
		* Builds an implicit heap from the array. Assumes < operator is defined
		* for the template type.
		*/
		void Heapify()
		{
			Heapify(Less<ElementType>());
		}

		/**
		* Adds a new element to the heap.
		*
		* @param InItem Item to be added.
		* @param Predicate Predicate class instance.
		* @return The index of the new element.
		*/
		template <class PREDICATE_CLASS>
		int32 HeapPush(const ElementType& InItem, const PREDICATE_CLASS& Predicate)
		{
			// Add at the end, then sift up
			Add(InItem);
			DereferenceWrapper<ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			int32 Result = SiftUp(0, Size() - 1, PredicateWrapper);

#if DEBUG_HEAP
			VerifyHeap(PredicateWrapper);
#endif

			return Result;
		}

		/**
		* Adds a new element to the heap. Assumes < operator is defined for the
		* template type.
		*
		* @param InItem Item to be added.
		* @return The index of the new element.
		*/
		int32 HeapPush(const ElementType& InItem)
		{
			return HeapPush(InItem, Less<ElementType>());
		}

		/**
		* Removes the top element from the heap.
		*
		* @param OutItem The removed item.
		* @param Predicate Predicate class instance.
		*/
		template <class PREDICATE_CLASS>
		void HeapPop(ElementType& OutItem, const PREDICATE_CLASS& Predicate, bool bAllowShrinking = true)
		{
			OutItem = (*this)[0];
			RemoveAtSwap(0, 1, bAllowShrinking);

			DereferenceWrapper< ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			SiftDown(0, Size(), PredicateWrapper);

#if DEBUG_HEAP
			VerifyHeap(PredicateWrapper);
#endif
		}

		/**
		* Removes the top element from the heap. Assumes < operator is defined for
		* the template type.
		*
		* @param OutItem The removed item.
		* @param bAllowShrinking (Optional) Tells if this call can shrink the array allocation if suitable after the pop. Default is true.
		*/
		void HeapPop(ElementType& OutItem, bool bAllowShrinking = true)
		{
			HeapPop(OutItem, Less<ElementType>(), bAllowShrinking);
		}

		/**
		* Verifies the heap.
		*
		* @param Predicate Predicate class instance.
		*/
		template <class PREDICATE_CLASS>
		void VerifyHeap(const PREDICATE_CLASS& Predicate)
		{
			// Verify Predicate
			ElementType* Heap = Data();
			for (int32 Index = 1; Index < Size(); Index++)
			{
				int32 ParentIndex = HeapGetParentIndex(Index);
				if (Predicate(Heap[Index], Heap[ParentIndex]))
				{
					Assert(false);
				}
			}
		}

		/**
		* Removes the top element from the heap.
		*
		* @param Predicate Predicate class instance.
		* @param bAllowShrinking (Optional) Tells if this call can shrink the array allocation if suitable after the discard. Default is true.
		*/
		template <class PREDICATE_CLASS>
		void HeapPopDiscard(const PREDICATE_CLASS& Predicate, bool bAllowShrinking = true)
		{
			RemoveAtSwap(0, 1, bAllowShrinking);
			DereferenceWrapper< ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			SiftDown(0, Size(), PredicateWrapper);

#if DEBUG_HEAP
			VerifyHeap(PredicateWrapper);
#endif
		}

		/**
		* Removes the top element from the heap. Assumes < operator is defined for the template type.
		*
		* @param bAllowShrinking (Optional) Tells if this call can shrink the array
		*		allocation if suitable after the discard. Default is true.
		*/
		void HeapPopDiscard(bool bAllowShrinking = true)
		{
			HeapPopDiscard(Less<ElementType>(), bAllowShrinking);
		}

		/**
		* Returns the top element from the heap (does not remove the element).
		*
		* Const version.
		*
		* @returns The reference to the top element from the heap.
		*/
		const ElementType& HeapTop() const
		{
			return (*this)[0];
		}

		/**
		* Returns the top element from the heap (does not remove the element).
		*
		* @returns The reference to the top element from the heap.
		*/
		ElementType& HeapTop()
		{
			return (*this)[0];
		}

		/**
		* Removes an element from the heap.
		*
		* @param Index Position at which to remove item.
		* @param Predicate Predicate class instance.
		* @param bAllowShrinking (Optional) Tells if this call can shrink the array allocation
		*		if suitable after the remove (default = true).
		*/
		template <class PREDICATE_CLASS>
		void HeapRemoveAt(int32 Index, const PREDICATE_CLASS& Predicate, bool bAllowShrinking = true)
		{
			RemoveAtSwap(Index, 1, bAllowShrinking);

			DereferenceWrapper< ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);
			SiftDown(Index, Size(), PredicateWrapper);
			SiftUp(0, Math::Min(Index, Size() - 1), PredicateWrapper);

#if DEBUG_HEAP
			VerifyHeap(PredicateWrapper);
#endif
		}

		/**
		* Removes an element from the heap. Assumes < operator is defined for the template type.
		*
		* @param Index Position at which to remove item.
		* @param bAllowShrinking (Optional) Tells if this call can shrink the array allocation
		*		if suitable after the remove (default = true).
		*/
		void HeapRemoveAt(int32 Index, bool bAllowShrinking = true)
		{
			HeapRemoveAt(Index, Less< ElementType >(), bAllowShrinking);
		}

		/**
		* Performs heap sort on the array.
		*
		* @param Predicate Predicate class instance.
		*/
		template <class PREDICATE_CLASS>
		void HeapSort(const PREDICATE_CLASS& Predicate)
		{
			ReversePredicateWrapper<ElementType, PREDICATE_CLASS> ReversePredicateWrapper(Predicate);
			Heapify(ReversePredicateWrapper);

			ElementType* Heap = Data();
			for (int32 Index = Size() - 1; Index>0; Index--)
			{
				Exchange(Heap[0], Heap[Index]);
				SiftDown(0, Index, ReversePredicateWrapper);
			}

#if DEBUG_HEAP
			DereferenceWrapper<ElementType, PREDICATE_CLASS> PredicateWrapper(Predicate);

			// Verify Heap Property
			VerifyHeap(PredicateWrapper);

			// Also verify Array is properly sorted
			for (int32 Index = 1; Index<Size(); Index++)
			{
				if (PredicateWrapper(Heap[Index], Heap[Index - 1]))
				{
					check(false);
				}
			}
#endif
		}

		/**
		* Performs heap sort on the array. Assumes < operator is defined for the
		* template type.
		*/
		void HeapSort()
		{
			HeapSort(Less<ElementType>());
		}

	private:

		/**
		* Gets the index of the left child of node at Index.
		*
		* @param Index Node for which the left child index is to be returned.
		* @returns Index of the left child.
		*/
		__forceinline int32 HeapGetLeftChildIndex(int32 Index) const
		{
			return Index * 2 + 1;
		}

		/**
		* Checks if node located at Index is a leaf or not.
		*
		* @param Index Node index.
		* @returns true if node is a leaf, false otherwise.
		*/
		__forceinline bool HeapIsLeaf(int32 Index, int32 Count) const
		{
			return HeapGetLeftChildIndex(Index) >= Count;
		}

		/**
		* Gets the parent index for node at Index.
		*
		* @param Index node index.
		* @returns Parent index.
		*/
		__forceinline int32 HeapGetParentIndex(int32 Index) const
		{
			return (Index - 1) / 2;
		}

	private:

		/**
		* Fixes a possible violation of order property between node at Index and a child.
		*
		* @param Index Node index.
		* @param Count Size of the heap (to avoid using Size()).
		* @param Predicate Predicate class instance.
		*/
		template <class PREDICATE_CLASS>
		__forceinline void SiftDown(int32 Index, const int32 Count, const PREDICATE_CLASS& Predicate)
		{
			ElementType* Heap = Data();
			while (!HeapIsLeaf(Index, Count))
			{
				const int32 LeftChildIndex = HeapGetLeftChildIndex(Index);
				const int32 RightChildIndex = LeftChildIndex + 1;

				int32 MinChildIndex = LeftChildIndex;
				if (RightChildIndex < Count)
				{
					MinChildIndex = Predicate(Heap[LeftChildIndex], Heap[RightChildIndex]) ? LeftChildIndex : RightChildIndex;
				}

				if (!Predicate(Heap[MinChildIndex], Heap[Index]))
				{
					break;
				}

				Exchange(Heap[Index], Heap[MinChildIndex]);
				Index = MinChildIndex;
			}
		}

		/**
		* Fixes a possible violation of order property between node at NodeIndex and a parent.
		*
		* @param RootIndex How far to go up?
		* @param NodeIndex Node index.
		* @param Predicate Predicate class instance.
		*
		* @return The new index of the node that was at NodeIndex
		*/
		template <class PREDICATE_CLASS>
		__forceinline int32 SiftUp(int32 RootIndex, int32 NodeIndex, const PREDICATE_CLASS& Predicate)
		{
			ElementType* Heap = Data();
			while (NodeIndex > RootIndex)
			{
				int32 ParentIndex = HeapGetParentIndex(NodeIndex);
				if (!Predicate(Heap[NodeIndex], Heap[ParentIndex]))
				{
					break;
				}

				Exchange(Heap[NodeIndex], Heap[ParentIndex]);
				NodeIndex = ParentIndex;
			}

			return NodeIndex;
		}
	};


	template <typename InElementType, typename Allocator>
	struct IsZeroConstructType<Array<InElementType, Allocator>>
	{
		enum { Value = AllocatorTraits<Allocator>::IsZeroConstruct };
	};


	template <typename InElementType, typename Allocator>
	struct ContainerTraits<Array<InElementType, Allocator> > : public ContainerTraitsBase<Array<InElementType, Allocator> >
	{
		enum { MoveWillEmptyContainer = AllocatorTraits<Allocator>::SupportsMove };
	};


	template <typename T, typename Allocator>
	struct IsContiguousContainer<Array<T, Allocator>>
	{
		enum { Value = true };
	};

	/**
	* Traits class which determines whether or not a type is a Array.
	*/
	template <typename T> struct IsTArray { enum { Value = false }; };

	template <typename InElementType, typename Allocator> struct IsTArray<               Array<InElementType, Allocator>> { enum { Value = true }; };
	template <typename InElementType, typename Allocator> struct IsTArray<const          Array<InElementType, Allocator>> { enum { Value = true }; };
	template <typename InElementType, typename Allocator> struct IsTArray<      volatile Array<InElementType, Allocator>> { enum { Value = true }; };
	template <typename InElementType, typename Allocator> struct IsTArray<const volatile Array<InElementType, Allocator>> { enum { Value = true }; };
}

//
// Array operator news.
//
template <typename T, typename Allocator> void* operator new(size_t Size, EDX::Array<T, Allocator>& Array)
{
	Assert(Size == sizeof(T));
	const EDX::int32 Index = Array.AddUninitialized(1);
	return &Array[Index];
}
template <typename T, typename Allocator> void* operator new(size_t Size, EDX::Array<T, Allocator>& Array, EDX::int32 Index)
{
	Assert(Size == sizeof(T));
	Array.InsertUninitialized(Index, 1);
	return &Array[Index];
}

template <typename T, typename Allocator> void operator delete(void* Mem, EDX::Array<T, Allocator>& Array)
{

}
template <typename T, typename Allocator> void operator delete(void* Mem, EDX::Array<T, Allocator>& Array, EDX::int32 Index)
{

}