#pragma once

#include "../Core/Template.h"
#include "../Core/Memory.h"
#include "../Core/Assertion.h"

namespace EDX
{
	/** Used to determine the alignment of an element type. */
	template<typename ElementType, bool IsClass = IsClass<ElementType>::Value>
	class ElementAlignmentCalculator
	{
		/**
		* We use a dummy FAlignedElement type that's used to calculate the padding added between the byte and the element
		* to fulfill the type's required alignment.
		*
		* Its default constructor and destructor are declared but never implemented to avoid the need for a ElementType default constructor.
		*/

	private:
		/**
		* In the case of class ElementTypes, we inherit it to allow abstract types to work.
		*/
		struct AlignedElements : ElementType
		{
			uint8 MisalignmentPadding;

			AlignedElements() = delete;
			~AlignedElements() = delete;
		};

		// We calculate the alignment here and then handle the zero case in the result by forwarding it to the non-class variant.
		// This is necessary because the compiler can perform empty-base-optimization to eliminate a redundant ElementType state.
		// Forwarding it to the non-class implementation should always work because an abstract type should never be empty.
		enum { CalculatedAlignment = sizeof(AlignedElements) - sizeof(ElementType) };

	public:
		enum { Value = ChooseClass<CalculatedAlignment != 0, IntegralConstant<SIZE_T, CalculatedAlignment>, ElementAlignmentCalculator<ElementType, false>>::Result::Value };
	};

	template<typename ElementType>
	class ElementAlignmentCalculator<ElementType, false>
	{
	private:
		/**
		* In the case of non-class ElementTypes, we contain it because non-class types cannot be inherited.
		*/
		struct AlignedElements
		{
			uint8 MisalignmentPadding;
			ElementType Element;

			AlignedElements();
			~AlignedElements();
		};
	public:
		enum { Value = sizeof(AlignedElements) - sizeof(ElementType) };
	};

#define ALIGNOF(T) (ElementAlignmentCalculator<T>::Value)


	/** branchless pointer selection
	* return A ? A : B;
	**/
	template<typename ReferencedType>
	ReferencedType* IfAThenAElseB(ReferencedType* A, ReferencedType* B);

	/** branchless pointer selection based on predicate
	* return PTRINT(Predicate) ? A : B;
	**/
	template<typename PredicateType, typename ReferencedType>
	ReferencedType* IfPThenAElseB(PredicateType Predicate, ReferencedType* A, ReferencedType* B);

	__forceinline int32 DefaultCalculateSlackShrink(int32 NumElements, int32 NumAllocatedElements, SIZE_T BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		int32 Retval;
		Assert(NumElements < NumAllocatedElements);

		// If the container has too much slack, shrink it to exactly fit the number of elements.
		const uint32 CurrentSlackElements = NumAllocatedElements - NumElements;
		const SIZE_T CurrentSlackBytes = (NumAllocatedElements - NumElements)*BytesPerElement;
		const bool bTooManySlackBytes = CurrentSlackBytes >= 16384;
		const bool bTooManySlackElements = 3 * NumElements < 2 * NumAllocatedElements;
		if ((bTooManySlackBytes || bTooManySlackElements) && (CurrentSlackElements > 64 || !NumElements)) //  hard coded 64 :-(
		{
			Retval = NumElements;
			if (Retval > 0)
			{
				if (bAllowQuantize)
				{
					Retval = Memory::QuantizeSize(Retval * BytesPerElement, Alignment) / BytesPerElement;
				}
			}
		}
		else
		{
			Retval = NumAllocatedElements;
		}

		return Retval;
	}

	__forceinline int32 DefaultCalculateSlackGrow(int32 NumElements, int32 NumAllocatedElements, SIZE_T BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		int32 Retval;
		Assert(NumElements > NumAllocatedElements && NumElements > 0);

		SIZE_T Grow = 4; // this is the amount for the first alloc
		if (NumAllocatedElements || SIZE_T(NumElements) > Grow)
		{
			// Allocate slack for the array proportional to its size.
			Grow = SIZE_T(NumElements) + 3 * SIZE_T(NumElements) / 8 + 16;
		}
		if (bAllowQuantize)
		{
			Retval = Memory::QuantizeSize(Grow * BytesPerElement, Alignment) / BytesPerElement;
		}
		else
		{
			Retval = Grow;
		}
		// NumElements and MaxElements are stored in 32 bit signed integers so we must be careful not to overflow here.
		if (NumElements > Retval)
		{
			Retval = int32(Math::EDX_INFINITY);
		}

		return Retval;
	}

	__forceinline int32 DefaultCalculateSlackReserve(int32 NumElements, SIZE_T BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		int32 Retval = NumElements;
		Assert(NumElements > 0);
		if (bAllowQuantize)
		{
			Retval = Memory::QuantizeSize(SIZE_T(Retval) * SIZE_T(BytesPerElement), Alignment) / BytesPerElement;
			// NumElements and MaxElements are stored in 32 bit signed integers so we must be careful not to overflow here.
			if (NumElements > Retval)
			{
				Retval = int32(Math::EDX_INFINITY);
			}
		}

		return Retval;
	}

	/** A type which is used to represent a script type that is unknown at compile time. */
	struct ScriptContainerElement
	{
	};

	/**
	* Used to declare an untyped array of data with compile-time alignment.
	* It needs to use template specialization as the MS_ALIGN and GCC_ALIGN macros require literal parameters.
	*/
	template<int32 Size, uint32 Alignment>
	struct AlignedBytes; // this intentionally won't compile, we don't support the requested alignment

						  /** Unaligned storage. */
	template<int32 Size>
	struct AlignedBytes<Size, 1>
	{
		uint8 Pad[Size];
	};


	// C++/CLI doesn't support alignment of native types in managed code, so we enforce that the element
	// size is a multiple of the desired alignment
#ifdef __cplusplus_cli
#define IMPLEMENT_ALIGNED_STORAGE(Align) \
		template<int32 Size>        \
		struct AlignedBytes<Size,Align> \
		{ \
			uint8 Pad[Size]; \
			static_assert(Size % Align == 0, "CLR interop types must not be aligned."); \
		};
#else
	/** A macro that implements AlignedBytes for a specific alignment. */
#define IMPLEMENT_ALIGNED_STORAGE(Align) \
	template<int32 Size>        \
	struct AlignedBytes<Size,Align> \
	{ \
		struct __declspec(align(Align)) TPadding \
		{ \
			uint8 Pad[Size]; \
		}; \
		TPadding Padding; \
	};
#endif

	// Implement AlignedBytes for these alignments.
	IMPLEMENT_ALIGNED_STORAGE(16);
	IMPLEMENT_ALIGNED_STORAGE(8);
	IMPLEMENT_ALIGNED_STORAGE(4);
	IMPLEMENT_ALIGNED_STORAGE(2);

#undef IMPLEMENT_ALIGNED_STORAGE

	/** An untyped array of data with compile-time alignment and size derived from another type. */
	template<typename ElementType>
	struct TypeCompatibleBytes :
		public AlignedBytes<
		sizeof(ElementType),
		ALIGNOF(ElementType)
		>
	{};

	template <typename AllocatorType>
	struct AllocatorTraitsBase
	{
		enum { SupportsMove = false };
		enum { IsZeroConstruct = false };
	};

	template <typename AllocatorType>
	struct AllocatorTraits : AllocatorTraitsBase<AllocatorType>
	{
	};

	/** This is the allocation policy interface; it exists purely to document the policy's interface, and should not be used. */
	class ContainerAllocatorInterface
	{
	public:

		/** Determines whether the user of the allocator may use the ForAnyElementType inner class. */
		enum { NeedsElementType = true };
		enum { RequireRangeCheck = true };

		/**
		* A class that receives both the explicit allocation policy template parameters specified by the user of the container,
		* but also the implicit ElementType template parameter from the container type.
		*/
		template<typename ElementType>
		class ForElementType
		{
			/**
			* Moves the state of another allocator into this one.
			* Assumes that the allocator is currently empty, i.e. memory may be allocated but any existing elements have already been destructed (if necessary).
			* @param Other - The allocator to move the state from.  This allocator should be left in a valid empty state.
			*/
			void MoveToEmpty(ForElementType& Other);

			/** Accesses the container's current data. */
			ElementType* GetAllocation() const;

			/**
			* Resizes the container's allocation.
			* @param PreviousNumElements - The number of elements that were stored in the previous allocation.
			* @param NumElements - The number of elements to allocate space for.
			* @param NumBytesPerElement - The number of bytes/element.
			*/
			void ResizeAllocation(
				int32 PreviousNumElements,
				int32 NumElements,
				SIZE_T NumBytesPerElement
			);

			/**
			* Calculates the amount of slack to allocate for an array that has just grown or shrunk to a given number of elements.
			* @param NumElements - The number of elements to allocate space for.
			* @param CurrentNumSlackElements - The current number of elements allocated.
			* @param NumBytesPerElement - The number of bytes/element.
			*/
			int32 CalculateSlack(
				int32 NumElements,
				int32 CurrentNumSlackElements,
				SIZE_T NumBytesPerElement
			) const;

			/**
			* Calculates the amount of slack to allocate for an array that has just shrunk to a given number of elements.
			* @param NumElements - The number of elements to allocate space for.
			* @param CurrentNumSlackElements - The current number of elements allocated.
			* @param NumBytesPerElement - The number of bytes/element.
			*/
			int32 CalculateSlackShrink(
				int32 NumElements,
				int32 CurrentNumSlackElements,
				SIZE_T NumBytesPerElement
			) const;

			/**
			* Calculates the amount of slack to allocate for an array that has just grown to a given number of elements.
			* @param NumElements - The number of elements to allocate space for.
			* @param CurrentNumSlackElements - The current number of elements allocated.
			* @param NumBytesPerElement - The number of bytes/element.
			*/
			int32 CalculateSlackGrow(
				int32 NumElements,
				int32 CurrentNumSlackElements,
				SIZE_T NumBytesPerElement
			) const;

			SIZE_T GetAllocatedSize(int32 NumAllocatedElements, SIZE_T NumBytesPerElement) const;
		};

		/**
		* A class that may be used when NeedsElementType=false is specified.
		* If NeedsElementType=true, then this must be present but will not be used, and so can simply be a typedef to void
		*/
		typedef ForElementType<ScriptContainerElement> ForAnyElementType;
	};

	/** The indirect allocation policy always allocates the elements indirectly. */
	template<uint32 Alignment = DEFAULT_ALIGNMENT>
	class AlignedHeapAllocator
	{
	public:

		enum { NeedsElementType = false };
		enum { RequireRangeCheck = true };

		class ForAnyElementType
		{
		private:
			/** A pointer to the container's elements. */
			ScriptContainerElement* Data;

		public:

			/** Default constructor. */
			ForAnyElementType()
				: Data(nullptr)
			{}

			ForAnyElementType(const ForAnyElementType&) = delete;
			ForAnyElementType& operator=(const ForAnyElementType&) = delete;

			/**
			* Moves the state of another allocator into this one.
			* Assumes that the allocator is currently empty, i.e. memory may be allocated but any existing elements have already been destructed (if necessary).
			* @param Other - The allocator to move the state from.  This allocator should be left in a valid empty state.
			*/
			__forceinline void MoveToEmpty(ForAnyElementType& Other)
			{
				Assert(this != &Other);

				if (Data)
				{
					Memory::Free(Data);
				}

				Data = Other.Data;
				Other.Data = nullptr;
			}

			/** Destructor. */
			__forceinline ~ForAnyElementType()
			{
				if (Data)
				{
					Memory::Free(Data);
				}
			}

			// ContainerAllocatorInterface
			__forceinline ScriptContainerElement* GetAllocation() const
			{
				return Data;
			}
			void ResizeAllocation(
				int32 PreviousNumElements,
				int32 NumElements,
				SIZE_T NumBytesPerElement
			)
			{
				// Avoid calling Memory::AlignedRealloc( nullptr, 0 ) as ANSI C mandates returning a valid pointer which is not what we want.
				if (Data || NumElements)
				{
					//check(((uint64)NumElements*(uint64)ElementTypeInfo.GetSize() < (uint64)INT_MAX));
					Data = (ScriptContainerElement*)Memory::AlignedRealloc(Data, NumElements*NumBytesPerElement, Alignment);
				}
			}
			__forceinline int32 CalculateSlackReserve(int32 NumElements, int32 NumBytesPerElement) const
			{
				return DefaultCalculateSlackReserve(NumElements, NumBytesPerElement, true, Alignment);
			}
			__forceinline int32 CalculateSlackShrink(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
			{
				return DefaultCalculateSlackShrink(NumElements, NumAllocatedElements, NumBytesPerElement, true, Alignment);
			}
			__forceinline int32 CalculateSlackGrow(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
			{
				return DefaultCalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement, true, Alignment);
			}

			SIZE_T GetAllocatedSize(int32 NumAllocatedElements, SIZE_T NumBytesPerElement) const
			{
				return NumAllocatedElements * NumBytesPerElement;
			}

			bool HasAllocation()
			{
				return !!Data;
			}
		};

		template<typename ElementType>
		class ForElementType : public ForAnyElementType
		{
		public:

			/** Default constructor. */
			ForElementType()
			{}

			__forceinline ElementType* GetAllocation() const
			{
				return (ElementType*)ForAnyElementType::GetAllocation();
			}
		};
	};

	template <uint32 Alignment>
	struct AllocatorTraits<AlignedHeapAllocator<Alignment>> : AllocatorTraitsBase<AlignedHeapAllocator<Alignment>>
	{
		enum { SupportsMove = true };
		enum { IsZeroConstruct = true };
	};

	/** The indirect allocation policy always allocates the elements indirectly. */
	class HeapAllocator
	{
	public:

		enum { NeedsElementType = false };
		enum { RequireRangeCheck = true };

		class ForAnyElementType
		{
		private:
			/** A pointer to the container's elements. */
			ScriptContainerElement* Data;

		public:
			/** Default constructor. */
			ForAnyElementType()
				: Data(nullptr)
			{}

			ForAnyElementType(const ForAnyElementType&) = delete;
			ForAnyElementType& operator=(const ForAnyElementType&) = delete;

			/**
			* Moves the state of another allocator into this one.
			* Assumes that the allocator is currently empty, i.e. memory may be allocated but any existing elements have already been destructed (if necessary).
			* @param Other - The allocator to move the state from.  This allocator should be left in a valid empty state.
			*/
			__forceinline void MoveToEmpty(ForAnyElementType& Other)
			{
				Assert(this != &Other);

				if (Data)
				{
					Memory::Free(Data);
				}

				Data = Other.Data;
				Other.Data = nullptr;
			}

			/** Destructor. */
			__forceinline ~ForAnyElementType()
			{
				if (Data)
				{
					Memory::Free(Data);
				}
			}

			// ContainerAllocatorInterface
			__forceinline ScriptContainerElement* GetAllocation() const
			{
				return Data;
			}
			__forceinline void ResizeAllocation(int32 PreviousNumElements, int32 NumElements, SIZE_T NumBytesPerElement)
			{
				// Avoid calling Memory::AlignedRealloc( nullptr, 0 ) as ANSI C mandates returning a valid pointer which is not what we want.
				if (Data || NumElements)
				{
					//check(((uint64)NumElements*(uint64)ElementTypeInfo.GetSize() < (uint64)INT_MAX));
					Data = (ScriptContainerElement*)Memory::AlignedRealloc(Data, NumElements*NumBytesPerElement);
				}
			}
			__forceinline int32 CalculateSlackReserve(int32 NumElements, int32 NumBytesPerElement) const
			{
				return DefaultCalculateSlackReserve(NumElements, NumBytesPerElement, true);
			}
			__forceinline int32 CalculateSlackShrink(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
			{
				return DefaultCalculateSlackShrink(NumElements, NumAllocatedElements, NumBytesPerElement, true);
			}
			__forceinline int32 CalculateSlackGrow(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
			{
				return DefaultCalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement, true);
			}

			SIZE_T GetAllocatedSize(int32 NumAllocatedElements, SIZE_T NumBytesPerElement) const
			{
				return NumAllocatedElements * NumBytesPerElement;
			}

			bool HasAllocation()
			{
				return !!Data;
			}
		};

		template<typename ElementType>
		class ForElementType : public ForAnyElementType
		{
		public:

			/** Default constructor. */
			ForElementType()
			{}

			__forceinline ElementType* GetAllocation() const
			{
				return (ElementType*)ForAnyElementType::GetAllocation();
			}
		};
	};

	template <>
	struct AllocatorTraits<HeapAllocator> : AllocatorTraitsBase<HeapAllocator>
	{
		enum { SupportsMove = true };
		enum { IsZeroConstruct = true };
	};

	class DefaultAllocator;

	/**
	* The inline allocation policy allocates up to a specified number of elements in the same allocation as the container.
	* Any allocation needed beyond that causes all data to be moved into an indirect allocation.
	* It always uses DEFAULT_ALIGNMENT.
	*/
	template <uint32 NumInlineElements, typename SecondaryAllocator = DefaultAllocator>
	class InlineAllocator
	{
	public:

		enum { NeedsElementType = true };
		enum { RequireRangeCheck = true };

		template<typename ElementType>
		class ForElementType
		{
		private:
			/** The data is stored in this array if less than NumInlineElements is needed. */
			TypeCompatibleBytes<ElementType> InlineData[NumInlineElements];

			/** The data is allocated through the indirect allocation policy if more than NumInlineElements is needed. */
			typename SecondaryAllocator::template ForElementType<ElementType> SecondaryData;

		public:

			/** Default constructor. */
			ForElementType()
			{
			}

			ForElementType(const ForElementType&) = delete;
			ForElementType& operator=(const ForElementType&) = delete;

			/**
			* Moves the state of another allocator into this one.
			* Assumes that the allocator is currently empty, i.e. memory may be allocated but any existing elements have already been destructed (if necessary).
			* @param Other - The allocator to move the state from.  This allocator should be left in a valid empty state.
			*/
			__forceinline void MoveToEmpty(ForElementType& Other)
			{
				Assert(this != &Other);

				if (!Other.SecondaryData.GetAllocation())
				{
					// Relocate objects from other inline storage only if it was stored inline in Other
					RelocateConstructItems<ElementType>((void*)InlineData, Other.GetInlineElements(), NumInlineElements);
				}

				// Move secondary storage in any case.
				// This will move secondary storage if it exists but will also handle the case where secondary storage is used in Other but not in *this.
				SecondaryData.MoveToEmpty(Other.SecondaryData);
			}

			// ContainerAllocatorInterface
			__forceinline ElementType* GetAllocation() const
			{
				return IfAThenAElseB<ElementType>(SecondaryData.GetAllocation(), GetInlineElements());
			}

			void ResizeAllocation(int32 PreviousNumElements, int32 NumElements, SIZE_T NumBytesPerElement)
			{
				// Check if the new allocation will fit in the inline data area.
				if (NumElements <= NumInlineElements)
				{
					// If the old allocation wasn't in the inline data area, relocate it into the inline data area.
					if (SecondaryData.GetAllocation())
					{
						RelocateConstructItems<ElementType>((void*)InlineData, (ElementType*)SecondaryData.GetAllocation(), PreviousNumElements);

						// Free the old indirect allocation.
						SecondaryData.ResizeAllocation(0, 0, NumBytesPerElement);
					}
				}
				else
				{
					if (!SecondaryData.GetAllocation())
					{
						// Allocate new indirect memory for the data.
						SecondaryData.ResizeAllocation(0, NumElements, NumBytesPerElement);

						// Move the data out of the inline data area into the new allocation.
						RelocateConstructItems<ElementType>((void*)SecondaryData.GetAllocation(), GetInlineElements(), PreviousNumElements);
					}
					else
					{
						// Reallocate the indirect data for the new size.
						SecondaryData.ResizeAllocation(PreviousNumElements, NumElements, NumBytesPerElement);
					}
				}
			}

			__forceinline int32 CalculateSlackReserve(int32 NumElements, SIZE_T NumBytesPerElement) const
			{
				// If the elements use less space than the inline allocation, only use the inline allocation as slack.
				return NumElements <= NumInlineElements ?
					NumInlineElements :
					SecondaryData.CalculateSlackReserve(NumElements, NumBytesPerElement);
			}
			__forceinline int32 CalculateSlackShrink(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
			{
				// If the elements use less space than the inline allocation, only use the inline allocation as slack.
				return NumElements <= NumInlineElements ?
					NumInlineElements :
					SecondaryData.CalculateSlackShrink(NumElements, NumAllocatedElements, NumBytesPerElement);
			}
			__forceinline int32 CalculateSlackGrow(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
			{
				// If the elements use less space than the inline allocation, only use the inline allocation as slack.
				return NumElements <= NumInlineElements ?
					NumInlineElements :
					SecondaryData.CalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement);
			}

			SIZE_T GetAllocatedSize(int32 NumAllocatedElements, SIZE_T NumBytesPerElement) const
			{
				return SecondaryData.GetAllocatedSize(NumAllocatedElements, NumBytesPerElement);
			}

			bool HasAllocation()
			{
				return SecondaryData.HasAllocation();
			}

		private:
			/** @return the base of the aligned inline element data */
			ElementType* GetInlineElements() const
			{
				return (ElementType*)InlineData;
			}
		};

		typedef void ForAnyElementType;
	};

	template <uint32 NumInlineElements, typename SecondaryAllocator>
	struct AllocatorTraits<InlineAllocator<NumInlineElements, SecondaryAllocator>> : AllocatorTraitsBase<InlineAllocator<NumInlineElements, SecondaryAllocator>>
	{
		enum { SupportsMove = AllocatorTraits<SecondaryAllocator>::SupportsMove };
	};

	/**
	* The fixed allocation policy allocates up to a specified number of elements in the same allocation as the container.
	* It's like the inline allocator, except it doesn't provide secondary storage when the inline storage has been filled.
	*/
	template <uint32 NumInlineElements>
	class FixedAllocator
	{
	public:

		enum { NeedsElementType = true };
		enum { RequireRangeCheck = true };

		template<typename ElementType>
		class ForElementType
		{
		public:

			/** Default constructor. */
			ForElementType()
			{
			}

			/**
			* Moves the state of another allocator into this one.
			* Assumes that the allocator is currently empty, i.e. memory may be allocated but any existing elements have already been destructed (if necessary).
			* @param Other - The allocator to move the state from.  This allocator should be left in a valid empty state.
			*/
			__forceinline void MoveToEmpty(ForElementType& Other)
			{
				Assert(this != &Other);

				// Relocate objects from other inline storage
				RelocateConstructItems<ElementType>((void*)InlineData, Other.GetInlineElements(), NumInlineElements);
			}

			// FContainerAllocatorInterface
			__forceinline ElementType* GetAllocation() const
			{
				return GetInlineElements();
			}

			void ResizeAllocation(int32 PreviousNumElements, int32 NumElements, SIZE_T NumBytesPerElement)
			{
				// Ensure the requested allocation will fit in the inline data area.
				Assert(NumElements <= NumInlineElements);
			}

			__forceinline int32 CalculateSlackReserve(int32 NumElements, SIZE_T NumBytesPerElement) const
			{
				// Ensure the requested allocation will fit in the inline data area.
				Assert(NumElements <= NumInlineElements);
				return NumInlineElements;
			}
			__forceinline int32 CalculateSlackShrink(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
			{
				// Ensure the requested allocation will fit in the inline data area.
				Assert(NumAllocatedElements <= NumInlineElements);
				return NumInlineElements;
			}
			__forceinline int32 CalculateSlackGrow(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
			{
				// Ensure the requested allocation will fit in the inline data area.
				Assert(NumElements <= NumInlineElements);
				return NumInlineElements;
			}

			SIZE_T GetAllocatedSize(int32 NumAllocatedElements, SIZE_T NumBytesPerElement) const
			{
				return 0;
			}

			bool HasAllocation()
			{
				return false;
			}


		private:
			ForElementType(const ForElementType&);
			ForElementType& operator=(const ForElementType&);

			/** The data is stored in this array if less than NumInlineElements is needed. */
			TypeCompatibleBytes<ElementType> InlineData[NumInlineElements];

			/** @return the base of the aligned inline element data */
			ElementType* GetInlineElements() const
			{
				return (ElementType*)InlineData;
			}
		};

		typedef void ForAnyElementType;
	};

	template <uint32 NumInlineElements>
	struct AllocatorTraits<FixedAllocator<NumInlineElements>> : AllocatorTraitsBase<FixedAllocator<NumInlineElements>>
	{
		enum { SupportsMove = true };
	};

	// We want these to be correctly typed as int32, but we don't want them to have linkage, so we make them macros
#define NumBitsPerDWORD ((int32)32)
#define NumBitsPerDWORDLogTwo ((int32)5)

	//
	// Sparse array allocation definitions
	//

	class DefaultAllocator;
	class DefaultBitArrayAllocator;

	/** Encapsulates the allocators used by a sparse array in a single type. */
	template<typename InElementAllocator = DefaultAllocator, typename InBitArrayAllocator = DefaultBitArrayAllocator>
	class SparseArrayAllocator
	{
	public:

		typedef InElementAllocator ElementAllocator;
		typedef InBitArrayAllocator BitArrayAllocator;
	};

	/** An inline sparse array allocator that allows sizing of the inline allocations for a set number of elements. */
	template<
		uint32 NumInlineElements,
		typename SecondaryAllocator = SparseArrayAllocator<DefaultAllocator, DefaultAllocator>
	>
		class InlineSparseArrayAllocator
	{
	private:

		/** The size to allocate inline for the bit array. */
		enum { InlineBitArrayDWORDs = (NumInlineElements + NumBitsPerDWORD - 1) / NumBitsPerDWORD };

	public:

		typedef InlineAllocator<NumInlineElements, typename SecondaryAllocator::ElementAllocator>		ElementAllocator;
		typedef InlineAllocator<InlineBitArrayDWORDs, typename SecondaryAllocator::BitArrayAllocator>	BitArrayAllocator;
	};

	//
	// Set allocation definitions.
	//

#define DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET	2
#define DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS			8
#define DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS		4

	/** Encapsulates the allocators used by a set in a single type. */
	template<
		typename InSparseArrayAllocator = SparseArrayAllocator<>,
		typename InHashAllocator = InlineAllocator<1, DefaultAllocator>,
		uint32   AverageNumberOfElementsPerHashBucket = DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET,
		uint32   BaseNumberOfHashBuckets = DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS,
		uint32   MinNumberOfHashedElements = DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS
	>
	class SetAllocator
	{
	public:

		/** Computes the number of hash buckets to use for a given number of elements. */
		static __forceinline uint32 GetNumberOfHashBuckets(uint32 NumHashedElements)
		{
			if (NumHashedElements >= MinNumberOfHashedElements)
			{
				return Math::RoundUpPowOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket + BaseNumberOfHashBuckets);
			}

			return 1;
		}

		typedef InSparseArrayAllocator SparseArrayAllocator;
		typedef InHashAllocator        HashAllocator;
	};

	class DefaultAllocator;

	/** An inline set allocator that allows sizing of the inline allocations for a set number of elements. */
	template<
		uint32   NumInlineElements,
		typename SecondaryAllocator = SetAllocator<SparseArrayAllocator<DefaultAllocator, DefaultAllocator>, DefaultAllocator>,
		uint32   AverageNumberOfElementsPerHashBucket = DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET,
		uint32   MinNumberOfHashedElements = DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS
	>
		class InlineSetAllocator
	{
	private:

		enum { NumInlineHashBuckets = (NumInlineElements + AverageNumberOfElementsPerHashBucket - 1) / AverageNumberOfElementsPerHashBucket };

	public:

		/** Computes the number of hash buckets to use for a given number of elements. */
		static __forceinline uint32 GetNumberOfHashBuckets(uint32 NumHashedElements)
		{
			const uint32 NumDesiredHashBuckets = Math::RoundUpPowOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket);
			if (NumDesiredHashBuckets < NumInlineHashBuckets)
			{
				return NumInlineHashBuckets;
			}

			if (NumHashedElements < MinNumberOfHashedElements)
			{
				return NumInlineHashBuckets;
			}

			return NumDesiredHashBuckets;
		}

		typedef InlineSparseArrayAllocator<NumInlineElements, typename SecondaryAllocator::SparseArrayAllocator> SparseArrayAllocator;
		typedef InlineAllocator<NumInlineHashBuckets, typename SecondaryAllocator::HashAllocator>                HashAllocator;
	};


	/**
	* 'typedefs' for various allocator defaults.
	*
	* These should be replaced with actual typedefs when Core.h include order is sorted out, as then we won't need to
	* 'forward' these AllocatorTraits specializations below.
	*/

	class DefaultAllocator : public HeapAllocator { public: typedef HeapAllocator          Typedef; };
	class DefaultSetAllocator : public SetAllocator<> { public: typedef SetAllocator<>         Typedef; };
	class DefaultBitArrayAllocator : public InlineAllocator<4> { public: typedef InlineAllocator<4>     Typedef; };
	class DefaultSparseArrayAllocator : public SparseArrayAllocator<> { public: typedef SparseArrayAllocator<> Typedef; };

	template <> struct AllocatorTraits<DefaultAllocator> : AllocatorTraits<typename DefaultAllocator::Typedef> {};
	template <> struct AllocatorTraits<DefaultSetAllocator> : AllocatorTraits<typename DefaultSetAllocator::Typedef> {};
	template <> struct AllocatorTraits<DefaultBitArrayAllocator> : AllocatorTraits<typename DefaultBitArrayAllocator::Typedef> {};
	template <> struct AllocatorTraits<DefaultSparseArrayAllocator> : AllocatorTraits<typename DefaultSparseArrayAllocator::Typedef> {};

}