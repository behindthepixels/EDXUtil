#pragma once

#include "../Core/Types.h"
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
}