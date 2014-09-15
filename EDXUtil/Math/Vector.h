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
	}
}