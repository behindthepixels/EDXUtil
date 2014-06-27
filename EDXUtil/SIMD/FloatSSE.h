#pragma once

namespace EDX
{
	// 4-wide SSE float type
	class FloatSSE
	{
	public:
		typedef BoolSSE Mask;          // mask type for us
		typedef IntSSE Int ;			// int type
		enum  { size = 4 };         // number of SIMD elements
		union { __m128 m128; float v[4]; int i[4]; }; // data

	public:
		//----------------------------------------------------------------------------------------------
		// Constructors, Assignment & Cast Operators
		//----------------------------------------------------------------------------------------------
		__forceinline FloatSSE() 
			: m128(_mm_setzero_ps()) {}

		__forceinline FloatSSE(const FloatSSE& copyFrom)
			: m128(copyFrom.m128) {}

		__forceinline FloatSSE& operator = (const FloatSSE& copyFrom)
		{
			m128 = copyFrom.m128;
			return *this;
		}

		__forceinline FloatSSE(const __m128& val)
			: m128(val) {}

		__forceinline operator const __m128&(void) const { return m128; }
		__forceinline operator		 __m128&(void)		 { return m128; }

		__forceinline explicit FloatSSE(const float* const pfVal)
			: m128(_mm_loadu_ps(pfVal)) {}

		//__forceinline ssef(float rhs) : m128(_mm_set1_ps(rhs)) {}

		__forceinline FloatSSE (const float& fVal)
			: m128(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(_mm_load_ss(&fVal)), _mm_SHUFFLE(0, 0, 0, 0)))) {}

		__forceinline FloatSSE(float a, float b)
			: m128(_mm_set_ps(b, a, b, a)) {}

		__forceinline FloatSSE(float a, float b, float c, float d)
			: m128(_mm_set_ps(d, c, b, a)) {}

		__forceinline explicit FloatSSE(const __m128i rhs)
			: m128(_mm_cvtepi32_ps(rhs)) {}

		//----------------------------------------------------------------------------------------------
		// Constants
		//----------------------------------------------------------------------------------------------
		__forceinline FloatSSE(Math::Zero)		: m128(_mm_setzero_ps()) {}
		__forceinline FloatSSE(Math::One)		: m128(_mm_set_ps(1.0f,1.0f,1.0f,1.0f)) {}
		__forceinline FloatSSE(Math::PosInf)	: m128(_mm_set_ps(Math::EDX_INFINITY,Math::EDX_INFINITY,Math::EDX_INFINITY,Math::EDX_INFINITY)) {}
		__forceinline FloatSSE(Math::NegInf)	: m128(_mm_set_ps(Math::EDX_NEG_INFINITY,Math::EDX_NEG_INFINITY,Math::EDX_NEG_INFINITY,Math::EDX_NEG_INFINITY)) {}
		__forceinline FloatSSE(Math::Step)		: m128(_mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f)) {}
		__forceinline FloatSSE(Math::NaN)		: m128(_mm_set1_ps(Math::EDX_NAN)) {}

		//----------------------------------------------------------------------------------------------
		// Array Access
		//----------------------------------------------------------------------------------------------
		__forceinline const float& operator [] (const size_t i) const { assert(i < 4); return v[i]; }
		__forceinline		float& operator [] (const size_t i)		  { assert(i < 4); return v[i]; }

		//----------------------------------------------------------------------------------------------
		// Unary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const FloatSSE operator + () const { return *this; }
		__forceinline const FloatSSE operator - () const { return _mm_xor_ps(m128, _mm_castsi128_ps(_mm_set1_epi32(0x80000000))); }

		//----------------------------------------------------------------------------------------------
		// Binary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const FloatSSE operator + (const FloatSSE& rhs) const { return _mm_add_ps(m128, rhs.m128); }
		__forceinline const FloatSSE operator + (const float& rhs) const { return *this + FloatSSE(rhs); }
		__forceinline const FloatSSE operator - (const FloatSSE& rhs) const { return _mm_sub_ps(m128, rhs.m128); }
		__forceinline const FloatSSE operator - (const float& rhs) const { return *this - FloatSSE(rhs); }
		__forceinline const FloatSSE operator * (const FloatSSE& rhs) const { return _mm_mul_ps(m128, rhs.m128); }
		__forceinline const FloatSSE operator * (const float& rhs) const { return *this * FloatSSE(rhs); }
		__forceinline const FloatSSE operator / (const FloatSSE& rhs) const { return *this * _mm_rcp_ps(rhs); }
		__forceinline const FloatSSE operator / (const float& rhs) const { return *this * (1.0f / rhs); }
		__forceinline const FloatSSE operator ^ (const FloatSSE& rhs) const { return _mm_xor_ps(m128, rhs.m128); }
		__forceinline const FloatSSE operator ^ (const IntSSE& rhs) const { return _mm_xor_ps(m128, _mm_castsi128_ps(rhs.m128)); }

		//----------------------------------------------------------------------------------------------
		// Assignment Operators
		//----------------------------------------------------------------------------------------------
		__forceinline FloatSSE& operator += (const FloatSSE& rhs) { return *this = *this + rhs; }
		__forceinline FloatSSE& operator += (const float& rhs) { return *this = *this + rhs; }

		__forceinline FloatSSE& operator -= (const FloatSSE& rhs) { return *this = *this - rhs; }
		__forceinline FloatSSE& operator -= (const float& rhs) { return *this = *this - rhs; }

		__forceinline FloatSSE& operator *= (const FloatSSE& rhs) { return *this = *this * rhs; }
		__forceinline FloatSSE& operator *= (const float& rhs) { return *this = *this * rhs; }

		__forceinline FloatSSE& operator /= (const FloatSSE& rhs) { return *this = *this / rhs; }
		__forceinline FloatSSE& operator /= (const float& rhs) { return *this = *this / rhs; }

		//----------------------------------------------------------------------------------------------
		// Comparison Operators + Select
		//----------------------------------------------------------------------------------------------
		__forceinline const BoolSSE operator == (const FloatSSE& rhs) const { return _mm_cmpeq_ps (m128, rhs.m128); }
		__forceinline const BoolSSE operator == (const float& rhs) const { return *this == FloatSSE(rhs); }
		__forceinline const BoolSSE operator != (const FloatSSE& rhs) const { return _mm_cmpneq_ps(m128, rhs.m128); }
		__forceinline const BoolSSE operator != (const float& rhs) const { return *this != FloatSSE(rhs); }
		__forceinline const BoolSSE operator < (const FloatSSE& rhs) const { return _mm_cmplt_ps (m128, rhs.m128); }
		__forceinline const BoolSSE operator < (const float& rhs) const { return *this < FloatSSE(rhs); }
		__forceinline const BoolSSE operator >= (const FloatSSE& rhs) const { return _mm_cmpnlt_ps(m128, rhs.m128); }
		__forceinline const BoolSSE operator > (const FloatSSE& rhs) const { return _mm_cmpnle_ps(m128, rhs.m128); }
		__forceinline const BoolSSE operator > (const float& rhs) const { return *this > FloatSSE(rhs); }
		__forceinline const BoolSSE operator >= (const float& rhs) const { return *this >= FloatSSE(rhs); }
		__forceinline const BoolSSE operator <= (const FloatSSE& rhs) const { return _mm_cmple_ps (m128, rhs.m128); }
		__forceinline const BoolSSE operator <= (const float& rhs) const { return *this <= FloatSSE(rhs); }

	};

	//----------------------------------------------------------------------------------------------
	// Comparison Operators
	//----------------------------------------------------------------------------------------------
	__forceinline const BoolSSE operator == (const float& lhs, const FloatSSE& rhs) { return FloatSSE(lhs) == rhs; }
	__forceinline const BoolSSE operator != (const float& lhs, const FloatSSE& rhs) { return FloatSSE(lhs) != rhs; }
	__forceinline const BoolSSE operator < (const float& lhs, const FloatSSE& rhs) { return FloatSSE(lhs) < rhs; }
	__forceinline const BoolSSE operator >= (const float& lhs, const FloatSSE& rhs) { return FloatSSE(lhs) >= rhs; }
	__forceinline const BoolSSE operator > (const float& lhs, const FloatSSE& rhs) { return FloatSSE(lhs) > rhs; }
	__forceinline const BoolSSE operator <= (const float& lhs, const FloatSSE& rhs) { return FloatSSE(lhs) <= rhs; }

	namespace SSE
	{
		__forceinline const FloatSSE Abs(const FloatSSE& rhs) { return _mm_and_ps(rhs.m128, _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff))); }
		__forceinline const FloatSSE Sign(const FloatSSE& rhs) { return _mm_blendv_ps(FloatSSE(Math::EDX_ONE), -FloatSSE(Math::EDX_ONE), _mm_cmplt_ps (rhs,FloatSSE(Math::EDX_ZERO))); }
		__forceinline const FloatSSE SignMask(const FloatSSE& rhs) { return _mm_and_ps(rhs.m128,_mm_castsi128_ps(_mm_set1_epi32(0x80000000))); }

		__forceinline const FloatSSE Rcp(const FloatSSE& rhs)
		{
			const FloatSSE r = _mm_rcp_ps(rhs.m128);
			return _mm_sub_ps(_mm_add_ps(r, r), _mm_mul_ps(_mm_mul_ps(r, r), rhs));
		}
		__forceinline const FloatSSE Sqr(const FloatSSE& rhs) { return _mm_mul_ps(rhs,rhs); }
		__forceinline const FloatSSE Sqrt(const FloatSSE& rhs) { return _mm_sqrt_ps(rhs.m128); }
		__forceinline const FloatSSE Rsqrt(const FloatSSE& rhs)
		{
			const FloatSSE r = _mm_rsqrt_ps(rhs.m128);
			return _mm_add_ps(_mm_mul_ps(_mm_set_ps(1.5f, 1.5f, 1.5f, 1.5f), r),
				_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(rhs, _mm_set_ps(-0.5f, -0.5f, -0.5f, -0.5f)), r), _mm_mul_ps(r, r)));}


		//----------------------------------------------------------------------------------------------
		// Binary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const FloatSSE operator + (const float& lhs, const FloatSSE& rhs) { return FloatSSE(lhs) + rhs; }
		__forceinline const FloatSSE operator - (const float& lhs, const FloatSSE& rhs) { return FloatSSE(lhs) - rhs; }
		__forceinline const FloatSSE operator * (const float& lhs, const FloatSSE& rhs) { return FloatSSE(lhs) * rhs; }
		__forceinline const FloatSSE operator / (const float& lhs, const FloatSSE& rhs) { return lhs * Rcp(rhs); }

		__forceinline const FloatSSE mn(const FloatSSE& lhs, const FloatSSE& rhs) { return _mm_mn_ps(lhs.m128, rhs.m128); }
		__forceinline const FloatSSE mn(const FloatSSE& lhs, const float& rhs) { return _mm_mn_ps(lhs.m128, FloatSSE(rhs)); }
		__forceinline const FloatSSE mn(const float& lhs, const FloatSSE& rhs) { return _mm_mn_ps(FloatSSE(lhs), rhs.m128); }

		__forceinline const FloatSSE Max(const FloatSSE& lhs, const FloatSSE& rhs) { return _mm_max_ps(lhs.m128, rhs.m128); }
		__forceinline const FloatSSE Max(const FloatSSE& lhs, const float& rhs) { return _mm_max_ps(lhs.m128, FloatSSE(rhs)); }
		__forceinline const FloatSSE Max(const float& lhs, const FloatSSE& rhs) { return _mm_max_ps(FloatSSE(lhs), rhs.m128); }


		//----------------------------------------------------------------------------------------------
		// Select
		//----------------------------------------------------------------------------------------------
		__forceinline const FloatSSE Select(const BoolSSE& mask, const FloatSSE& t, const FloatSSE& f)
		{ 
			return _mm_blendv_ps(f, t, mask); 
		}

		//----------------------------------------------------------------------------------------------
		// Rounding Functions
		//----------------------------------------------------------------------------------------------
		__forceinline const FloatSSE RoundEven(const FloatSSE& lhs) { return _mm_round_ps(lhs, _mm_FROUND_TO_NEAREST_INT); }
		__forceinline const FloatSSE RoundDown(const FloatSSE& lhs) { return _mm_round_ps(lhs, _mm_FROUND_TO_NEG_INF  ); }
		__forceinline const FloatSSE RoundUp(const FloatSSE& lhs) { return _mm_round_ps(lhs, _mm_FROUND_TO_POS_INF  ); }
		__forceinline const FloatSSE RoundZero(const FloatSSE& lhs) { return _mm_round_ps(lhs, _mm_FROUND_TO_ZERO   ); }
		__forceinline const FloatSSE Floor(const FloatSSE& lhs) { return _mm_round_ps(lhs, _mm_FROUND_TO_NEG_INF  ); }
		__forceinline const FloatSSE Ceil(const FloatSSE& lhs) { return _mm_round_ps(lhs, _mm_FROUND_TO_POS_INF  ); }

		//----------------------------------------------------------------------------------------------
		// Movement/Shifting/Shuffling Functions
		//----------------------------------------------------------------------------------------------
		__forceinline FloatSSE UnpackLow(const FloatSSE& lhs, const FloatSSE& rhs) { return _mm_unpacklo_ps(lhs.m128, rhs.m128); }
		__forceinline FloatSSE UnpackHigh(const FloatSSE& lhs, const FloatSSE& rhs) { return _mm_unpackhi_ps(lhs.m128, rhs.m128); }

		template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const FloatSSE Shuffle(const FloatSSE& rhs)
		{
			return _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(rhs), _mm_SHUFFLE(i3, i2, i1, i0)));
		}

		template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const FloatSSE Shuffle(const FloatSSE& lhs, const FloatSSE& rhs)
		{
			return _mm_shuffle_ps(lhs, rhs, _mm_SHUFFLE(i3, i2, i1, i0));
		}

		__forceinline const FloatSSE Shuffle8(const FloatSSE& lhs, const IntSSE& shuf)
		{ 
			return _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castps_si128(lhs), shuf)); 
		}

		template<> __forceinline const FloatSSE Shuffle<0, 0, 2, 2>(const FloatSSE& rhs) { return _mm_moveldup_ps(rhs); }
		template<> __forceinline const FloatSSE Shuffle<1, 1, 3, 3>(const FloatSSE& rhs) { return _mm_movehdup_ps(rhs); }
		template<> __forceinline const FloatSSE Shuffle<0, 1, 0, 1>(const FloatSSE& rhs) { return _mm_castpd_ps(_mm_movedup_pd(_mm_castps_pd(rhs))); }

		template<size_t i>
		__forceinline float Extract(const FloatSSE& lhs) { return _mm_cvtss_f32(Shuffle<i,i,i,i>(lhs)); }
		template<size_t dst, size_t src, size_t clr>
		__forceinline const FloatSSE Insert(const FloatSSE& lhs, const FloatSSE& rhs) { return _mm_insert_ps(lhs, rhs, (dst << 4) | (src << 6) | clr); }
		template<size_t dst, size_t src>
		__forceinline const FloatSSE Insert(const FloatSSE& lhs, const FloatSSE& rhs) { return insert<dst, src, 0>(lhs, rhs); }
		template<size_t dst>
		__forceinline const FloatSSE Insert(const FloatSSE& lhs, const float rhs) { return insert<dst, 0>(lhs, _mm_set_ss(rhs)); }

		//----------------------------------------------------------------------------------------------
		// Transpose
		//----------------------------------------------------------------------------------------------
		__forceinline void Transpose(const FloatSSE& r0, const FloatSSE& r1, const FloatSSE& r2, const FloatSSE& r3, FloatSSE& c0, FloatSSE& c1, FloatSSE& c2, FloatSSE& c3)
		{
			FloatSSE l02 = UnpackLow(r0,r2);
			FloatSSE h02 = UnpackHigh(r0,r2);
			FloatSSE l13 = UnpackLow(r1,r3);
			FloatSSE h13 = UnpackHigh(r1,r3);
			c0 = UnpackLow(l02,l13);
			c1 = UnpackHigh(l02,l13);
			c2 = UnpackLow(h02,h13);
			c3 = UnpackHigh(h02,h13);
		}

		__forceinline void Transpose(const FloatSSE& r0, const FloatSSE& r1, const FloatSSE& r2, const FloatSSE& r3, FloatSSE& c0, FloatSSE& c1, FloatSSE& c2)
		{
			FloatSSE l02 = UnpackLow(r0,r2);
			FloatSSE h02 = UnpackHigh(r0,r2);
			FloatSSE l13 = UnpackLow(r1,r3);
			FloatSSE h13 = UnpackHigh(r1,r3);
			c0 = UnpackLow(l02,l13);
			c1 = UnpackHigh(l02,l13);
			c2 = UnpackLow(h02,h13);
		}

		//----------------------------------------------------------------------------------------------
		// Reductions
		//----------------------------------------------------------------------------------------------
		__forceinline const FloatSSE VReducemn(const FloatSSE& v) { FloatSSE h = mn(Shuffle<1,0,3,2>(v),v); return mn(Shuffle<2,3,0,1>(h),h); }
		__forceinline const FloatSSE VReduceMax(const FloatSSE& v) { FloatSSE h = Max(Shuffle<1,0,3,2>(v),v); return Max(Shuffle<2,3,0,1>(h),h); }
		__forceinline const FloatSSE vReduceAdd(const FloatSSE& v) { FloatSSE h = Shuffle<1,0,3,2>(v) + v ; return Shuffle<2,3,0,1>(h) + h ; }

		__forceinline float Reducemn(const FloatSSE& v) { return _mm_cvtss_f32(VReducemn(v)); }
		__forceinline float ReduceMax(const FloatSSE& v) { return _mm_cvtss_f32(VReduceMax(v)); }
		__forceinline float ReduceAdd(const FloatSSE& v) { return _mm_cvtss_f32(vReduceAdd(v)); }

		__forceinline size_t Selectmn(const FloatSSE& v) { return __bsf(_mm_movemask_ps(v == VReducemn(v))); }
		__forceinline size_t SelectMax(const FloatSSE& v) { return __bsf(_mm_movemask_ps(v == VReduceMax(v))); }

		__forceinline size_t Selectmn(const BoolSSE& valid, const FloatSSE& v) { const FloatSSE tmp = Select(valid, v, FloatSSE(Math::EDX_INFINITY)); return __bsf(_mm_movemask_ps(valid & (tmp == VReducemn(tmp)))); }
		__forceinline size_t SelectMax(const BoolSSE& valid, const FloatSSE& v) { const FloatSSE tmp = Select(valid, v, FloatSSE(Math::EDX_NEG_INFINITY)); return __bsf(_mm_movemask_ps(valid & (tmp == VReduceMax(tmp)))); }

	}

	//----------------------------------------------------------------------------------------------
	// Output Operators
	//----------------------------------------------------------------------------------------------
	inline std::ostream& operator << (std::ostream& out, const FloatSSE& rhs)
	{
		return out << "<" << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << ", " << rhs[3] << ">";
	}
}