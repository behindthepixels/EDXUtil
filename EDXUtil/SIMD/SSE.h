#pragma once

#include <smmintrin.h>

#include "../EDXPrerequisites.h"
#include "../Math/Constants.h"


#if defined(WIN32)

#include <intrin.h>

using namespace EDX;

__forceinline uint64 __rdpmc(int i)
{
	return __readpmc(i);
}

__forceinline int __bsf(int v)
{
	unsigned long r = 0; _BitScanForward(&r, v); return r;
}

__forceinline int __bsr(int v)
{
	unsigned long r = 0; _BitScanReverse(&r, v); return r;
}

__forceinline int __btc(int v, int i)
{
	long r = v; _bittestandcomplement(&r, i); return r;
}

__forceinline int __bts(int v, int i)
{
	long r = v; _bittestandset(&r, i); return r;
}

__forceinline int __btr(int v, int i)
{
	long r = v; _bittestandreset(&r, i); return r;
}

#if defined(_WIN64)

__forceinline size_t __bsf(size_t v)
{
	size_t r = 0; _BitScanForward64((unsigned long*)&r, v); return r;
}

__forceinline size_t __bsr(size_t v)
{
	size_t r = 0; _BitScanReverse64((unsigned long*)&r, v); return r;
}

__forceinline size_t __btc(size_t v, size_t i)
{
	//size_t r = v; _bittestandcomplement64((__int64*)&r,i); return r;
	return v ^ (size_t(1) << i); // faster than using intrinsics, as intrinsic goes through memory
}

__forceinline size_t __bts(size_t v, size_t i)
{
	__int64 r = v; _bittestandset64(&r, i); return r;
}

__forceinline size_t __btr(size_t v, size_t i)
{
	__int64 r = v; _bittestandreset64(&r, i); return r;
}

typedef __int64 atomc_t;

__forceinline int64 atomc_add(volatile int64* m, const int64 v)
{
	return _InterlockedExchangeAdd64(m, v);
}

__forceinline int64 atomc_xchg(volatile int64 *p, int64 v)
{
	return _InterlockedExchange64((volatile long long *)p, v);
}

__forceinline int64 atomc_cmpxchg(volatile int64* m, const int64 v, const int64 c)
{
	return _InterlockedCompareExchange64(m, v, c);
}

#endif

#endif

// Inspried by the code in embree
#include "BoolSSE.h"
#include "IntSSE.h"
#include "FloatSSE.h"

#include "../Math/Vector.h"

namespace EDX
{
	typedef Vec<2, FloatSSE> Vec2f_SSE;
	typedef Vec<2, IntSSE> Vec2i_SSE;
	typedef Vec<2, BoolSSE> Vec2b_SSE;

	typedef Vec<3, FloatSSE> Vec3f_SSE;
	typedef Vec<3, IntSSE> Vec3i_SSE;
	typedef Vec<3, BoolSSE> Vec3b_SSE;

	typedef Vec<4, FloatSSE> Vec4f_SSE;
	typedef Vec<4, IntSSE> Vec4i_SSE;
	typedef Vec<4, BoolSSE> Vec4b_SSE;

	namespace Math
	{
		template <>
		inline bool IsNaN(const FloatSSE& num) { return false; }
	}
}
