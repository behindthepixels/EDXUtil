#pragma once

#include "../Windows/Atomics.h"

namespace EDX
{
	// Functions for manipulating bit sets.
	struct BitSet
	{
		/** Clears the next set bit in the mask and returns its index. */
		static __forceinline uint32 GetAndClearNextBit(uint32& Mask)
		{
			const uint32 LowestBitMask = (Mask) & (-(int32)Mask);
			const uint32 BitIndex = Math::FloorLog2(LowestBitMask);
			Mask ^= LowestBitMask;
			return BitIndex;
		}
	};


	// Forward declaration.
	template<typename Allocator = DefaultBitArrayAllocator>
	class BitArray;

	template<typename Allocator = DefaultBitArrayAllocator>
	class ConstSetBitIterator;

	template<typename Allocator = DefaultBitArrayAllocator, typename OtherAllocator = DefaultBitArrayAllocator>
	class ConstDualSetBitIterator;

	/** Used to read/write a bit in the array as a bool. */
	class BitReference
	{
	public:

		__forceinline BitReference(uint32& InData, uint32 InMask)
			: Data(InData)
			, Mask(InMask)
		{}

		__forceinline operator bool() const
		{
			return (Data & Mask) != 0;
		}
		__forceinline void operator=(const bool NewValue)
		{
			if (NewValue)
			{
				Data |= Mask;
			}
			else
			{
				Data &= ~Mask;
			}
		}
		__forceinline void AtomicSet(const bool NewValue)
		{
			if (NewValue)
			{
				if (!(Data & Mask))
				{
					while (1)
					{
						uint32 Current = Data;
						uint32 Desired = Current | Mask;
						if (Current == Desired || WindowsAtomics::InterlockedCompareExchange((volatile int32*)&Data, (int32)Desired, (int32)Current) == (int32)Current)
						{
							return;
						}
					}
				}
			}
			else
			{
				if (Data & Mask)
				{
					while (1)
					{
						uint32 Current = Data;
						uint32 Desired = Current & ~Mask;
						if (Current == Desired || WindowsAtomics::InterlockedCompareExchange((volatile int32*)&Data, (int32)Desired, (int32)Current) == (int32)Current)
						{
							return;
						}
					}
				}
			}
		}
		__forceinline BitReference& operator=(const BitReference& Copy)
		{
			// As this is emulating a reference, assignment should not rebind,
			// it should write to the referenced bit.
			*this = (bool)Copy;
			return *this;
		}

	private:
		uint32& Data;
		uint32 Mask;
	};


	/** Used to read a bit in the array as a bool. */
	class ConstBitReference
	{
	public:

		__forceinline ConstBitReference(const uint32& InData, uint32 InMask)
			: Data(InData)
			, Mask(InMask)
		{}

		__forceinline operator bool() const
		{
			return (Data & Mask) != 0;
		}

	private:
		const uint32& Data;
		uint32 Mask;
	};


	/** Used to reference a bit in an unspecified bit array. */
	class RelativeBitReference
	{
	public:
		__forceinline explicit RelativeBitReference(int32 BitIndex)
			: DWORDIndex(BitIndex >> NumBitsPerDWORDLogTwo)
			, Mask(1 << (BitIndex & (NumBitsPerDWORD - 1)))
		{
		}

		int32  DWORDIndex;
		uint32 Mask;
	};


	/**
	* A dynamically sized bit array.
	* An array of Booleans.  They stored in one bit/Boolean.  There are iterators that efficiently iterate over only set bits.
	*/
	template<typename Allocator/* = DefaultBitArrayAllocator*/>
	class BitArray
	{
	public:

		template<typename>
		friend class ConstSetBitIterator;

		template<typename, typename>
		friend class ConstDualSetBitIterator;

		/**
		* Minimal initialization constructor.
		* @param Value - The value to initial the bits to.
		* @param InNumBits - The initial number of bits in the array.
		*/
		explicit BitArray(const bool Value = false, const int32 InNumBits = 0)
			: NumBits(0)
			, MaxBits(0)
		{
			Init(Value, InNumBits);
		}

		/**
		* Move constructor.
		*/
		__forceinline BitArray(BitArray&& Other)
		{
			MoveOrCopy(*this, Other);
		}

		/**
		* Copy constructor.
		*/
		__forceinline BitArray(const BitArray& Copy)
			: NumBits(0)
			, MaxBits(0)
		{
			*this = Copy;
		}

		/**
		* Move assignment.
		*/
		__forceinline BitArray& operator=(BitArray&& Other)
		{
			if (this != &Other)
			{
				MoveOrCopy(*this, Other);
			}

			return *this;
		}

		/**
		* Assignment operator.
		*/
		__forceinline BitArray& operator=(const BitArray& Copy)
		{
			// check for self assignment since we don't use swap() mechanic
			if (this == &Copy)
			{
				return *this;
			}

			Clear(Copy.Size());
			NumBits = MaxBits = Copy.NumBits;
			if (NumBits)
			{
				const int32 NumDWORDs = Math::DivideAndRoundUp(MaxBits, NumBitsPerDWORD);
				AlignedRealloc(0);
				Memory::Memcpy(Data(), Copy.Data(), NumDWORDs * sizeof(uint32));
			}
			return *this;
		}

	private:
		template <typename BitArrayType>
		static __forceinline typename EnableIf<ContainerTraits<BitArrayType>::MoveWillEmptyContainer>::Type MoveOrCopy(BitArrayType& ToArray, BitArrayType& FromArray)
		{
			ToArray.AllocatorInstance.MoveToEmpty(FromArray.AllocatorInstance);

			ToArray.NumBits = FromArray.NumBits;
			ToArray.MaxBits = FromArray.MaxBits;
			FromArray.NumBits = 0;
			FromArray.MaxBits = 0;
		}

		template <typename BitArrayType>
		static __forceinline typename EnableIf<!ContainerTraits<BitArrayType>::MoveWillEmptyContainer>::Type MoveOrCopy(BitArrayType& ToArray, BitArrayType& FromArray)
		{
			ToArray = FromArray;
		}

	public:
		///**
		//* Serializer
		//*/
		//friend FArchive& operator<<(FArchive& Ar, BitArray& BitArray)
		//{
		//	// serialize number of bits
		//	Ar << BitArray.NumBits;

		//	if (Ar.IsLoading())
		//	{
		//		// no need for slop when reading
		//		BitArray.MaxBits = BitArray.NumBits;

		//		// allocate room for new bits
		//		BitArray.AlignedRealloc(0);
		//	}

		//	// calc the number of dwords for all the bits
		//	const int32 NumDWORDs = Math::DivideAndRoundUp(BitArray.NumBits, NumBitsPerDWORD);

		//	// serialize the data as one big chunk
		//	Ar.Serialize(BitArray.Data(), NumDWORDs * sizeof(uint32));

		//	return Ar;
		//}

		/**
		* Adds a bit to the array with the given value.
		* @return The index of the added bit.
		*/
		int32 Add(const bool Value)
		{
			const int32 Index = NumBits;
			const bool bReallocate = (NumBits + 1) > MaxBits;

			NumBits++;

			if (bReallocate)
			{
				// Allocate memory for the new bits.
				const uint32 MaxDWORDs = AllocatorInstance.CalculateSlackGrow(
					Math::DivideAndRoundUp(NumBits, NumBitsPerDWORD),
					Math::DivideAndRoundUp(MaxBits, NumBitsPerDWORD),
					sizeof(uint32)
				);
				MaxBits = MaxDWORDs * NumBitsPerDWORD;
				AlignedRealloc(NumBits - 1);
			}

			(*this)[Index] = Value;

			return Index;
		}

		/**
		* Removes all bits from the array, potentially leaving space allocated for an expected number of bits about to be added.
		* @param ExpectedNumBits - The expected number of bits about to be added.
		*/
		void Clear(int32 ExpectedNumBits = 0)
		{
			NumBits = 0;

			ExpectedNumBits = Math::DivideAndRoundUp(ExpectedNumBits, NumBitsPerDWORD) * NumBitsPerDWORD;
			// If the expected number of bits doesn't match the allocated number of bits, reallocate.
			if (MaxBits != ExpectedNumBits)
			{
				MaxBits = ExpectedNumBits;
				AlignedRealloc(0);
			}
		}

		/**
		* Removes all bits from the array retaining any space already allocated.
		*/
		void Reset()
		{
			// We need this because iterators often use whole DWORDs when masking, which includes off-the-end elements
			Memory::Memset(Data(), 0, Math::DivideAndRoundUp(NumBits, NumBitsPerDWORD) * sizeof(uint32));

			NumBits = 0;
		}

		/**
		* Resets the array's contents.
		* @param Value - The value to initial the bits to.
		* @param NumBits - The number of bits in the array.
		*/
		void Init(bool Value, int32 InNumBits)
		{
			Clear(InNumBits);
			if (InNumBits)
			{
				NumBits = InNumBits;
				Memory::Memset(Data(), Value ? 0xff : 0, Math::DivideAndRoundUp(NumBits, NumBitsPerDWORD) * sizeof(uint32));
			}
		}

		/**
		* Sets or unsets a range of bits within the array.
		* @param  Index  The index of the first bit to set.
		* @param  Num    The number of bits to set.
		* @param  Value  The value to set the bits to.
		*/
		void SetRange(int32 Index, int32 Num, bool Value)
		{
			Assert(Index >= 0 && Num >= 0 && Index + Num <= NumBits);

			if (Num == 0)
			{
				return;
			}

			// Work out which uint32 index to set from, and how many
			uint32 StartIndex = Index / 32;
			uint32 Count = (Index + Num + 31) / 32 - StartIndex;

			// Work out masks for the start/end of the sequence
			uint32 StartMask = 0xFFFFFFFFu << (Index % 32);
			uint32 EndMask = 0xFFFFFFFFu >> (32 - (Index + Num) % 32) % 32;

			uint32* Data = Data() + StartIndex;
			if (Value)
			{
				if (Count == 1)
				{
					*Data |= StartMask & EndMask;
				}
				else
				{
					*Data++ |= StartMask;
					Count -= 2;
					while (Count != 0)
					{
						*Data++ = ~0;
						--Count;
					}
					*Data |= EndMask;
				}
			}
			else
			{
				if (Count == 1)
				{
					*Data &= ~(StartMask & EndMask);
				}
				else
				{
					*Data++ &= ~StartMask;
					Count -= 2;
					while (Count != 0)
					{
						*Data++ = 0;
						--Count;
					}
					*Data &= ~EndMask;
				}
			}
		}

		/**
		* Removes bits from the array.
		* @param BaseIndex - The index of the first bit to remove.
		* @param NumBitsToRemove - The number of consecutive bits to remove.
		*/
		void RemoveAt(int32 BaseIndex, int32 NumBitsToRemove = 1)
		{
			Assert(BaseIndex >= 0 && BaseIndex + NumBitsToRemove <= NumBits);

			// Until otherwise necessary, this is an obviously correct implementation rather than an efficient implementation.
			Iterator WriteIt(*this);
			for (ConstIterator ReadIt(*this); ReadIt; ++ReadIt)
			{
				// If this bit isn't being removed, write it back to the array at its potentially new index.
				if (ReadIt.GetIndex() < BaseIndex || ReadIt.GetIndex() >= BaseIndex + NumBitsToRemove)
				{
					if (WriteIt.GetIndex() != ReadIt.GetIndex())
					{
						WriteIt.GetValue() = (bool)ReadIt.GetValue();
					}
					++WriteIt;
				}
			}
			NumBits -= NumBitsToRemove;
		}

		/* Removes bits from the array by swapping them with bits at the end of the array.
		* This is mainly implemented so that other code using Array::RemoveSwap will have
		* matching indices.
		* @param BaseIndex - The index of the first bit to remove.
		* @param NumBitsToRemove - The number of consecutive bits to remove.
		*/
		void RemoveAtSwap(int32 BaseIndex, int32 NumBitsToRemove = 1)
		{
			Assert(BaseIndex >= 0 && BaseIndex + NumBitsToRemove <= NumBits);
			if (BaseIndex < NumBits - NumBitsToRemove)
			{
				// Copy bits from the end to the region we are removing
				for (int32 Index = 0; Index<NumBitsToRemove; Index++)
				{
					(*this)[BaseIndex + Index] = (bool)(*this)[NumBits - NumBitsToRemove + Index];
				}
			}
			// Remove the bits from the end of the array.
			RemoveAt(NumBits - NumBitsToRemove, NumBitsToRemove);
		}


		/**
		* Helper function to return the amount of memory allocated by this container
		* @return number of bytes allocated by this container
		*/
		uint32 GetAllocatedSize(void) const
		{
			return Math::DivideAndRoundUp(MaxBits, NumBitsPerDWORD) * sizeof(uint32);
		}

		/** Tracks the container's memory use through an archive. */
		//void CountBytes(FArchive& Ar)
		//{
		//	Ar.CountBytes(
		//		Math::DivideAndRoundUp(NumBits, NumBitsPerDWORD) * sizeof(uint32),
		//		Math::DivideAndRoundUp(MaxBits, NumBitsPerDWORD) * sizeof(uint32)
		//	);
		//}

		/**
		* Finds the first zero bit in the array, sets it to true, and returns the bit index.
		* If there is none, INDEX_NONE is returned.
		*/
		int32 FindAndSetFirstZeroBit()
		{
			// Iterate over the array until we see a word with a zero bit.
			uint32* RESTRICT DwordArray = Data();
			const int32 DwordCount = Math::DivideAndRoundUp(Size(), NumBitsPerDWORD);
			int32 DwordIndex = 0;
			while (DwordIndex < DwordCount && DwordArray[DwordIndex] == 0xffffffff)
			{
				DwordIndex++;
			}

			if (DwordIndex < DwordCount)
			{
				// Flip the bits, then we only need to find the first one bit -- easy.
				const uint32 Bits = ~(DwordArray[DwordIndex]);
				const uint32 LowestBitMask = (Bits) & (-(int32)Bits);
				const int32 LowestBitIndex = Math::FloorLog2(LowestBitMask) + (DwordIndex << NumBitsPerDWORDLogTwo);
				if (LowestBitIndex < NumBits)
				{
					DwordArray[DwordIndex] |= LowestBitMask;
					return LowestBitIndex;
				}
			}

			return INDEX_NONE;
		}

		// Accessors.
		__forceinline bool IsValidIndex(int32 InIndex) const
		{
			return InIndex >= 0 && InIndex < NumBits;
		}

		__forceinline int32 Size() const { return NumBits; }
		__forceinline BitReference operator[](int32 Index)
		{
			Assert(Index >= 0 && Index<NumBits);
			return BitReference(
				Data()[Index / NumBitsPerDWORD],
				1 << (Index & (NumBitsPerDWORD - 1))
			);
		}
		__forceinline const ConstBitReference operator[](int32 Index) const
		{
			Assert(Index >= 0 && Index<NumBits);
			return ConstBitReference(
				Data()[Index / NumBitsPerDWORD],
				1 << (Index & (NumBitsPerDWORD - 1))
			);
		}
		__forceinline BitReference AccessCorrespondingBit(const RelativeBitReference& RelativeReference)
		{
			Assert(RelativeReference.Mask);
			Assert(RelativeReference.DWORDIndex >= 0);
			Assert(((uint32)RelativeReference.DWORDIndex + 1) * NumBitsPerDWORD - 1 - Math::CountLeadingZeros(RelativeReference.Mask) < (uint32)NumBits);
			return BitReference(
				Data()[RelativeReference.DWORDIndex],
				RelativeReference.Mask
			);
		}
		__forceinline const ConstBitReference AccessCorrespondingBit(const RelativeBitReference& RelativeReference) const
		{
			Assert(RelativeReference.Mask);
			Assert(RelativeReference.DWORDIndex >= 0);
			Assert(((uint32)RelativeReference.DWORDIndex + 1) * NumBitsPerDWORD - 1 - Math::CountLeadingZeros(RelativeReference.Mask) < (uint32)NumBits);
			return ConstBitReference(
				Data()[RelativeReference.DWORDIndex],
				RelativeReference.Mask
			);
		}

		/** BitArray iterator. */
		class Iterator : public RelativeBitReference
		{
		public:
			__forceinline Iterator(BitArray<Allocator>& InArray, int32 StartIndex = 0)
				: RelativeBitReference(StartIndex)
				, Array(InArray)
				, Index(StartIndex)
			{
			}
			__forceinline Iterator& operator++()
			{
				++Index;
				this->Mask <<= 1;
				if (!this->Mask)
				{
					// Advance to the next uint32.
					this->Mask = 1;
					++this->DWORDIndex;
				}
				return *this;
			}
			/** conversion to "bool" returning true if the iterator is valid. */
			__forceinline explicit operator bool() const
			{
				return Index < Array.Size();
			}
			/** inverse of the "bool" operator */
			__forceinline bool operator !() const
			{
				return !(bool)*this;
			}

			__forceinline BitReference GetValue() const { return BitReference(Array.Data()[this->DWORDIndex], this->Mask); }
			__forceinline int32 GetIndex() const { return Index; }
		private:
			BitArray<Allocator>& Array;
			int32 Index;
		};

		/** Const BitArray iterator. */
		class ConstIterator : public RelativeBitReference
		{
		public:
			__forceinline ConstIterator(const BitArray<Allocator>& InArray, int32 StartIndex = 0)
				: RelativeBitReference(StartIndex)
				, Array(InArray)
				, Index(StartIndex)
			{
			}
			__forceinline ConstIterator& operator++()
			{
				++Index;
				this->Mask <<= 1;
				if (!this->Mask)
				{
					// Advance to the next uint32.
					this->Mask = 1;
					++this->DWORDIndex;
				}
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			__forceinline explicit operator bool() const
			{
				return Index < Array.Size();
			}
			/** inverse of the "bool" operator */
			__forceinline bool operator !() const
			{
				return !(bool)*this;
			}

			__forceinline ConstBitReference GetValue() const { return ConstBitReference(Array.Data()[this->DWORDIndex], this->Mask); }
			__forceinline int32 GetIndex() const { return Index; }
		private:
			const BitArray<Allocator>& Array;
			int32 Index;
		};

		/** Const reverse iterator. */
		class ConstReverseIterator : public RelativeBitReference
		{
		public:
			__forceinline ConstReverseIterator(const BitArray<Allocator>& InArray)
				: RelativeBitReference(InArray.Size() - 1)
				, Array(InArray)
				, Index(InArray.Size() - 1)
			{
			}
			__forceinline ConstReverseIterator& operator++()
			{
				--Index;
				this->Mask >>= 1;
				if (!this->Mask)
				{
					// Advance to the next uint32.
					this->Mask = (1 << (NumBitsPerDWORD - 1));
					--this->DWORDIndex;
				}
				return *this;
			}

			/** conversion to "bool" returning true if the iterator is valid. */
			__forceinline explicit operator bool() const
			{
				return Index >= 0;
			}
			/** inverse of the "bool" operator */
			__forceinline bool operator !() const
			{
				return !(bool)*this;
			}

			__forceinline ConstBitReference GetValue() const { return ConstBitReference(Array.Data()[this->DWORDIndex], this->Mask); }
			__forceinline int32 GetIndex() const { return Index; }
		private:
			const BitArray<Allocator>& Array;
			int32 Index;
		};

		__forceinline const uint32* Data() const
		{
			return (uint32*)AllocatorInstance.GetAllocation();
		}

		__forceinline uint32* Data()
		{
			return (uint32*)AllocatorInstance.GetAllocation();
		}

	private:
		typedef typename Allocator::template ForElementType<uint32> AllocatorType;

		AllocatorType AllocatorInstance;
		int32         NumBits;
		int32         MaxBits;

		void AlignedRealloc(int32 PreviousNumBits)
		{
			const int32 PreviousNumDWORDs = Math::DivideAndRoundUp(PreviousNumBits, NumBitsPerDWORD);
			const int32 MaxDWORDs = Math::DivideAndRoundUp(MaxBits, NumBitsPerDWORD);

			AllocatorInstance.ResizeAllocation(PreviousNumDWORDs, MaxDWORDs, sizeof(uint32));

			if (MaxDWORDs)
			{
				// Reset the newly allocated slack DWORDs.
				Memory::Memzero((uint32*)AllocatorInstance.GetAllocation() + PreviousNumDWORDs, (MaxDWORDs - PreviousNumDWORDs) * sizeof(uint32));
			}
		}
	};


	template<typename Allocator>
	struct ContainerTraits<BitArray<Allocator> > : public ContainerTraitsBase<BitArray<Allocator> >
	{
		enum { MoveWillEmptyContainer = AllocatorTraits<Allocator>::SupportsMove };
	};


	/** An iterator which only iterates over set bits. */
	template<typename Allocator>
	class ConstSetBitIterator : public RelativeBitReference
	{
	public:

		/** Constructor. */
		ConstSetBitIterator(const BitArray<Allocator>& InArray, int32 StartIndex = 0)
			: RelativeBitReference(StartIndex)
			, Array(InArray)
			, UnvisitedBitMask((~0) << (StartIndex & (NumBitsPerDWORD - 1)))
			, CurrentBitIndex(StartIndex)
			, BaseBitIndex(StartIndex & ~(NumBitsPerDWORD - 1))
		{
			Assert(StartIndex >= 0 && StartIndex <= Array.Size());
			if (StartIndex != Array.Size())
			{
				FindFirstSetBit();
			}
		}

		/** Forwards iteration operator. */
		__forceinline ConstSetBitIterator& operator++()
		{
			// Mark the current bit as visited.
			UnvisitedBitMask &= ~this->Mask;

			// Find the first set bit that hasn't been visited yet.
			FindFirstSetBit();

			return *this;
		}

		__forceinline friend bool operator==(const ConstSetBitIterator& Lhs, const ConstSetBitIterator& Rhs)
		{
			// We only need to compare the bit index and the array... all the rest of the state is unobservable.
			return Lhs.CurrentBitIndex == Rhs.CurrentBitIndex && &Lhs.Array == &Rhs.Array;
		}

		__forceinline friend bool operator!=(const ConstSetBitIterator& Lhs, const ConstSetBitIterator& Rhs)
		{
			return !(Lhs == Rhs);
		}

		/** conversion to "bool" returning true if the iterator is valid. */
		__forceinline explicit operator bool() const
		{
			return CurrentBitIndex < Array.Size();
		}
		/** inverse of the "bool" operator */
		__forceinline bool operator !() const
		{
			return !(bool)*this;
		}

		/** Index accessor. */
		__forceinline int32 GetIndex() const
		{
			return CurrentBitIndex;
		}

	private:

		const BitArray<Allocator>& Array;

		uint32 UnvisitedBitMask;
		int32 CurrentBitIndex;
		int32 BaseBitIndex;

		/** Find the first set bit starting with the current bit, inclusive. */
		void FindFirstSetBit()
		{
			const uint32* ArrayData = Array.Data();
			const int32   mSize = Array.Size();
			const int32   LastDWORDIndex = (mSize - 1) / NumBitsPerDWORD;

			// Advance to the next non-zero uint32.
			uint32 RemainingBitMask = ArrayData[this->DWORDIndex] & UnvisitedBitMask;
			while (!RemainingBitMask)
			{
				++this->DWORDIndex;
				BaseBitIndex += NumBitsPerDWORD;
				if (this->DWORDIndex > LastDWORDIndex)
				{
					// We've advanced past the end of the array.
					CurrentBitIndex = mSize;
					return;
				}

				RemainingBitMask = ArrayData[this->DWORDIndex];
				UnvisitedBitMask = ~0;
			}

			// This operation has the effect of unsetting the lowest set bit of BitMask
			const uint32 NewRemainingBitMask = RemainingBitMask & (RemainingBitMask - 1);

			// This operation XORs the above mask with the original mask, which has the effect
			// of returning only the bits which differ; specifically, the lowest bit
			this->Mask = NewRemainingBitMask ^ RemainingBitMask;

			// If the Nth bit was the lowest set bit of BitMask, then this gives us N
			CurrentBitIndex = BaseBitIndex + NumBitsPerDWORD - 1 - Math::CountLeadingZeros(this->Mask);

			// If we've accidentally iterated off the end of an array but still within the same DWORD
			// then set the index to the last index of the array
			if (CurrentBitIndex > mSize)
			{
				CurrentBitIndex = mSize;
			}
		}
	};


	/** An iterator which only iterates over the bits which are set in both of two bit-arrays. */
	template<typename Allocator, typename OtherAllocator>
	class ConstDualSetBitIterator : public RelativeBitReference
	{
	public:

		/** Constructor. */
		__forceinline ConstDualSetBitIterator(
			const BitArray<Allocator>& InArrayA,
			const BitArray<OtherAllocator>& InArrayB,
			int32 StartIndex = 0
		)
			: RelativeBitReference(StartIndex)
			, ArrayA(InArrayA)
			, ArrayB(InArrayB)
			, UnvisitedBitMask((~0) << (StartIndex & (NumBitsPerDWORD - 1)))
			, CurrentBitIndex(StartIndex)
			, BaseBitIndex(StartIndex & ~(NumBitsPerDWORD - 1))
		{
			Assert(ArrayA.Size() == ArrayB.Size());

			FindFirstSetBit();
		}

		/** Advancement operator. */
		__forceinline ConstDualSetBitIterator& operator++()
		{
			Assert(ArrayA.Size() == ArrayB.Size());

			// Mark the current bit as visited.
			UnvisitedBitMask &= ~this->Mask;

			// Find the first set bit that hasn't been visited yet.
			FindFirstSetBit();

			return *this;

		}

		/** conversion to "bool" returning true if the iterator is valid. */
		__forceinline explicit operator bool() const
		{
			return CurrentBitIndex < ArrayA.Size();
		}
		/** inverse of the "bool" operator */
		__forceinline bool operator !() const
		{
			return !(bool)*this;
		}

		/** Index accessor. */
		__forceinline int32 GetIndex() const
		{
			return CurrentBitIndex;
		}

	private:

		const BitArray<Allocator>& ArrayA;
		const BitArray<OtherAllocator>& ArrayB;

		uint32 UnvisitedBitMask;
		int32 CurrentBitIndex;
		int32 BaseBitIndex;

		/** Find the first bit that is set in both arrays, starting with the current bit, inclusive. */
		void FindFirstSetBit()
		{
			static const uint32 EmptyArrayData = 0;
			const uint32* ArrayDataA = IfAThenAElseB(ArrayA.Data(), &EmptyArrayData);
			const uint32* ArrayDataB = IfAThenAElseB(ArrayB.Data(), &EmptyArrayData);

			// Advance to the next non-zero uint32.
			uint32 RemainingBitMask = ArrayDataA[this->DWORDIndex] & ArrayDataB[this->DWORDIndex] & UnvisitedBitMask;
			while (!RemainingBitMask)
			{
				this->DWORDIndex++;
				BaseBitIndex += NumBitsPerDWORD;
				const int32 LastDWORDIndex = (ArrayA.Size() - 1) / NumBitsPerDWORD;
				if (this->DWORDIndex <= LastDWORDIndex)
				{
					RemainingBitMask = ArrayDataA[this->DWORDIndex] & ArrayDataB[this->DWORDIndex];
					UnvisitedBitMask = ~0;
				}
				else
				{
					// We've advanced past the end of the array.
					CurrentBitIndex = ArrayA.Size();
					return;
				}
			};

			// We can assume that RemainingBitMask!=0 here.
			Assert(RemainingBitMask);

			// This operation has the effect of unsetting the lowest set bit of BitMask
			const uint32 NewRemainingBitMask = RemainingBitMask & (RemainingBitMask - 1);

			// This operation XORs the above mask with the original mask, which has the effect
			// of returning only the bits which differ; specifically, the lowest bit
			this->Mask = NewRemainingBitMask ^ RemainingBitMask;

			// If the Nth bit was the lowest set bit of BitMask, then this gives us N
			CurrentBitIndex = BaseBitIndex + NumBitsPerDWORD - 1 - Math::CountLeadingZeros(this->Mask);
		}
	};
}