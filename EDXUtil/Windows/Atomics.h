#pragma once

#include "Base.h"
#include "../Core/Template.h"

#define PLATFORM_HAS_64BIT_ATOMICS 1

namespace EDX
{
	/**
	* Windows implementation of the Atomics OS functions
	*/
	class WindowsAtomics
	{
	public:
		static __forceinline int32 InterlockedIncrement(volatile int32* Value)
		{
			return (int32)::InterlockedIncrement((LPLONG)Value);
		}

#if PLATFORM_HAS_64BIT_ATOMICS
		static __forceinline int64 InterlockedIncrement(volatile int64* Value)
		{
			return (int64)::InterlockedIncrement64((LONGLONG*)Value);
		}
#endif

		static __forceinline int32 InterlockedDecrement(volatile int32* Value)
		{
			return (int32)::InterlockedDecrement((LPLONG)Value);
		}

#if PLATFORM_HAS_64BIT_ATOMICS
		static __forceinline int64 InterlockedDecrement(volatile int64* Value)
		{
			return (int64)::InterlockedDecrement64((LONGLONG*)Value);
		}
#endif

		static __forceinline int32 InterlockedAdd(volatile int32* Value, int32 Amount)
		{
			return (int32)::InterlockedExchangeAdd((LPLONG)Value, (LONG)Amount);
		}

#if PLATFORM_HAS_64BIT_ATOMICS
		static __forceinline int64 InterlockedAdd(volatile int64* Value, int64 Amount)
		{
			return (int64)::InterlockedExchangeAdd64((LONGLONG*)Value, (LONGLONG)Amount);
		}
#endif

		static __forceinline int32 InterlockedExchange(volatile int32* Value, int32 Exchange)
		{
			return (int32)::InterlockedExchange((LPLONG)Value, (LONG)Exchange);
		}

#if PLATFORM_HAS_64BIT_ATOMICS
		static __forceinline int64 InterlockedExchange(volatile int64* Value, int64 Exchange)
		{
			return (int64)::InterlockedExchange64((LONGLONG*)Value, (LONGLONG)Exchange);
		}
#endif

		static __forceinline void* InterlockedExchangePtr(void** Dest, void* Exchange)
		{
#ifdef _DEBUG
			if (IsAligned(Dest, sizeof(void*)) == false)
			{
				HandleAtomicsFailure(EDX_TEXT("InterlockedExchangePointer requires Dest pointer to be aligned to %d bytes"), sizeof(void*));
			}
#endif

#if PLATFORM_HAS_64BIT_ATOMICS
			return ::InterlockedExchangePointer(Dest, Exchange);
#else
			return (void*)::InterlockedExchange((PLONG)(Dest), (LONG)(Exchange));
#endif
		}

		static __forceinline int32 InterlockedCompareExchange(volatile int32* Dest, int32 Exchange, int32 Comparand)
		{
			return (int32)::InterlockedCompareExchange((LPLONG)Dest, (LONG)Exchange, (LONG)Comparand);
		}

#if PLATFORM_HAS_64BIT_ATOMICS
		static __forceinline int64 InterlockedCompareExchange(volatile int64* Dest, int64 Exchange, int64 Comparand)
		{
#ifdef _DEBUG
			if (IsAligned(Dest, sizeof(void*)) == false)
			{
				HandleAtomicsFailure(EDX_TEXT("InterlockedCompareExchangePointer requires Dest pointer to be aligned to %d bytes"), sizeof(void*));
			}
#endif

			return (int64)::InterlockedCompareExchange64((LONGLONG*)Dest, (LONGLONG)Exchange, (LONGLONG)Comparand);
		}
#endif

		/**
		*	The function compares the Destination value with the Comparand value:
		*		- If the Destination value is equal to the Comparand value, the Exchange value is stored in the address specified by Destination,
		*		- Otherwise, the initial value of the Destination parameter is stored in the address specified specified by Comparand.
		*
		*	@return true if Comparand equals the original value of the Destination parameter, or false if Comparand does not equal the original value of the Destination parameter.
		*
		*	Early AMD64 processors lacked the CMPXCHG16B instruction.
		*	To determine whether the processor supports this operation, call the IsProcessorFeaturePresent function with PF_COMPARE64_EXCHANGE128.
		*/
#if	PLATFORM_HAS_128BIT_ATOMICS
		static __forceinline bool InterlockedCompareExchange128(volatile FInt128* Dest, const FInt128& Exchange, FInt128* Comparand)
		{
#if _DEBUG
			if (IsAligned(Dest, 16) == false)
			{
				HandleAtomicsFailure(EDX_TEXT("InterlockedCompareExchangePointer requires Dest pointer to be aligned to 16 bytes"));
			}
			if (IsAligned(Comparand, 16) == false)
			{
				HandleAtomicsFailure(EDX_TEXT("InterlockedCompareExchangePointer requires Comparand pointer to be aligned to 16 bytes"));
			}
#endif

			return ::InterlockedCompareExchange128((int64 volatile *)Dest, Exchange.High, Exchange.Low, (int64*)Comparand) == 1;
		}
#endif // PLATFORM_HAS_128BIT_ATOMICS

		static __forceinline void* InterlockedCompareExchangePointer(void** Dest, void* Exchange, void* Comparand)
		{
#if _DEBUG
			if (IsAligned(Dest, sizeof(void*)) == false)
			{
				HandleAtomicsFailure(EDX_TEXT("InterlockedCompareExchangePointer requires Dest pointer to be aligned to %d bytes"), sizeof(void*));
			}
#endif

			return ::InterlockedCompareExchangePointer(Dest, Exchange, Comparand);
		}

		/**
		* @return true, if the processor we are running on can execute compare and exchange 128-bit operation.
		* @see cmpxchg16b, early AMD64 processors don't support this operation.
		*/
		static __forceinline bool CanUseCompareExchange128()
		{
			return !!IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE128);
		}

	protected:
		/**
		* Handles atomics function failure.
		*
		* Since 'check' has not yet been declared here we need to call external function to use it.
		*
		* @param InFormat - The string format string.
		*/
		static void HandleAtomicsFailure(const TCHAR* InFormat, ...)
		{
			Assertf(false, InFormat);
		}
	};

}