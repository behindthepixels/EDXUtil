#pragma once

#pragma warning(disable: 4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable: 4267) // conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable: 4018) // signed/unsigned mismatch
#pragma warning(disable: 4800) // forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4838) // conversion from 'const int' to 'float' requires a narrowing conversion
//
//#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#endif

// C++ support
#include <cstdlib>
#include <crtdbg.h>
#include <new>
#include <malloc.h>
#include <cstdio>
#include <cmath>
#include <cfloat>


#define USE_UNICODE 0

#if USE_UNICODE
	#define EDX_TEXT(x) L##x
#else
	#define EDX_TEXT(x) x
#endif


namespace EDX
{
	typedef unsigned int	 uint;
	typedef unsigned char	 _byte;
	typedef unsigned long	 dword;

	// Unsigned base types.
	typedef unsigned char 		uint8;		// 8-bit  unsigned.
	typedef unsigned short int	uint16;		// 16-bit unsigned.
	typedef unsigned int		uint32;		// 32-bit unsigned.
	typedef unsigned long long	uint64;		// 64-bit unsigned.

											// Signed base types.
	typedef	signed char			int8;		// 8-bit  signed.
	typedef signed short int	int16;		// 16-bit signed.
	typedef signed int	 		int32;		// 32-bit signed.
	typedef signed long long	int64;		// 64-bit signed.

											// Character types.
	typedef char				ANSICHAR;	// An ANSI character       -                  8-bit fixed-width representation of 7-bit characters.
	typedef wchar_t				WIDECHAR;	// A wide character        - In-memory only.  ?-bit fixed-width representation of the platform's natural wide character set.  Could be different sizes on different platforms.
	typedef uint8				CHAR8;		// An 8-bit character type - In-memory only.  8-bit representation.  Should really be char8_t but making this the generic option is easier for compilers which don't fully support C++11 yet (i.e. MSVC).
	typedef uint16				CHAR16;		// A 16-bit character type - In-memory only.  16-bit representation.  Should really be char16_t but making this the generic option is easier for compilers which don't fully support C++11 yet (i.e. MSVC).
	typedef uint32				CHAR32;		// A 32-bit character type - In-memory only.  32-bit representation.  Should really be char32_t but making this the generic option is easier for compilers which don't fully support C++11 yet (i.e. MSVC).'

#if USE_UNICODE
	typedef WIDECHAR			TCHAR;		// A switchable character  - In-memory only.  Either ANSICHAR or WIDECHAR, depending on a licensee's requirements.
#else
	typedef ANSICHAR			TCHAR;		// A switchable character  - In-memory only.  Either ANSICHAR or WIDECHAR, depending on a licensee's requirements.
#endif

	template<typename T32BITS, typename T64BITS, int PointerSize>
	struct SelectIntPointerType
	{
		// nothing here are is it an error if the partial specializations fail
	};

	template<typename T32BITS, typename T64BITS>
	struct SelectIntPointerType<T32BITS, T64BITS, 8>
	{
		typedef T64BITS TIntPointer; // select the 64 bit type
	};

	template<typename T32BITS, typename T64BITS>
	struct SelectIntPointerType<T32BITS, T64BITS, 4>
	{
		typedef T32BITS TIntPointer; // select the 64 bit type
	};

	typedef SelectIntPointerType<uint32, uint64, sizeof(void*)>::TIntPointer UPTRINT;	// unsigned int the same size as a pointer
	typedef SelectIntPointerType<int32, int64, sizeof(void*)>::TIntPointer PTRINT;		// signed int the same size as a pointer

	typedef size_t SIZE_T;																// unsigned int the same size as a pointer
	typedef PTRINT SSIZE_T;																// unsigned int the same size as a pointer

	typedef int32					TYPE_OF_NULL;
	typedef decltype(nullptr)		TYPE_OF_NULLPTR;

	enum { INDEX_NONE = -1 };
	enum { UNICODE_BOM = 0xfeff };
}