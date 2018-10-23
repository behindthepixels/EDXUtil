#pragma once

#include "Array.h"
#include "BitArray.h"
#include "../Core/Template.h"

namespace EDX
{
	// Forward declarations.
	template<typename ElementType, typename Allocator = DefaultSparseArrayAllocator >
	class SparseArray;


	/** The result of a sparse array allocation. */
	struct SparseArrayAllocationInfo
	{
		int32 Index;
		void* Pointer;
	};

	/** Allocated elements are overlapped with free element info in the element list. */
	template<typename ElementType>
	union TSparseArrayElementOrFreeListLink
	{
		/** If the element is allocated, its value is stored here. */
		ElementType ElementData;

		struct
		{
			/** If the element isn't allocated, this is a link to the previous element in the array's free list. */
			int32 PrevFreeIndex;

			/** If the element isn't allocated, this is a link to the next element in the array's free list. */
			int32 NextFreeIndex;
		};
	};

	/**
	* A dynamically sized array where element indices aren't necessarily contiguous.  Memory is allocated for all
	* elements in the array's index range, so it doesn't save memory; but it does allow O(1) element removal that
	* doesn't invalidate the indices of subsequent elements.  It uses Array to store the elements, and a BitArray
	* to store whether each element index is allocated (for fast iteration over allocated elements).
	*
	**/
	template<typename ElementType, typename Allocator /*= DefaultSparseArrayAllocator */>
	class SparseArray
	{
		friend struct ContainerTraits<SparseArray>;

	public:

		/** Destructor. */
		~SparseArray()
		{
			// Destruct the elements in the array.
			Clear();
		}

		/** Marks an index as allocated, and returns information about the allocation. */
		SparseArrayAllocationInfo AllocateIndex(int32 Index)
		{
			Assert(Index >= 0);
			Assert(Index < GetMaxIndex());
			Assert(!AllocationFlags[Index]);

			// Flag the element as allocated.
			AllocationFlags[Index] = true;

			// Set the allocation info.
			SparseArrayAllocationInfo Result;
			Result.Index = Index;
			Result.Pointer = &GetData(Result.Index).ElementData;

			return Result;
		}

		/**
		* Allocates space for an element in the array.  The element is not initialized, and you must use the corresponding placement new operator
		* to construct the element in the allocated memory.
		*/
		SparseArrayAllocationInfo AddUninitialized()
		{
			int32 Index;
			if (NumFreeIndices)
			{
				// Remove and use the first index from the list of free elements.
				Index = FirstFreeIndex;
				FirstFreeIndex = GetData(FirstFreeIndex).NextFreeIndex;
				--NumFreeIndices;
				if (NumFreeIndices)
				{
					GetData(FirstFreeIndex).PrevFreeIndex = -1;
				}
			}
			else
			{
				// Add a new element.
				Index = Data.AddUninitialized(1);
				AllocationFlags.Add(false);
			}

			return AllocateIndex(Index);
		}

		/** Adds an element to the array. */
		int32 Add(typename TypeTraits<ElementType>::ConstInitType Element)
		{
			SparseArrayAllocationInfo Allocation = AddUninitialized();
			new(Allocation) ElementType(Element);
			return Allocation.Index;
		}

		/**
		* Allocates space for an element in the array at a given index.  The element is not initialized, and you must use the corresponding placement new operator
		* to construct the element in the allocated memory.
		*/
		SparseArrayAllocationInfo InsertUninitialized(int32 Index)
		{
			// Enlarge the array to include the given index.
			if (Index >= Data.Size())
			{
				Data.AddUninitialized(Index + 1 - Data.Size());
				while (AllocationFlags.Size() < Data.Size())
				{
					const int32 FreeIndex = AllocationFlags.Size();
					GetData(FreeIndex).PrevFreeIndex = -1;
					GetData(FreeIndex).NextFreeIndex = FirstFreeIndex;
					if (NumFreeIndices)
					{
						GetData(FirstFreeIndex).PrevFreeIndex = FreeIndex;
					}
					FirstFreeIndex = FreeIndex;
					verify(AllocationFlags.Add(false) == FreeIndex);
					++NumFreeIndices;
				};
			}

			// Verify that the specified index is free.
			Assert(!AllocationFlags[Index]);

			// Remove the index from the list of free elements.
			--NumFreeIndices;
			const int32 PrevFreeIndex = GetData(Index).PrevFreeIndex;
			const int32 NextFreeIndex = GetData(Index).NextFreeIndex;
			if (PrevFreeIndex != -1)
			{
				GetData(PrevFreeIndex).NextFreeIndex = NextFreeIndex;
			}
			else
			{
				FirstFreeIndex = NextFreeIndex;
			}
			if (NextFreeIndex != -1)
			{
				GetData(NextFreeIndex).PrevFreeIndex = PrevFreeIndex;
			}

			return AllocateIndex(Index);
		}

		/**
		* Inserts an element to the array.
		*/
		void Insert(int32 Index, typename TypeTraits<ElementType>::ConstInitType Element)
		{
			new(InsertUninitialized(Index)) ElementType(Element);
		}

		/** Removes Count elements from the array, starting from Index. */
		void RemoveAt(int32 Index, int32 Count = 1)
		{
			if (TypeTraits<ElementType>::NeedsDestructor)
			{
				for (int32 It = Index, ItCount = Count; ItCount; ++It, --ItCount)
				{
					((ElementType&)GetData(It).ElementData).~ElementType();
				}
			}

			RemoveAtUninitialized(Index, Count);
		}

		/** Removes Count elements from the array, starting from Index, without destructing them. */
		void RemoveAtUninitialized(int32 Index, int32 Count = 1)
		{
			for (; Count; --Count)
			{
				Assert(AllocationFlags[Index]);

				// Mark the element as free and add it to the free element list.
				if (NumFreeIndices)
				{
					GetData(FirstFreeIndex).PrevFreeIndex = Index;
				}
				auto& IndexData = GetData(Index);
				IndexData.PrevFreeIndex = -1;
				IndexData.NextFreeIndex = NumFreeIndices > 0 ? FirstFreeIndex : INDEX_NONE;
				FirstFreeIndex = Index;
				++NumFreeIndices;
				AllocationFlags[Index] = false;

				++Index;
			}
		}

		/**
		* Removes all elements from the array, potentially leaving space allocated for an expected number of elements about to be added.
		* @param ExpectedNumElements - The expected number of elements about to be added.
		*/
		void Clear(int32 ExpectedNumElements = 0)
		{
			// Destruct the allocated elements.
			if (TypeTraits<ElementType>::NeedsDestructor)
			{
				for (Iterator It(*this); It; ++It)
				{
					ElementType& Element = *It;
					Element.~ElementType();
				}
			}

			// Free the allocated elements.
			Data.Clear(ExpectedNumElements);
			FirstFreeIndex = -1;
			NumFreeIndices = 0;
			AllocationFlags.Clear(ExpectedNumElements);
		}

		/** Empties the array, but keep its allocated memory as slack. */
		void Reset()
		{
			// Destruct the allocated elements.
			if (TypeTraits<ElementType>::NeedsDestructor)
			{
				for (Iterator It(*this); It; ++It)
				{
					ElementType& Element = *It;
					Element.~ElementType();
				}
			}

			// Free the allocated elements.
			Data.Reset();
			FirstFreeIndex = -1;
			NumFreeIndices = 0;
			AllocationFlags.Reset();
		}

		/**
		* Preallocates enough memory to contain the specified number of elements.
		*
		* @param	ExpectedNumElements		the total number of elements that the array will have
		*/
		void Reserve(int32 ExpectedNumElements)
		{
			if (ExpectedNumElements > Data.Size())
			{
				const int32 ElementsToAdd = ExpectedNumElements - Data.Size();

				// allocate memory in the array itself
				int32 ElementIndex = Data.AddUninitialized(ElementsToAdd);

				// now mark the new elements as free
				for (int32 FreeIndex = ElementIndex; FreeIndex < ExpectedNumElements; FreeIndex++)
				{
					if (NumFreeIndices)
					{
						GetData(FirstFreeIndex).PrevFreeIndex = FreeIndex;
					}
					GetData(FreeIndex).PrevFreeIndex = -1;
					GetData(FreeIndex).NextFreeIndex = NumFreeIndices > 0 ? FirstFreeIndex : INDEX_NONE;
					FirstFreeIndex = FreeIndex;
					++NumFreeIndices;
				}
				//@fixme - this will have to do until BitArray has a Reserve method....
				for (int32 i = 0; i < ElementsToAdd; i++)
				{
					AllocationFlags.Add(false);
				}
			}
		}

		/** Shrinks the array's storage to avoid slack. */
		void Shrink()
		{
			// Determine the highest allocated index in the data array.
			int32 MaxAllocatedIndex = INDEX_NONE;
			for (ConstSetBitIterator<typename Allocator::BitArrayAllocator> AllocatedIndexIt(AllocationFlags); AllocatedIndexIt; ++AllocatedIndexIt)
			{
				MaxAllocatedIndex = Math::Max(MaxAllocatedIndex, AllocatedIndexIt.GetIndex());
			}

			const int32 FirstIndexToRemove = MaxAllocatedIndex + 1;
			if (FirstIndexToRemove < Data.Size())
			{
				if (NumFreeIndices > 0)
				{
					// Look for elements in the free list that are in the memory to be freed.
					int32 FreeIndex = FirstFreeIndex;
					while (FreeIndex != INDEX_NONE)
					{
						if (FreeIndex >= FirstIndexToRemove)
						{
							const int32 PrevFreeIndex = GetData(FreeIndex).PrevFreeIndex;
							const int32 NextFreeIndex = GetData(FreeIndex).NextFreeIndex;
							if (NextFreeIndex != -1)
							{
								GetData(NextFreeIndex).PrevFreeIndex = PrevFreeIndex;
							}
							if (PrevFreeIndex != -1)
							{
								GetData(PrevFreeIndex).NextFreeIndex = NextFreeIndex;
							}
							else
							{
								FirstFreeIndex = NextFreeIndex;
							}
							--NumFreeIndices;

							FreeIndex = NextFreeIndex;
						}
						else
						{
							FreeIndex = GetData(FreeIndex).NextFreeIndex;
						}
					}
				}

				// Truncate unallocated elements at the end of the data array.
				Data.RemoveAt(FirstIndexToRemove, Data.Size() - FirstIndexToRemove);
				AllocationFlags.RemoveAt(FirstIndexToRemove, AllocationFlags.Size() - FirstIndexToRemove);
			}

			// Shrink the data array.
			Data.Shrink();
		}

		/** Compacts the allocated elements into a contiguous index range. */
		/** Returns true if any elements were relocated, false otherwise. */
		bool Compact()
		{
			int32 NumFree = NumFreeIndices;
			if (NumFree == 0)
			{
				return false;
			}

			bool bResult = false;

			FElementOrFreeListLink* ElementData = Data.Data();

			int32 EndIndex = Data.Size();
			int32 TargetIndex = EndIndex - NumFree;
			int32 FreeIndex = FirstFreeIndex;
			while (FreeIndex != -1)
			{
				int32 NextFreeIndex = GetData(FreeIndex).NextFreeIndex;
				if (FreeIndex < TargetIndex)
				{
					// We need an element here
					do
					{
						--EndIndex;
					} while (!AllocationFlags[EndIndex]);

					RelocateConstructItems<FElementOrFreeListLink>(ElementData + FreeIndex, ElementData + EndIndex, 1);
					AllocationFlags[FreeIndex] = true;

					bResult = true;
				}

				FreeIndex = NextFreeIndex;
			}

			Data.RemoveAt(TargetIndex, NumFree);
			AllocationFlags.RemoveAt(TargetIndex, NumFree);

			NumFreeIndices = 0;
			FirstFreeIndex = -1;

			return bResult;
		}

		/** Compacts the allocated elements into a contiguous index range. Does not change the iteration order of the elements. */
		/** Returns true if any elements were relocated, false otherwise. */
		bool CompactStable()
		{
			if (NumFreeIndices == 0)
			{
				return false;
			}

			// Copy the existing elements to a new array.
			SparseArray<ElementType, Allocator> CompactedArray;
			CompactedArray.Clear(Size());
			for (ConstIterator It(*this); It; ++It)
			{
				new(CompactedArray.AddUninitialized()) ElementType(*It);
			}

			// Replace this array with the compacted array.
			Exchange(*this, CompactedArray);

			return true;
		}

		/** Sorts the elements using the provided comparison class. */
		template<typename PREDICATE_CLASS>
		void Sort(const PREDICATE_CLASS& Predicate)
		{
			if (Size() > 0)
			{
				// Compact the elements array so all the elements are contiguous.
				Compact();

				// Sort the elements according to the provided comparison class.
				EDX::Sort(&GetData(0), Size(), ElementCompareClass< PREDICATE_CLASS >(Predicate));
			}
		}

		/** Sorts the elements assuming < operator is defined for ElementType. */
		void Sort()
		{
			Sort(Less< ElementType >());
		}

		/**
		* Helper function to return the amount of memory allocated by this container
		* @return number of bytes allocated by this container
		*/
		uint32 GetAllocatedSize(void) const
		{
			return	(Data.Size() + Data.GetSlack()) * sizeof(FElementOrFreeListLink) +
				AllocationFlags.GetAllocatedSize();
		}

		///** Tracks the container's memory use through an archive. */
		//void CountBytes(FArchive& Ar)
		//{
		//	Data.CountBytes(Ar);
		//	AllocationFlags.CountBytes(Ar);
		//}

		///** Serializer. */
		//friend FArchive& operator<<(FArchive& Ar, SparseArray& Array)
		//{
		//	Array.CountBytes(Ar);
		//	if (Ar.IsLoading())
		//	{
		//		// Load array.
		//		int32 NewNumElements = 0;
		//		Ar << NewNumElements;
		//		Array.Clear(NewNumElements);
		//		for (int32 ElementIndex = 0; ElementIndex < NewNumElements; ElementIndex++)
		//		{
		//			Ar << *::new(Array.AddUninitialized())ElementType;
		//		}
		//	}
		//	else
		//	{
		//		// Save array.
		//		int32 NewNumElements = Array.Size();
		//		Ar << NewNumElements;
		//		for (Iterator It(Array); It; ++It)
		//		{
		//			Ar << *It;
		//		}
		//	}
		//	return Ar;
		//}

		/**
		* Equality comparison operator.
		* Checks that both arrays have the same elements and element indices; that means that unallocated elements are signifigant!
		*/
		friend bool operator==(const SparseArray& A, const SparseArray& B)
		{
			if (A.GetMaxIndex() != B.GetMaxIndex())
			{
				return false;
			}

			for (int32 ElementIndex = 0; ElementIndex < A.GetMaxIndex(); ElementIndex++)
			{
				const bool bIsAllocatedA = A.IsAllocated(ElementIndex);
				const bool bIsAllocatedB = B.IsAllocated(ElementIndex);
				if (bIsAllocatedA != bIsAllocatedB)
				{
					return false;
				}
				else if (bIsAllocatedA)
				{
					if (A[ElementIndex] != B[ElementIndex])
					{
						return false;
					}
				}
			}

			return true;
		}

		/**
		* Inequality comparison operator.
		* Checks that both arrays have the same elements and element indices; that means that unallocated elements are signifigant!
		*/
		friend bool operator!=(const SparseArray& A, const SparseArray& B)
		{
			return !(A == B);
		}

		/** Default constructor. */
		SparseArray()
			: FirstFreeIndex(-1)
			, NumFreeIndices(0)
		{}

		/** Move constructor. */
		SparseArray(SparseArray&& InCopy)
		{
			MoveOrCopy(*this, InCopy);
		}

		/** Copy constructor. */
		SparseArray(const SparseArray& InCopy)
			: FirstFreeIndex(-1)
			, NumFreeIndices(0)
		{
			*this = InCopy;
		}

		/** Move assignment operator. */
		SparseArray& operator=(SparseArray&& InCopy)
		{
			if (this != &InCopy)
			{
				MoveOrCopy(*this, InCopy);
			}
			return *this;
		}

		/** Copy assignment operator. */
		SparseArray& operator=(const SparseArray& InCopy)
		{
			if (this != &InCopy)
			{
				// Reallocate the array.
				Clear(InCopy.GetMaxIndex());
				Data.AddUninitialized(InCopy.GetMaxIndex());

				// Copy the other array's element allocation state.
				FirstFreeIndex = InCopy.FirstFreeIndex;
				NumFreeIndices = InCopy.NumFreeIndices;
				AllocationFlags = InCopy.AllocationFlags;

				// Determine whether we need per element construction or bulk copy is fine
				if (TypeTraits<ElementType>::NeedsCopyConstructor)
				{
					FElementOrFreeListLink* SrcData = (FElementOrFreeListLink*)Data.Data();
					const FElementOrFreeListLink* DestData = (FElementOrFreeListLink*)InCopy.Data.Data();

					// Use the inplace new to copy the element to an array element
					for (int32 Index = 0; Index < InCopy.GetMaxIndex(); Index++)
					{
						FElementOrFreeListLink& DestElement = SrcData[Index];
						const FElementOrFreeListLink& SourceElement = DestData[Index];
						if (InCopy.IsAllocated(Index))
						{
							::new((uint8*)&DestElement.ElementData) ElementType(*(ElementType*)&SourceElement.ElementData);
						}
						DestElement.PrevFreeIndex = SourceElement.PrevFreeIndex;
						DestElement.NextFreeIndex = SourceElement.NextFreeIndex;
					}
				}
				else
				{
					// Use the much faster path for types that allow it
					Memory::Memcpy(Data.Data(), InCopy.Data.Data(), sizeof(FElementOrFreeListLink) * InCopy.GetMaxIndex());
				}
			}
			return *this;
		}

	private:
		template <typename SparseArrayType>
		__forceinline static typename EnableIf<ContainerTraits<SparseArrayType>::MoveWillEmptyContainer>::Type MoveOrCopy(SparseArrayType& ToArray, SparseArrayType& FromArray)
		{
			ToArray.Data = (DataType&&)FromArray.Data;
			ToArray.AllocationFlags = (AllocationBitArrayType&&)FromArray.AllocationFlags;

			ToArray.FirstFreeIndex = FromArray.FirstFreeIndex;
			ToArray.NumFreeIndices = FromArray.NumFreeIndices;
			FromArray.FirstFreeIndex = -1;
			FromArray.NumFreeIndices = 0;
		}

		template <typename SparseArrayType>
		__forceinline static typename EnableIf<!ContainerTraits<SparseArrayType>::MoveWillEmptyContainer>::Type MoveOrCopy(SparseArrayType& ToArray, SparseArrayType& FromArray)
		{
			ToArray = FromArray;
		}

	public:
		// Accessors.
		ElementType& operator[](int32 Index)
		{
			Assert(Index >= 0 && Index < Data.Size() && Index < AllocationFlags.Size());
			//check(AllocationFlags[Index]); // Disabled to improve loading times -BZ
			return *(ElementType*)&GetData(Index).ElementData;
		}
		const ElementType& operator[](int32 Index) const
		{
			Assert(Index >= 0 && Index < Data.Size() && Index < AllocationFlags.Size());
			//check(AllocationFlags[Index]); // Disabled to improve loading times -BZ
			return *(ElementType*)&GetData(Index).ElementData;
		}

		bool IsAllocated(int32 Index) const { return AllocationFlags[Index]; }
		int32 GetMaxIndex() const { return Data.Size(); }
		int32 Size() const { return Data.Size() - NumFreeIndices; }

		/**
		* Checks that the specified address is not part of an element within the container.  Used for implementations
		* to check that reference arguments aren't going to be invalidated by possible reallocation.
		*
		* @param Addr The address to check.
		*/
		__forceinline void CheckAddress(const ElementType* Addr) const
		{
			Data.CheckAddress(Addr);
		}

	private:

		/** The base class of sparse array iterators. */
		template<bool bConst>
		class BaseIterator
		{
		public:
			typedef ConstSetBitIterator<typename Allocator::BitArrayAllocator> BitArrayItType;

		private:
			typedef typename ChooseClass<bConst, const SparseArray, SparseArray>::Result ArrayType;
			typedef typename ChooseClass<bConst, const ElementType, ElementType>::Result ItElementType;

		public:
			explicit BaseIterator(ArrayType& InArray, const BitArrayItType& InBitArrayIt)
				: Array(InArray)
				, BitArrayIt(InBitArrayIt)
			{
			}

			__forceinline BaseIterator& operator++()
			{
				// Iterate to the next set allocation flag.
				++BitArrayIt;
				return *this;
			}

			__forceinline int32 GetIndex() const { return BitArrayIt.GetIndex(); }

			__forceinline friend bool operator==(const BaseIterator& Lhs, const BaseIterator& Rhs) { return Lhs.BitArrayIt == Rhs.BitArrayIt && &Lhs.Array == &Rhs.Array; }
			__forceinline friend bool operator!=(const BaseIterator& Lhs, const BaseIterator& Rhs) { return Lhs.BitArrayIt != Rhs.BitArrayIt || &Lhs.Array != &Rhs.Array; }

			/** conversion to "bool" returning true if the iterator is valid. */
			__forceinline explicit operator bool() const
			{
				return !!BitArrayIt;
			}

			/** inverse of the "bool" operator */
			__forceinline bool operator !() const
			{
				return !(bool)*this;
			}

			__forceinline ItElementType& operator*() const { return Array[GetIndex()]; }
			__forceinline ItElementType* operator->() const { return &Array[GetIndex()]; }
			__forceinline const RelativeBitReference& GetRelativeBitReference() const { return BitArrayIt; }

		protected:
			ArrayType&     Array;
			BitArrayItType BitArrayIt;
		};

	public:

		/** Iterates over all allocated elements in a sparse array. */
		class Iterator : public BaseIterator<false>
		{
		public:
			Iterator(SparseArray& InArray)
				: BaseIterator<false>(InArray, ConstSetBitIterator<typename Allocator::BitArrayAllocator>(InArray.AllocationFlags))
			{
			}

			Iterator(SparseArray& InArray, const typename BaseIterator<false>::BitArrayItType& InBitArrayIt)
				: BaseIterator<false>(InArray, InBitArrayIt)
			{
			}

			/** Safely removes the current element from the array. */
			void RemoveCurrent()
			{
				this->Array.RemoveAt(this->GetIndex());
			}
		};

		/** Iterates over all allocated elements in a const sparse array. */
		class ConstIterator : public BaseIterator<true>
		{
		public:
			ConstIterator(const SparseArray& InArray)
				: BaseIterator<true>(InArray, ConstSetBitIterator<typename Allocator::BitArrayAllocator>(InArray.AllocationFlags))
			{
			}

			ConstIterator(const SparseArray& InArray, const typename BaseIterator<true>::BitArrayItType& InBitArrayIt)
				: BaseIterator<true>(InArray, InBitArrayIt)
			{
			}
		};

		/** Creates an iterator for the contents of this array */
		Iterator CreateIterator()
		{
			return Iterator(*this);
		}

		/** Creates a const iterator for the contents of this array */
		ConstIterator CreateConstIterator() const
		{
			return ConstIterator(*this);
		}

	private:
		/**
		* DO NOT USE DIRECTLY
		* STL-like iterators to enable range-based for loop support.
		*/
		__forceinline friend Iterator      begin(SparseArray& Array) { return Iterator(Array, ConstSetBitIterator<typename Allocator::BitArrayAllocator>(Array.AllocationFlags)); }
		__forceinline friend ConstIterator begin(const SparseArray& Array) { return ConstIterator(Array, ConstSetBitIterator<typename Allocator::BitArrayAllocator>(Array.AllocationFlags)); }
		__forceinline friend Iterator      end(SparseArray& Array) { return Iterator(Array, ConstSetBitIterator<typename Allocator::BitArrayAllocator>(Array.AllocationFlags, Array.AllocationFlags.Size())); }
		__forceinline friend ConstIterator end(const SparseArray& Array) { return ConstIterator(Array, ConstSetBitIterator<typename Allocator::BitArrayAllocator>(Array.AllocationFlags, Array.AllocationFlags.Size())); }

	public:
		/** An iterator which only iterates over the elements of the array which correspond to set bits in a separate bit array. */
		template<typename SubsetAllocator = DefaultBitArrayAllocator>
		class ConstSubsetIterator
		{
		public:
			ConstSubsetIterator(const SparseArray& InArray, const BitArray<SubsetAllocator>& InBitArray) :
				Array(InArray),
				BitArrayIt(InArray.AllocationFlags, InBitArray)
			{}
			__forceinline ConstSubsetIterator& operator++()
			{
				// Iterate to the next element which is both allocated and has its bit set in the other bit array.
				++BitArrayIt;
				return *this;
			}
			__forceinline int32 GetIndex() const { return BitArrayIt.GetIndex(); }

			/** conversion to "bool" returning true if the iterator is valid. */
			__forceinline explicit operator bool() const
			{
				return !!BitArrayIt;
			}
			/** inverse of the "bool" operator */
			__forceinline bool operator !() const
			{
				return !(bool)*this;
			}

			__forceinline const ElementType& operator*() const { return Array(GetIndex()); }
			__forceinline const ElementType* operator->() const { return &Array(GetIndex()); }
			__forceinline const RelativeBitReference& GetRelativeBitReference() const { return BitArrayIt; }
		private:
			const SparseArray& Array;
			ConstDualSetBitIterator<typename Allocator::BitArrayAllocator, SubsetAllocator> BitArrayIt;
		};

		/** Concatenation operators */
		SparseArray& operator+=(const SparseArray& OtherArray)
		{
			this->Reserve(this->Size() + OtherArray.Size());
			for (typename SparseArray::ConstIterator It(OtherArray); It; ++It)
			{
				this->Add(*It);
			}
			return *this;
		}
		SparseArray& operator+=(const Array<ElementType>& OtherArray)
		{
			this->Reserve(this->Size() + OtherArray.Size());
			for (int32 Idx = 0; Idx < OtherArray.Size(); Idx++)
			{
				this->Add(OtherArray[Idx]);
			}
			return *this;
		}

	private:

		/**
		* The element type stored is only indirectly related to the element type requested, to avoid instantiating Array redundantly for
		* compatible types.
		*/
		typedef TSparseArrayElementOrFreeListLink<
			AlignedBytes<sizeof(ElementType), ALIGNOF(ElementType)>
		> FElementOrFreeListLink;

		/** Extracts the element value from the array's element structure and passes it to the user provided comparison class. */
		template <typename PREDICATE_CLASS>
		class ElementCompareClass
		{
			const PREDICATE_CLASS& Predicate;

		public:
			ElementCompareClass(const PREDICATE_CLASS& InPredicate)
				: Predicate(InPredicate)
			{}

			bool operator()(const FElementOrFreeListLink& A, const FElementOrFreeListLink& B) const
			{
				return Predicate(*(ElementType*)&A.ElementData, *(ElementType*)&B.ElementData);
			}
		};

		/** Accessor for the element or free list data. */
		FElementOrFreeListLink& GetData(int32 Index)
		{
			return ((FElementOrFreeListLink*)Data.Data())[Index];
		}

		/** Accessor for the element or free list data. */
		const FElementOrFreeListLink& GetData(int32 Index) const
		{
			return ((FElementOrFreeListLink*)Data.Data())[Index];
		}

		typedef Array<FElementOrFreeListLink, typename Allocator::ElementAllocator> DataType;
		DataType Data;

		typedef BitArray<typename Allocator::BitArrayAllocator> AllocationBitArrayType;
		AllocationBitArrayType AllocationFlags;

		/** The index of an unallocated element in the array that currently contains the head of the linked list of free elements. */
		int32 FirstFreeIndex;

		/** The number of elements in the free list. */
		int32 NumFreeIndices;
	};

	template<typename ElementType, typename Allocator>
	struct ContainerTraits<SparseArray<ElementType, Allocator> > : public ContainerTraitsBase<SparseArray<ElementType, Allocator> >
	{
		enum {
			MoveWillEmptyContainer =
			ContainerTraits<typename SparseArray<ElementType, Allocator>::DataType>::MoveWillEmptyContainer &&
			ContainerTraits<typename SparseArray<ElementType, Allocator>::AllocationBitArrayType>::MoveWillEmptyContainer
		};
	};
}

/**
* Placement new/delete operator which constructs an element in a sparse array allocation.
*/
inline void* operator new(size_t Size, const EDX::SparseArrayAllocationInfo& Allocation)
{
	__assume(Allocation.Pointer);
	return Allocation.Pointer;
}

inline void operator delete(void* Mem, const EDX::SparseArrayAllocationInfo& Allocation)
{
	delete Allocation.Pointer;
}