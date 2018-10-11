#pragma once

#include "../Core/Types.h"
#include "../Core/Template.h"
#include "../Math/EDXMath.h"
#include "../Core/Assertion.h"

namespace EDX
{
	enum Alignment
	{
		// Default allocator alignment.
		// Blocks >= 16 bytes will be 16-byte-aligned, Blocks < 16 will be 8-byte aligned. If the allocator does
		// not support allocation alignment, the alignment will be ignored.
		DEFAULT_ALIGNMENT = 0,

		// Minimum allocator alignment
		MIN_ALIGNMENT = 8,
	};

	class Memory
	{
	public:

		static __forceinline void* Memmove(void* Dest, const void* Src, size_t Count)
		{
			return memmove(Dest, Src, Count);
		}

		static __forceinline int32 Memcmp(const void* Buf1, const void* Buf2, size_t Count)
		{
			return memcmp(Buf1, Buf2, Count);
		}

		static __forceinline void* Memset(void* Dest, uint8 Char, size_t Count)
		{
			return memset(Dest, Char, Count);
		}

		template<class T>
		static __forceinline void Memset(T& Src, uint8 ValueToSet)
		{
			static_assert(!IsPointerType<T>::Value, "For pointers use the three parameters function");
			Memset(&Src, ValueToSet, sizeof(T));
		}

		static __forceinline void* Memzero(void* Dest, size_t Count)
		{
			return memset(Dest, 0, Count);
		}

		template<class T>
		static __forceinline void Memzero(T& Src)
		{
			static_assert(!IsPointerType<T>::Value, "For pointers use the two parameters function");
			Memzero(&Src, sizeof(T));
		}

		static __forceinline void* Memcpy(void* Dest, const void* Src, size_t Count)
		{
			return memcpy(Dest, Src, Count);
		}

		template<class T>
		static __forceinline void Memcpy(T& Dest, const T& Src)
		{
			static_assert(!IsPointerType<T>::Value, "For pointers use the three parameters function");
			Memcpy(&Dest, &Src, sizeof(T));
		}

		template <typename T>
		static __forceinline void Valswap(T& A, T& B)
		{
			// Usually such an implementation would use move semantics, but
			// we're only ever going to call it on fundamental types and Move
			// is not necessarily in scope here anyway, so we don't want to
			// #include it if we don't need to.
			T Tmp = A;
			A = B;
			B = Tmp;
		}

		static inline void MemswapGreaterThan8(void* Ptr1, void* Ptr2, SIZE_T Size)
		{
			union PtrUnion
			{
				void*   PtrVoid;
				uint8*  Ptr8;
				uint16* Ptr16;
				uint32* Ptr32;
				uint64* Ptr64;
				UPTRINT PtrUint;
			};

			PtrUnion Union1 = { Ptr1 };
			PtrUnion Union2 = { Ptr2 };

			// We may skip up to 7 bytes below, so better make sure that we're swapping more than that
			// (8 is a common case that we also want to inline before we this call, so skip that too)
			Assert(Size > 8);

			if (Union1.PtrUint & 1)
			{
				Valswap(*Union1.Ptr8++, *Union2.Ptr8++);
				Size -= 1;
			}
			if (Union1.PtrUint & 2)
			{
				Valswap(*Union1.Ptr16++, *Union2.Ptr16++);
				Size -= 2;
			}
			if (Union1.PtrUint & 4)
			{
				Valswap(*Union1.Ptr32++, *Union2.Ptr32++);
				Size -= 4;
			}

			uint32 CommonAlignment = Math::Min(Math::CountTrailingZeros(Union1.PtrUint - Union2.PtrUint), 3u);
			switch (CommonAlignment)
			{
			default:
				for (; Size >= 8; Size -= 8)
				{
					Valswap(*Union1.Ptr64++, *Union2.Ptr64++);
				}

			case 2:
				for (; Size >= 4; Size -= 4)
				{
					Valswap(*Union1.Ptr32++, *Union2.Ptr32++);
				}

			case 1:
				for (; Size >= 2; Size -= 2)
				{
					Valswap(*Union1.Ptr16++, *Union2.Ptr16++);
				}

			case 0:
				for (; Size >= 1; Size -= 1)
				{
					Valswap(*Union1.Ptr8++, *Union2.Ptr8++);
				}
			}
		}


		static inline void Memswap(void* Ptr1, void* Ptr2, SIZE_T Size)
		{
			switch (Size)
			{
			case 0:
				break;

			case 1:
				Valswap(*(uint8*)Ptr1, *(uint8*)Ptr2);
				break;

			case 2:
				Valswap(*(uint16*)Ptr1, *(uint16*)Ptr2);
				break;

			case 3:
				Valswap(*((uint16*&)Ptr1)++, *((uint16*&)Ptr2)++);
				Valswap(*(uint8*)Ptr1, *(uint8*)Ptr2);
				break;

			case 4:
				Valswap(*(uint32*)Ptr1, *(uint32*)Ptr2);
				break;

			case 5:
				Valswap(*((uint32*&)Ptr1)++, *((uint32*&)Ptr2)++);
				Valswap(*(uint8*)Ptr1, *(uint8*)Ptr2);
				break;

			case 6:
				Valswap(*((uint32*&)Ptr1)++, *((uint32*&)Ptr2)++);
				Valswap(*(uint16*)Ptr1, *(uint16*)Ptr2);
				break;

			case 7:
				Valswap(*((uint32*&)Ptr1)++, *((uint32*&)Ptr2)++);
				Valswap(*((uint16*&)Ptr1)++, *((uint16*&)Ptr2)++);
				Valswap(*(uint8*)Ptr1, *(uint8*)Ptr2);
				break;

			case 8:
				Valswap(*(uint64*)Ptr1, *(uint64*)Ptr2);
				break;

			case 16:
				Valswap(((uint64*)Ptr1)[0], ((uint64*)Ptr2)[0]);
				Valswap(((uint64*)Ptr1)[1], ((uint64*)Ptr2)[1]);
				break;

			default:
				MemswapGreaterThan8(Ptr1, Ptr2, Size);
				break;
			}
		}

		//
		// C style memory allocation stubs that fall back to C runtime
		//
		static __forceinline void* SystemMalloc(size_t Size)
		{
			return ::malloc(Size);
		}

		static __forceinline void SystemFree(void* Ptr)
		{
			::free(Ptr);
		}

		template<typename T>
		static __forceinline T* AlignedAlloc(uint32 Num, uint32 Alignment = DEFAULT_ALIGNMENT)
		{
			size_t Size = Num * sizeof(T);
			return (T*)AlignedAlloc(Size, Alignment);
		}

		//
		// C style memory allocation stubs.
		//

		static __forceinline void* AlignedAlloc(size_t Size, uint32 Alignment = DEFAULT_ALIGNMENT)
		{
			Alignment = Math::Max(Size >= 16 ? (uint32)16 : (uint32)8, Alignment);

			void* Result = _aligned_malloc(Size, Alignment);
			Assert(Result);

			return Result;
		}

		static void* AlignedRealloc(void* Ptr, size_t NewSize, uint32 Alignment = DEFAULT_ALIGNMENT)
		{
			void* Result;
			Alignment = Math::Max(NewSize >= 16 ? (uint32)16 : (uint32)8, Alignment);

			if (Ptr && NewSize)
			{
				Result = _aligned_realloc(Ptr, NewSize, Alignment);
			}
			else if (Ptr == nullptr)
			{
				Result = _aligned_malloc(NewSize, Alignment);
			}
			else
			{
				_aligned_free(Ptr);
				Result = nullptr;
			}

			if (Result == nullptr && NewSize != 0)
			{
				// Handle out of memory
			}

			return Result;
		}

		static void Free(void* Ptr)
		{
			_aligned_free(Ptr);
		}

		template<class T>
		static void SafeFree(T*& Ptr)
		{
			if (Ptr != nullptr)
			{
				_aligned_free((void*)Ptr);
				Ptr = nullptr;
			}
		}

		static size_t GetAllocSize(void* Ptr)
		{
			if (!Ptr)
			{
				return 0;
			}

			size_t SizeOut = _aligned_msize(Ptr, 16, 0); // Assumes alignment of 16

			return SizeOut;
		}

		/**
		* For some allocators this will return the actual size that should be requested to eliminate
		* internal fragmentation. The return value will always be >= Count. This can be used to grow
		* and shrink containers to optimal sizes.
		* This call is always fast and threadsafe with no locking.
		*/
		static size_t QuantizeSize(size_t Count, uint32 Alignment = DEFAULT_ALIGNMENT)
		{
			return Count;
		}


		template<class T>
		static __forceinline void SafeDelete(T*& pPtr)
		{
			if (pPtr != NULL)
			{
				delete pPtr;
				pPtr = NULL;
			}
		}

		template<class T>
		static __forceinline void SafeDeleteArray(T*& pPtr)
		{
			if (pPtr != NULL)
			{
				delete[] pPtr;
				pPtr = NULL;
			}
		}

		template<class T>
		static __forceinline void SafeClear(T* pPtr, size_t Size)
		{
			if (pPtr != NULL)
			{
				Memzero(pPtr, sizeof(T) * Size);
			}
		}
	};

	namespace MemoryOps_Private
	{
		template <typename DestinationElementType, typename SourceElementType>
		struct CanBitwiseRelocate
		{
			enum
			{
				Value =
				(
					IsBitwiseConstructible<DestinationElementType, SourceElementType>::Value &&
					!TypeTraits<SourceElementType>::NeedsDestructor
					) ||
				AreTypesEqual<DestinationElementType, SourceElementType>::Value
			};
		};
	}

	/**
	* Default constructs a range of items in memory.
	*
	* @param	Elements	The address of the first memory location to construct at.
	* @param	Count		The number of elements to destruct.
	*/
	template <typename ElementType>
	__forceinline typename EnableIf<!IsZeroConstructType<ElementType>::Value>::Type DefaultConstructItems(void* Address, int32 Count)
	{
		ElementType* Element = (ElementType*)Address;
		while (Count)
		{
			new (Element) ElementType;
			++Element;
			--Count;
		}
	}


	template <typename ElementType>
	__forceinline typename EnableIf<IsZeroConstructType<ElementType>::Value>::Type DefaultConstructItems(void* Elements, int32 Count)
	{
		Memory::Memset(Elements, 0, sizeof(ElementType) * Count);
	}


	/**
	* Destructs a range of items in memory.
	*
	* @param	Elements	A pointer to the first item to destruct.
	* @param	Count		The number of elements to destruct.
	*/
	template <typename ElementType>
	__forceinline typename EnableIf<TypeTraits<ElementType>::NeedsDestructor>::Type DestructItems(ElementType* Element, int32 Count)
	{
		while (Count)
		{
			// We need a typedef here because VC won't compile the destructor call below if ElementType itself has a member called ElementType
			typedef ElementType DestructItemsElementTypeTypedef;

			Element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
			++Element;
			--Count;
		}
	}


	template <typename ElementType>
	__forceinline typename EnableIf<!TypeTraits<ElementType>::NeedsDestructor>::Type DestructItems(ElementType* Elements, int32 Count)
	{
	}


	/**
	* Constructs a range of items into memory from a set of arguments.  The arguments come from an another array.
	*
	* @param	Dest		The memory location to start copying into.
	* @param	Source		A pointer to the first argument to pass to the constructor.
	* @param	Count		The number of elements to copy.
	*/
	template <typename DestinationElementType, typename SourceElementType>
	__forceinline typename EnableIf<!IsBitwiseConstructible<DestinationElementType, SourceElementType>::Value>::Type ConstructItems(void* Dest, const SourceElementType* Source, int32 Count)
	{
		while (Count)
		{
			new (Dest) DestinationElementType(*Source);
			++(DestinationElementType*&)Dest;
			++Source;
			--Count;
		}
	}


	template <typename DestinationElementType, typename SourceElementType>
	__forceinline typename EnableIf<IsBitwiseConstructible<DestinationElementType, SourceElementType>::Value>::Type ConstructItems(void* Dest, const SourceElementType* Source, int32 Count)
	{
		Memory::Memcpy(Dest, Source, sizeof(SourceElementType) * Count);
	}

	/**
	* Copy assigns a range of items.
	*
	* @param	Dest		The memory location to start assigning to.
	* @param	Source		A pointer to the first item to assign.
	* @param	Count		The number of elements to assign.
	*/
	template <typename ElementType>
	__forceinline typename EnableIf<TypeTraits<ElementType>::NeedsCopyAssignment>::Type CopyAssignItems(ElementType* Dest, const ElementType* Source, int32 Count)
	{
		while (Count)
		{
			*Dest = *Source;
			++Dest;
			++Source;
			--Count;
		}
	}


	template <typename ElementType>
	__forceinline typename EnableIf<!TypeTraits<ElementType>::NeedsCopyAssignment>::Type CopyAssignItems(ElementType* Dest, const ElementType* Source, int32 Count)
	{
		Memory::Memcpy(Dest, Source, sizeof(ElementType) * Count);
	}


	/**
	* Relocates a range of items to a new memory location as a new type. This is a so-called 'destructive move' for which
	* there is no single operation in C++ but which can be implemented very efficiently in general.
	*
	* @param	Dest		The memory location to relocate to.
	* @param	Source		A pointer to the first item to relocate.
	* @param	Count		The number of elements to relocate.
	*/
	template <typename DestinationElementType, typename SourceElementType>
	__forceinline typename EnableIf<!MemoryOps_Private::CanBitwiseRelocate<DestinationElementType, SourceElementType>::Value>::Type RelocateConstructItems(void* Dest, const SourceElementType* Source, int32 Count)
	{
		while (Count)
		{
			// We need a typedef here because VC won't compile the destructor call below if SourceElementType itself has a member called SourceElementType
			typedef SourceElementType RelocateConstructItemsElementTypeTypedef;

			new (Dest) DestinationElementType(*Source);
			++(DestinationElementType*&)Dest;
			(Source++)->RelocateConstructItemsElementTypeTypedef::~RelocateConstructItemsElementTypeTypedef();
			--Count;
		}
	}

	template <typename DestinationElementType, typename SourceElementType>
	__forceinline typename EnableIf<MemoryOps_Private::CanBitwiseRelocate<DestinationElementType, SourceElementType>::Value>::Type RelocateConstructItems(void* Dest, const SourceElementType* Source, int32 Count)
	{
		/* All existing containers seem to assume trivial relocatability (i.e. memcpy'able) of their members,
		* so we're going to assume that this is safe here.  However, it's not generally possible to assume this
		* in general as objects which contain pointers/references to themselves are not safe to be trivially
		* relocated.
		*
		* However, it is not yet possible to automatically infer this at compile time, so we can't enable
		* different (i.e. safer) implementations anyway. */

		Memory::Memmove(Dest, Source, sizeof(SourceElementType) * Count);
	}


	/**
	* Move constructs a range of items into memory.
	*
	* @param	Dest		The memory location to start moving into.
	* @param	Source		A pointer to the first item to move from.
	* @param	Count		The number of elements to move.
	*/
	template <typename ElementType>
	__forceinline typename EnableIf<TypeTraits<ElementType>::NeedsMoveConstructor>::Type MoveConstructItems(void* Dest, const ElementType* Source, int32 Count)
	{
		while (Count)
		{
			new (Dest) ElementType((ElementType&&)*Source);
			++(ElementType*&)Dest;
			++Source;
			--Count;
		}
	}

	template <typename ElementType>
	__forceinline typename EnableIf<!TypeTraits<ElementType>::NeedsMoveConstructor>::Type MoveConstructItems(void* Dest, const ElementType* Source, int32 Count)
	{
		Memory::Memmove(Dest, Source, sizeof(ElementType) * Count);
	}

	/**
	* Move assigns a range of items.
	*
	* @param	Dest		The memory location to start move assigning to.
	* @param	Source		A pointer to the first item to move assign.
	* @param	Count		The number of elements to move assign.
	*/
	template <typename ElementType>
	__forceinline typename EnableIf<TypeTraits<ElementType>::NeedsMoveAssignment>::Type MoveAssignItems(ElementType* Dest, const ElementType* Source, int32 Count)
	{
		while (Count)
		{
			*Dest = (ElementType&&)*Source;
			++Dest;
			++Source;
			--Count;
		}
	}

	template <typename ElementType>
	__forceinline typename EnableIf<!TypeTraits<ElementType>::NeedsMoveAssignment>::Type MoveAssignItems(ElementType* Dest, const ElementType* Source, int32 Count)
	{
		Memory::Memmove(Dest, Source, sizeof(ElementType) * Count);
	}

	template <typename ElementType>
	__forceinline typename EnableIf<TypeTraits<ElementType>::IsBytewiseComparable, bool>::Type CompareItems(const ElementType* A, const ElementType* B, int32 Count)
	{
		return !Memory::Memcmp(A, B, sizeof(ElementType) * Count);
	}


	template <typename ElementType>
	__forceinline typename EnableIf<!TypeTraits<ElementType>::IsBytewiseComparable, bool>::Type CompareItems(const ElementType* A, const ElementType* B, int32 Count)
	{
		while (Count)
		{
			if (!(*A == *B))
			{
				return false;
			}

			++A;
			++B;
			--Count;
		}

		return true;
	}

	template<typename T>
	__forceinline void MoveByRelocate(T& A, T& B)
	{
		// Destruct the previous value of A.
		A.~T();

		// Relocate B into the 'hole' left by the destruction of A, leaving a hole in B instead.
		RelocateConstructItems<T>(&A, &B, 1);
	}
}