#pragma once

#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"

namespace EDX
{
	namespace Math
	{
		template<uint Dim, typename T>
		struct LerpFunc
		{
			__forceinline static T Func(const T val[1], const Vec<Dim, float>& vLin)
			{
				return T();
			}
		};
		template<typename T>
		struct LerpFunc<1, T>
		{
			__forceinline static T Func(const T val[1], const Vec<1, float>& vLin)
			{
				return Math::Lerp(val[0], val[1], vLin[0]);
			}
		};
		template<typename T>
		struct LerpFunc<2, T>
		{
			__forceinline static T Func(const T val[1], const Vec<2, float>& vLin)
			{
				return Math::BiLerp(val[0], val[1], val[2], val[3], vLin.x, vLin.y);
			}
		};
		template<typename T>
		struct LerpFunc<3, T>
		{
			__forceinline static T Func(const T val[1], const Vec<3, float>& vLin)
			{
				return Math::TriLerp(val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7], vLin.x, vLin.y, vLin.z);
			}
		};
		template<uint Dim, typename T>
		__forceinline T Lerp(const T val[1], const Vec<Dim, float>& vLin)
		{
			return LerpFunc<Dim, T>::Func(val, vLin);
		}

		template<uint Dimension, typename T1, typename T2, typename T3>
		__forceinline Vec<Dimension, T3> Max(const Vec<Dimension, T1>& vec1, const Vec<Dimension, T1>& vec2)
		{
			Vec<Dimension, T3> ret;
			for (auto d = 0; d < Dimension; d++)
				ret[d] = Math::Max(vec1[d], vec2[d]);
			return ret;
		}

		template<uint Dimension, typename T1, typename T2, typename T3>
		__forceinline Vec<Dimension, T3> Min(const Vec<Dimension, T1>& vec1, const Vec<Dimension, T1>& vec2)
		{
			Vec<Dimension, T3> ret;
			for (auto d = 0; d < Dimension; d++)
				ret[d] = Math::Min(vec1[d], vec2[d]);
			return ret;
		}
		template<uint Dimension>
		__forceinline Vec<Dimension, int> FloorToInt(const Vec<Dimension, float>& vec)
		{
			Vec<Dimension, int> ret;
			for (auto d = 0; d < Dimension; d++)
				ret[d] = FloorToInt(vec[d]);
			return ret;
		}

		template<uint Dimension>
		__forceinline Vec<Dimension, int> RoundToInt(const Vec<Dimension, float>& vec)
		{
			Vec<Dimension, int> ret;
			for (auto d = 0; d < Dimension; d++)
				ret[d] = RoundToInt(vec[d]);
			return ret;
		}

		template<uint Dimension>
		__forceinline Vec<Dimension, float> Sqrt(const Vec<Dimension, float>& vec)
		{
			Vec<Dimension, float> ret;
			for (auto d = 0; d < Dimension; d++)
				ret[d] = Math::Sqrt(vec[d]);
			return ret;
		}

		template<uint Dimension>
		__forceinline Vec<Dimension, float> Exp(const Vec<Dimension, float>& vec)
		{
			Vec<Dimension, float> ret;
			for (auto d = 0; d < Dimension; d++)
				ret[d] = Math::Exp(vec[d]);
			return ret;
		}

		template<uint Dimension>
		__forceinline Vec<Dimension, double> Exp(const Vec<Dimension, double>& vec)
		{
			Vec<Dimension, double> ret;
			for (auto d = 0; d < Dimension; d++)
				ret[d] = Math::Exp(vec[d]);
			return ret;
		}
	}
}