#pragma once

#include "../EDXPrerequisites.h"
#include "Constants.h"

#include <utility>

namespace EDX
{
	namespace Math
	{
		template <typename T>
		inline bool NumericValid(T num) { return !_isnan(num) && _finite(num); }

		template <class T1, class T2, class T3>
		inline T1 Clamp(T1 tVal, T2 tmn, T3 tMax)
		{
			if (tVal < tmn) return tmn;
			if (tVal > tMax) return tMax;
			return tVal;
		}
		inline float Saturate(float fVal) { return Clamp(fVal, 0.0f, 1.0f); }
		inline float Pow(float fVal, float fPow) { return powf(fVal, fPow); }
		//inline float Abs(float fVal) { return fabsf(fVal); }
		template <class T> inline T Abs(T tVal) { return tVal >= 0 ? tVal : -tVal; }
		inline float Sqrt(float fVal) { return sqrtf(fVal); }
		inline float ToRadians(float fDeg) { return (fDeg / 180.0f) * float(Math::EDX_PI); }
		inline float ToDegrees(float fRad) { return (fRad / float(Math::EDX_PI)) * 180.0f; }
		inline float Sin(float fVal) { return sinf(fVal); }
		inline float Cos(float fVal) { return cosf(fVal); }
		inline void SinCos(float fVal, float& fSin, float& fCos) { fSin = sinf(fVal); fCos = cosf(fVal); }
		inline float Tan(float fVal) { return tanf(fVal); }
		inline float Exp(float fVal) { return expf(fVal); }
		inline float Atan2(float fVal1, float fVal2) { return atan2f(fVal1, fVal2); }

		inline int FloorToInt(float fVal) { return (int)fVal; }
		inline int CeilToInt(float fVal)
		{
			int iVal = (int)fVal;
			if (fVal - iVal > 1e-5f)
			{
				return iVal + 1;
			}
			else
			{
				return iVal;
			}
		}
		inline int RoundToInt(float fVal) { return FloorToInt(fVal + 0.5f); }

		template <class T1, class T2>
		inline float LinStep(const T1& tVal, const T2& tmn, const T2& tMax)
		{
			if (tmn == tMax)
			{
				return 0.0f;
			}
			return Saturate((tVal - tmn) / (tMax - tmn));
		}
		template<class T1, class T2> inline auto Lerp(const T1& min, const T2& eMax, const float fVal) -> decltype(min + eMax) { return min * (1 - fVal) + eMax * fVal; }
		template<class T1, class T2> inline auto Max(const T1& e1, const T2& e2) -> decltype(e1 + e2)  { return e1 > e2 ? e1 : e2; }
		template<class T1, class T2> inline auto AbsMax(const T1& e1, const T2& e2) -> decltype(e1 + e2)  { return Abs(e1) > Abs(e2) ? e1 : e2; }
		template<class T1, class T2> inline auto Min(const T1& e1, const T2& e2) -> decltype(e1 + e2)  { return e1 < e2 ? e1 : e2; }
		template<class T1, class T2> inline auto AbsMin(const T1& e1, const T2& e2) -> decltype(e1 + e2)  { return Abs(e1) < Abs(e2) ? e1 : e2; }

		template<class T1, class T2, class T3, class T4>
		inline auto BiLerp(const T1& e00, const T2& e01, const T3& e10, const T4& e11, const float fVal1, const float fVal2) -> decltype(e00 + e01 + e10 + e11)
		{
			return Lerp(Lerp(e00, e01, fVal1), Lerp(e10, e11, fVal1), fVal2);
		}
		template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
		inline auto TriLerp(const T1& e000, const T2& e001, const T3& e010, const T4& e011, const T5& e100, const T6& e101, const T7& e110, const T8& e111, const float fVal1, const float fVal2, const float fVal3) -> decltype(e000 + e001 + e010 + e011 + e100 + e101 + e110 + e111)
		{
			return Lerp(BiLerp(e000, e001, e010, e011, fVal1, fVal2), BiLerp(e100, e101, e110, e111, fVal1, fVal2), fVal3);
		}

		inline bool IsPowOfTwo(const int iVal)
		{
			return (iVal & (iVal - 1)) == 0;
		}
		inline int RoundUpPowTwo(uint iVal)
		{
			iVal--;
			iVal |= iVal >> 1;
			iVal |= iVal >> 2;
			iVal |= iVal >> 4;
			iVal |= iVal >> 8;
			iVal |= iVal >> 16;
			return iVal + 1;
		}
		inline int RoundUpTo(const uint iVal, const uint iRound)
		{
			uint iRet = iVal + iRound - (iVal % iRound);
			return iRet;
		}

		template<uint Dim> class Pow2
		{
		public:
			static const int Value = 1 << Dim;
		};
		template<uint Dim> class Pow4
		{
		public:
			static const int Value = 1 << 2 * Dim;
		};

		inline float Xor(const float f1, const float f2)
		{
			return _mm_cvtss_f32(_mm_xor_ps(_mm_set_ss(f1), _mm_set_ss(f2)));
		}

		inline float SignMask(const float f)
		{
			return _mm_cvtss_f32(_mm_and_ps(_mm_set_ss(f), _mm_castsi128_ps(_mm_set1_epi32(0x80000000))));
		}

		template<class T>
		inline T MonoCubicLerp(const T& e0, const T& e1, const T& e2, const T& e3, const float fLerp)
		{
			assert(abs(e2) < 1e8f);
			T eVal = Math::Lerp(e1, e2, fLerp);

			T emnusCur = fLerp;
			T emnusCurSqr = emnusCur * emnusCur;
			T emnusCurCub = emnusCurSqr * emnusCur;

			T eK = e2 - e1;
			T eD1 = (e2 - e0) * 0.5f;
			T eD2 = (e3 - e1) * 0.5f;
			if (eK * eD1 < 0)
			{
				eD1 = 0;
			}
			if (eK * eD2 < 0)
			{
				eD2 = 0;
			}
			if (eK == 0)
			{
				eD1 = eD2 = 0;
			}

			T eA0 = e1;
			T eA1 = eD1;
			T eA2 = 3 * eK - 2 * eD1 - eD2;
			T eA3 = eD1 + eD2 - 2 * eK;
			T eRet = eA3 * emnusCurCub + eA2 * emnusCurSqr + eA1 * emnusCur + eA0;
			//eRet = eA0 * emnusCurCub + eA1 * emnusCurSqr + eA2 * emnusCur + eA3;
			assert(NumericValid(eRet));

			return eRet;
		}
	}
}