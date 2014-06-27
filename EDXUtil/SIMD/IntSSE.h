#pragma once

namespace EDX
{
	// 4-wide SSE integer type.
	class IntSSE
	{
	public:
		typedef BoolSSE Mask;          // mask type for us
		enum  { size = 4 };         // number of SIMD elements
		union { __m128i m128; int32 v[4]; }; // data

	public:
		//----------------------------------------------------------------------------------------------
		// Constructors, Assignment & Cast Operators
		//----------------------------------------------------------------------------------------------
		__forceinline IntSSE()
			: m128(_mm_setzero_si128()) {}

		__forceinline IntSSE(const IntSSE& copyFrom)
			: m128(copyFrom.m128) {}

		__forceinline IntSSE& operator = (const IntSSE& copyFrom)
		{
			m128 = copyFrom.m128;
			return *this;
		}

		__forceinline IntSSE(const __m128i& val)
			: m128(val) {}

		__forceinline operator const __m128i&(void) const { return m128; }
		__forceinline operator		 __m128i&(void)		  { return m128; }

		__forceinline explicit IntSSE (const int32* const piVal)
			: m128(_mm_loadu_si128((__m128i*)piVal)) {}

		//__forceinline ssei(int32 a) : m128(_mm_set1_epi32(a)) {}

		__forceinline IntSSE(const int32& a)
			: m128(_mm_shuffle_epi32(_mm_castps_si128(_mm_load_ss((float*)&a)), _mm_SHUFFLE(0, 0, 0, 0))) {}

		__forceinline IntSSE(int32 a, int32 b)
			: m128(_mm_set_epi32(b, a, b, a)) {}

		__forceinline IntSSE(int32 a, int32 b, int32 c, int32 d)
			: m128(_mm_set_epi32(d, c, b, a)) {}

		__forceinline explicit IntSSE(const __m128& val)
			: m128(_mm_cvtps_epi32(val)) {}

		//----------------------------------------------------------------------------------------------
		// Constants
		//----------------------------------------------------------------------------------------------
		__forceinline IntSSE(Math::Zero)		: m128(_mm_setzero_si128()) {}
		__forceinline IntSSE(Math::One)		: m128(_mm_set_epi32(1, 1, 1, 1)) {}
		__forceinline IntSSE(Math::PosInf)		: m128(_mm_set_epi32(Math::EDX_INFINITY,Math::EDX_INFINITY,Math::EDX_INFINITY,Math::EDX_INFINITY)) {}
		__forceinline IntSSE(Math::NegInf)		: m128(_mm_set_epi32(Math::EDX_NEG_INFINITY,Math::EDX_NEG_INFINITY,Math::EDX_NEG_INFINITY,Math::EDX_NEG_INFINITY)) {}
		__forceinline IntSSE(Math::Step)		: m128(_mm_set_epi32(3, 2, 1, 0)) {}

		//----------------------------------------------------------------------------------------------
		// Array Access
		//----------------------------------------------------------------------------------------------
		__forceinline const int32& operator [] (const size_t i) const { assert(i < 4); return v[i]; }
		__forceinline		int32& operator [] (const size_t i)		  { assert(i < 4); return v[i]; }

		//----------------------------------------------------------------------------------------------
		// Unary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const IntSSE operator + () const { return *this; }
		__forceinline const IntSSE operator - () const { return _mm_sub_epi32(_mm_setzero_si128(), m128); }

		//----------------------------------------------------------------------------------------------
		// Binary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const IntSSE operator + (const IntSSE& rhs) const { return _mm_add_epi32(m128, rhs.m128); }
		__forceinline const IntSSE operator + (const int32& rhs) const { return *this + IntSSE(rhs); }
		__forceinline const IntSSE operator - (const IntSSE& rhs) const { return _mm_sub_epi32(m128, rhs.m128); }
		__forceinline const IntSSE operator - (const int32& rhs) const { return *this - IntSSE(rhs); }
		__forceinline const IntSSE operator * (const IntSSE& rhs) const { return _mm_mullo_epi32(m128, rhs.m128); }
		__forceinline const IntSSE operator * (const int32& rhs) const { return *this * IntSSE(rhs); }
		__forceinline const IntSSE operator & (const IntSSE& rhs) const { return _mm_and_si128(m128, rhs.m128); }
		__forceinline const IntSSE operator & (const int32& rhs) const { return *this & IntSSE(rhs); }
		__forceinline const IntSSE operator | (const IntSSE& rhs) const { return _mm_or_si128(m128, rhs.m128); }
		__forceinline const IntSSE operator | (const int32& rhs) const { return *this | IntSSE(rhs); }
		__forceinline const IntSSE operator ^ (const IntSSE& rhs) const { return _mm_xor_si128(m128, rhs.m128); }
		__forceinline const IntSSE operator ^ (const int32& rhs) const { return *this ^ IntSSE(rhs); }
		__forceinline const IntSSE operator << (const int32& n) const { return _mm_slli_epi32(m128, n); }
		__forceinline const IntSSE operator >> (const int32& n) const { return _mm_srai_epi32(m128, n); }

		//----------------------------------------------------------------------------------------------
		// Assignment Operators
		//----------------------------------------------------------------------------------------------
		__forceinline IntSSE& operator += (const IntSSE& rhs) { return *this = *this + rhs; }
		__forceinline IntSSE& operator += (const int32& rhs) { return *this = *this + rhs; }

		__forceinline IntSSE& operator -= (const IntSSE& rhs) { return *this = *this - rhs; }
		__forceinline IntSSE& operator -= (const int32& rhs) { return *this = *this - rhs; }

		__forceinline IntSSE& operator *= (const IntSSE& rhs) { return *this = *this * rhs; }
		__forceinline IntSSE& operator *= (const int32& rhs) { return *this = *this * rhs; }

		__forceinline IntSSE& operator &= (const IntSSE& rhs) { return *this = *this & rhs; }
		__forceinline IntSSE& operator &= (const int32& rhs) { return *this = *this & rhs; }

		__forceinline IntSSE& operator |= (const IntSSE& rhs) { return *this = *this | rhs; }
		__forceinline IntSSE& operator |= (const int32& rhs) { return *this = *this | rhs; }

		__forceinline IntSSE& operator <<= (const int32& rhs) { return *this = *this << rhs; }
		__forceinline IntSSE& operator >>= (const int32& rhs) { return *this = *this >> rhs; }

		//----------------------------------------------------------------------------------------------
		// Comparison Operators + Select
		//----------------------------------------------------------------------------------------------
		__forceinline const BoolSSE operator == (const IntSSE& rhs) const { return _mm_castsi128_ps(_mm_cmpeq_epi32 (m128, rhs.m128)); }
		__forceinline const BoolSSE operator == (const int32& rhs) const { return *this == IntSSE(rhs); }

		__forceinline const BoolSSE operator != (const IntSSE& rhs) const { return !(*this == rhs); }
		__forceinline const BoolSSE operator != (const int32& rhs) const { return *this != IntSSE(rhs); }

		__forceinline const BoolSSE operator < (const IntSSE& rhs) const { return _mm_castsi128_ps(_mm_cmplt_epi32 (m128, rhs.m128)); }
		__forceinline const BoolSSE operator < (const int32& rhs) const { return *this < IntSSE(rhs); }

		__forceinline const BoolSSE operator >= (const IntSSE& rhs) const { return !(*this < rhs); }
		__forceinline const BoolSSE operator >= (const int32& rhs) const { return *this >= IntSSE(rhs); }

		__forceinline const BoolSSE operator > (const IntSSE& rhs) const { return _mm_castsi128_ps(_mm_cmpgt_epi32 (m128, rhs.m128)); }
		__forceinline const BoolSSE operator > (const int32& rhs) const { return *this > IntSSE(rhs); }

		__forceinline const BoolSSE operator <= (const IntSSE& rhs) const { return !(*this > rhs); }
		__forceinline const BoolSSE operator <= (const int32& rhs) const { return *this <= IntSSE(rhs); }
	};

	//----------------------------------------------------------------------------------------------
	// Binary Operators
	//----------------------------------------------------------------------------------------------
	__forceinline const IntSSE operator + (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) + rhs; }
	__forceinline const IntSSE operator - (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) - rhs; }
	__forceinline const IntSSE operator * (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) * rhs; }
	__forceinline const IntSSE operator & (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) & rhs; }
	__forceinline const IntSSE operator | (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) | rhs; }
	__forceinline const IntSSE operator ^ (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) ^ rhs; }

	__forceinline const BoolSSE operator == (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) == rhs; }
	__forceinline const BoolSSE operator != (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) != rhs; }
	__forceinline const BoolSSE operator < (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) < rhs; }
	__forceinline const BoolSSE operator >= (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) >= rhs; }
	__forceinline const BoolSSE operator > (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) > rhs; }
	__forceinline const BoolSSE operator <= (const int32& lhs, const IntSSE& rhs) { return IntSSE(lhs) <= rhs; }

	namespace SSE
	{
		//----------------------------------------------------------------------------------------------
		// Comparison Operators + Select
		//----------------------------------------------------------------------------------------------
		__forceinline const IntSSE Abs(const IntSSE& lhs) { return _mm_abs_epi32(lhs.m128); }

		__forceinline const IntSSE srlhs (const IntSSE& lhs, const int32& rhs) { return _mm_srai_epi32(lhs.m128, rhs); }
		__forceinline const IntSSE srl (const IntSSE& lhs, const int32& rhs) { return _mm_srli_epi32(lhs.m128, rhs); }

		__forceinline const IntSSE mn(const IntSSE& lhs, const IntSSE& rhs) { return _mm_mn_epi32(lhs.m128, rhs.m128); }
		__forceinline const IntSSE mn(const IntSSE& lhs, const int32& rhs) { return mn(lhs, IntSSE(rhs)); }
		__forceinline const IntSSE mn(const int32& lhs, const IntSSE& rhs) { return mn(IntSSE(lhs), rhs); }

		__forceinline const IntSSE Max(const IntSSE& lhs, const IntSSE& rhs) { return _mm_max_epi32(lhs.m128, rhs.m128); }
		__forceinline const IntSSE Max(const IntSSE& lhs, const int32& rhs) { return Max(lhs, IntSSE(rhs)); }
		__forceinline const IntSSE Max(const int32& lhs, const IntSSE& rhs) { return Max(IntSSE(lhs), rhs); }


		__forceinline const IntSSE Select(const BoolSSE& m, const IntSSE& lhs, const IntSSE& rhs)
		{
			return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(rhs), _mm_castsi128_ps(lhs), m)); 
		}

		//----------------------------------------------------------------------------------------------
		// Movement/Shifting/Shuffling Functions
		//----------------------------------------------------------------------------------------------
		__forceinline IntSSE UnpackLow(const IntSSE& lhs, const IntSSE& rhs) { return _mm_castps_si128(_mm_unpacklo_ps(_mm_castsi128_ps(lhs.m128), _mm_castsi128_ps(rhs.m128))); }
		__forceinline IntSSE UnpackHigh(const IntSSE& lhs, const IntSSE& rhs) { return _mm_castps_si128(_mm_unpackhi_ps(_mm_castsi128_ps(lhs.m128), _mm_castsi128_ps(rhs.m128))); }

		template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const IntSSE Shuffle(const IntSSE& lhs)
		{
			return _mm_shuffle_epi32(lhs, _mm_SHUFFLE(i3, i2, i1, i0));
		}

		template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const IntSSE Shuffle(const IntSSE& lhs, const IntSSE& rhs)
		{
			return _mm_castps_si128(_mm_Shuffle_ps(_mm_castsi128_ps(lhs), _mm_castsi128_ps(rhs), _mm_SHUFFLE(i3, i2, i1, i0)));
		}

		template<> __forceinline const IntSSE Shuffle<0, 0, 2, 2>(const IntSSE& lhs) { return _mm_castps_si128(_mm_moveldup_ps(_mm_castsi128_ps(lhs))); }
		template<> __forceinline const IntSSE Shuffle<1, 1, 3, 3>(const IntSSE& lhs) { return _mm_castps_si128(_mm_movehdup_ps(_mm_castsi128_ps(lhs))); }
		template<> __forceinline const IntSSE Shuffle<0, 1, 0, 1>(const IntSSE& lhs) { return _mm_castpd_si128(_mm_movedup_pd (_mm_castsi128_pd(lhs))); }

		template<size_t src> __forceinline int Extract(const IntSSE& rhs) { return _mm_extract_epi32(rhs, src); }
		template<size_t dst> __forceinline const IntSSE Insert(const IntSSE& lhs, const int32 rhs) { return _mm_insert_epi32(lhs, rhs, dst); }

		//----------------------------------------------------------------------------------------------
		// Reductions
		//----------------------------------------------------------------------------------------------
		__forceinline const IntSSE VReducemn(const IntSSE& v) { IntSSE h = mn(Shuffle<1,0,3,2>(v),v); return mn(Shuffle<2,3,0,1>(h),h); }
		__forceinline const IntSSE VReduceMax(const IntSSE& v) { IntSSE h = Max(Shuffle<1,0,3,2>(v),v); return Max(Shuffle<2,3,0,1>(h),h); }
		__forceinline const IntSSE vReduceAdd(const IntSSE& v) { IntSSE h = Shuffle<1,0,3,2>(v) + v ; return Shuffle<2,3,0,1>(h) + h ; }

		__forceinline int Reducemn(const IntSSE& v) { return Extract<0>(VReducemn(v)); }
		__forceinline int ReduceMax(const IntSSE& v) { return Extract<0>(VReduceMax(v)); }
		__forceinline int ReduceAdd(const IntSSE& v) { return Extract<0>(vReduceAdd(v)); }

		__forceinline size_t Selectmn(const IntSSE& v) { return __bsf(_mm_movemask_ps(v == VReducemn(v))); }
		__forceinline size_t SelectMax(const IntSSE& v) { return __bsf(_mm_movemask_ps(v == VReduceMax(v))); }

		__forceinline size_t Selectmn(const BoolSSE& valid, const IntSSE& v) { const IntSSE tmp = Select(valid,v,IntSSE(Math::EDX_INFINITY)); return __bsf(_mm_movemask_ps(valid & (tmp == VReducemn(tmp)))); }
		__forceinline size_t SelectMax(const BoolSSE& valid, const IntSSE& v) { const IntSSE tmp = Select(valid,v,IntSSE(Math::EDX_NEG_INFINITY)); return __bsf(_mm_movemask_ps(valid & (tmp == VReduceMax(tmp)))); }

	}
	//----------------------------------------------------------------------------------------------
	// Output Operators
	//----------------------------------------------------------------------------------------------
	inline std::ostream& operator << (std::ostream& out, const IntSSE& rhs)
	{
		return out << "<" << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << ", " << rhs[3] << ">";
	}
}

