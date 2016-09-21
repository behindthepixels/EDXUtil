#pragma once

namespace EDX
{
	const __m128 _mm_lookupmask_ps[16] = {
		_mm_castsi128_ps(_mm_set_epi32(0, 0, 0, 0)),
		_mm_castsi128_ps(_mm_set_epi32(0, 0, 0,-1)),
		_mm_castsi128_ps(_mm_set_epi32(0, 0,-1, 0)),
		_mm_castsi128_ps(_mm_set_epi32(0, 0,-1,-1)),
		_mm_castsi128_ps(_mm_set_epi32(0,-1, 0, 0)),
		_mm_castsi128_ps(_mm_set_epi32(0,-1, 0,-1)),
		_mm_castsi128_ps(_mm_set_epi32(0,-1,-1, 0)),
		_mm_castsi128_ps(_mm_set_epi32(0,-1,-1,-1)),
		_mm_castsi128_ps(_mm_set_epi32(-1, 0, 0, 0)),
		_mm_castsi128_ps(_mm_set_epi32(-1, 0, 0,-1)),
		_mm_castsi128_ps(_mm_set_epi32(-1, 0,-1, 0)),
		_mm_castsi128_ps(_mm_set_epi32(-1, 0,-1,-1)),
		_mm_castsi128_ps(_mm_set_epi32(-1,-1, 0, 0)),
		_mm_castsi128_ps(_mm_set_epi32(-1,-1, 0,-1)),
		_mm_castsi128_ps(_mm_set_epi32(-1,-1,-1, 0)),
		_mm_castsi128_ps(_mm_set_epi32(-1,-1,-1,-1))
	};

	// 4-wide SSE bool type.
	class BoolSSE
	{
	public:
		typedef BoolSSE Mask;                    // mask type for us
		enum   { size = 4 };                  // number of SIMD elements
		union  { __m128 m128; int32 v[4]; };  // data

	public:
		//----------------------------------------------------------------------------------------------
		// Constructors, Assignment & Cast Operators
		//----------------------------------------------------------------------------------------------
		__forceinline BoolSSE() {}

		__forceinline BoolSSE(const BoolSSE& copyFrom)
			: m128(copyFrom.m128) {}

		__forceinline BoolSSE& operator = (const BoolSSE& copyFrom)
		{
			m128 = copyFrom.m128;
			return *this;
		}

		__forceinline BoolSSE(const __m128& val)
			: m128(val) {}

		__forceinline operator const __m128&(void) const { return m128; }
		__forceinline operator const __m128i(void) const { return _mm_castps_si128(m128); }
		__forceinline operator const __m128d(void) const { return _mm_castps_pd(m128); }

		__forceinline BoolSSE     (bool  a)
			: m128(_mm_lookupmask_ps[(size_t(a) << 3) | (size_t(a) << 2) | (size_t(a) << 1) | size_t(a)]) {}
		__forceinline BoolSSE     (bool  a, bool  b) 
			: m128(_mm_lookupmask_ps[(size_t(b) << 3) | (size_t(a) << 2) | (size_t(b) << 1) | size_t(a)]) {}
		__forceinline BoolSSE     (bool  a, bool  b, bool  c, bool  d)
			: m128(_mm_lookupmask_ps[(size_t(d) << 3) | (size_t(c) << 2) | (size_t(b) << 1) | size_t(a)]) {}

		//----------------------------------------------------------------------------------------------
		// Constants
		//----------------------------------------------------------------------------------------------
		__forceinline BoolSSE(Constants::False) : m128(_mm_setzero_ps()) {}
		__forceinline BoolSSE(Constants::True) : m128(_mm_castsi128_ps(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()))) {}

		//----------------------------------------------------------------------------------------------
		// Array Access
		//----------------------------------------------------------------------------------------------
		__forceinline bool operator [] (const size_t i) const { Assert(i < 4); return (_mm_movemask_ps(m128) >> i) & 1; }
		__forceinline int32& operator [] (const size_t i) { Assert(i < 4); return v[i]; }

		//----------------------------------------------------------------------------------------------
		// Unary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const BoolSSE operator ! () const { return _mm_xor_ps(*this, BoolSSE(Constants::EDX_TRUE)); }

		//----------------------------------------------------------------------------------------------
		// Binary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const BoolSSE operator & (const BoolSSE& rhs) const { return _mm_and_ps(*this, rhs); }
		__forceinline const BoolSSE operator | (const BoolSSE& rhs) const { return _mm_or_ps (*this, rhs); }
		__forceinline const BoolSSE operator ^ (const BoolSSE& rhs) const { return _mm_xor_ps(*this, rhs); }

		//----------------------------------------------------------------------------------------------
		// Assignment Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const BoolSSE operator &= (const BoolSSE& rhs) { return *this = *this & rhs; }
		__forceinline const BoolSSE operator |= (const BoolSSE& rhs) { return *this = *this | rhs; }
		__forceinline const BoolSSE operator ^= (const BoolSSE& rhs) { return *this = *this ^ rhs; }

		//----------------------------------------------------------------------------------------------
		// Comparison Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const BoolSSE operator != (const BoolSSE& rhs) const { return _mm_xor_ps(*this, rhs); }
		__forceinline const BoolSSE operator == (const BoolSSE& rhs) const { return _mm_castsi128_ps(_mm_cmpeq_epi32(*this, rhs)); }

	};

	namespace SSE
	{
		//----------------------------------------------------------------------------------------------
		// Select
		//----------------------------------------------------------------------------------------------
		__forceinline const BoolSSE Select(const BoolSSE& m, const BoolSSE& t, const BoolSSE& f)
		{
			return _mm_blendv_ps(f, t, m); 
		}

		//----------------------------------------------------------------------------------------------
		// Movement/Shifting/Shuffling Functions
		//----------------------------------------------------------------------------------------------

		__forceinline const BoolSSE UnpackLow(const BoolSSE& lhs, const BoolSSE& rhs) { return _mm_unpacklo_ps(lhs, rhs); }
		__forceinline const BoolSSE UnpackHigh(const BoolSSE& lhs, const BoolSSE& rhs) { return _mm_unpackhi_ps(lhs, rhs); }

		template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const BoolSSE Shuffle(const BoolSSE& lhs)
		{
			return _mm_shuffle_epi32(lhs, _MM_SHUFFLE(i3, i2, i1, i0));
		}

		template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const BoolSSE Shuffle(const BoolSSE& lhs, const BoolSSE& rhs)
		{
			return _MM_SHUFFLE_ps(lhs, rhs, _MM_SHUFFLE(i3, i2, i1, i0));
		}

		template<> __forceinline const BoolSSE Shuffle<0, 0, 2, 2>(const BoolSSE& lhs) { return _mm_moveldup_ps(lhs); }
		template<> __forceinline const BoolSSE Shuffle<1, 1, 3, 3>(const BoolSSE& lhs) { return _mm_movehdup_ps(lhs); }
		template<> __forceinline const BoolSSE Shuffle<0, 1, 0, 1>(const BoolSSE& lhs) { return _mm_castpd_ps(_mm_movedup_pd (lhs)); }

		template<size_t dst, size_t src, size_t clr> __forceinline const BoolSSE Insert(const BoolSSE& lhs, const BoolSSE& rhs) { return _mm_insert_ps(lhs, rhs, (dst << 4) | (src << 6) | clr); }
		template<size_t dst, size_t src> __forceinline const BoolSSE Insert(const BoolSSE& lhs, const BoolSSE& rhs) { return insert<dst, src, 0>(lhs, rhs); }
		template<size_t dst>             __forceinline const BoolSSE Insert(const BoolSSE& lhs, const bool rhs) { return insert<dst,0>(lhs, BoolSSE(rhs)); }

		//----------------------------------------------------------------------------------------------
		// Reduction Operations
		//----------------------------------------------------------------------------------------------
		__forceinline size_t popcnt(const BoolSSE& lhs) { return bool(lhs[0])+bool(lhs[1])+bool(lhs[2])+bool(lhs[3]); }

		__forceinline bool ReduceAnd(const BoolSSE& lhs) { return _mm_movemask_ps(lhs) == 0xf; }
		__forceinline bool ReduceOr(const BoolSSE& lhs) { return _mm_movemask_ps(lhs) != 0x0; }
		__forceinline bool All(const BoolSSE& rhs) { return _mm_movemask_ps(rhs) == 0xf; }
		__forceinline bool Any(const BoolSSE& rhs) { return _mm_movemask_ps(rhs) != 0x0; }
		__forceinline bool None(const BoolSSE& rhs) { return _mm_movemask_ps(rhs) == 0x0; }

	}
	//----------------------------------------------------------------------------------------------
	// Output Operators
	//----------------------------------------------------------------------------------------------
	inline std::ostream& operator << (std::ostream& out, const BoolSSE& rhs)
	{
		return out << "<" << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << ", " << rhs[3] << ">";
	}
}
