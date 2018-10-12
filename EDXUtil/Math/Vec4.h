#pragma once

#include "Constants.h"
#include "VecBase.h"
#include <iostream>

namespace EDX
{
	template<class T>
	class Vec<4, T>
	{
	public:
		T x, y, z, w;

	public:
		//----------------------------------------------------------------------------------------------
		// Constructors, Assignment & Cast Operators
		//----------------------------------------------------------------------------------------------
		__forceinline Vec()
			: x(Math::EDX_ZERO), y(Math::EDX_ZERO), z(Math::EDX_ZERO), w(Math::EDX_ZERO) {}

		__forceinline Vec(const Vec& vCopyFrom)
			: x(vCopyFrom.x), y(vCopyFrom.y), z(vCopyFrom.z), w(vCopyFrom.w) {}

		__forceinline Vec& operator = (const Vec& vOther)
		{
			x = vOther.x; y = vOther.y; z = vOther.z; w = vOther.w;
			return *this;
		}

		__forceinline Vec(const T& tVal)
			: x(tVal), y(tVal), z(tVal), w(tVal) {}

		__forceinline Vec(const T& tx, const T& ty, const T& tz, const T& tw)
			: x(tx), y(ty), z(tz), w(tw) {}

		//__forceinline Vec(const Vec<2, T>& vCopyFrom, T valZ = T(Math::EDX_ZERO), T valW = T(Math::EDX_ZERO))
		//	: x(vCopyFrom.x), y(vCopyFrom.y), z(valZ), w(valW) {}

		//__forceinline Vec(const Vec<3, T>& vCopyFrom, T val = T(Math::EDX_ZERO))
		//	: x(vCopyFrom.x), y(vCopyFrom.y), z(vCopyFrom.z), w(val) {}

		template<class T1>
		__forceinline Vec(const Vec<4, T1>& vConvertFrom)
			: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(vConvertFrom.z)), w(T(vConvertFrom.w)) {}

		//template<class T1>
		//__forceinline Vec(const Vec<2, T1>& vConvertFrom, T1 valZ, T1 valW)
		//	: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(valZ)), w(T1(valW)) {}

		//template<class T1>
		//__forceinline Vec(const Vec<3, T1>& vConvertFrom, T1 valW)
		//	: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(vConvertFrom.z)), w(T1(valW)) {}

		template<class T1>
		__forceinline Vec& operator = (const Vec<4, T1>& vOther)
		{
			x = T(vOther.x); y = T(vOther.y); z = T(vOther.z); w = T(vOther.w);
			return *this;
		}

		~Vec() {}

		__forceinline T Sum() const { return x + y + z + w; }
		__forceinline T Product() const { return x * y * z * w; }
		__forceinline Vec<3, T> xyz() const { return Vec<3, T>(x, y, z); }
		__forceinline Vec<3, T> HomogeneousProject() const
		{
			float invW = 1.0f / w;
			return Vec<3, T>(x * invW, y * invW, z * invW);
		}

		__forceinline const T& operator [] (const size_t idx) const { Assert(idx < 4); return (&x)[idx]; }
		__forceinline		 T& operator [] (const size_t idx)		 { Assert(idx < 4); return (&x)[idx]; }

		//----------------------------------------------------------------------------------------------
		// Unary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline Vec operator + () const { return Vec(+x, +y, +z, +w); }
		__forceinline Vec operator - () const { return Vec(-x, -y, -z, -w); }

		//----------------------------------------------------------------------------------------------
		// Binary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline Vec operator + (const Vec& rhs) const { return Vec(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
		__forceinline Vec operator - (const Vec& rhs) const { return Vec(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
		__forceinline Vec operator * (const Vec& rhs) const { return Vec(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
		__forceinline Vec operator * (const T& rhs) const { return Vec(x * rhs, y * rhs, z * rhs, w * rhs); }
		__forceinline Vec operator / (const Vec& rhs) const { return Vec(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }
		__forceinline Vec operator / (const T& rhs) const { return Vec(x / rhs, y / rhs, z / rhs, w / rhs); }
		__forceinline Vec operator << (const int shift) const { return Vec(x << shift, y << shift, z << shift, w << shift); }
		__forceinline Vec operator >> (const int shift) const { return Vec(x >> shift, y >> shift, z >> shift, w >> shift); }
		__forceinline Vec operator & (const int bits) const { return Vec(x & bits, y & bits, z & bits, w & bits); }
		__forceinline Vec operator | (const int bits) const { return Vec(x | bits, y | bits, z | bits, w | bits); }

		//----------------------------------------------------------------------------------------------
		// Assignment Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const Vec& operator += (const Vec& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
		__forceinline const Vec& operator -= (const Vec& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
		__forceinline const Vec& operator *= (const T& rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
		__forceinline const Vec& operator /= (const T& rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }
		__forceinline const Vec& operator *= (const Vec& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
		__forceinline const Vec& operator /= (const Vec& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }
		__forceinline const Vec& operator <<= (const int shift) { x <<= shift; y <<= shift; z <<= shift; w <<= shift; return *this; }
		__forceinline const Vec& operator >>= (const int shift) { x >>= shift; y >>= shift; z >>= shift; w >>= shift; return *this; }
		__forceinline const Vec& operator &= (const int bits) const { x &= bits, y &= bits; z &= bits; w &= bits; return *this; }
		__forceinline const Vec& operator |= (const int bits) const { x |= bits, y |= bits; z &= bits; w |= bits; return *this; }

		//----------------------------------------------------------------------------------------------
		// Comparison Operators
		//----------------------------------------------------------------------------------------------
		__forceinline bool operator == (const Vec& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
		__forceinline bool operator != (const Vec& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w; }

		static const Vec ZERO;
		static const Vec UNIT_SCALE;
		static const Vec UNIT_X;
		static const Vec UNIT_Y;
		static const Vec UNIT_Z;
		static const Vec UNIT_W;
		static const Vec UNIT[4];
	};

	template<class T> const Vec<4, T> Vec<4, T>::ZERO(Math::EDX_ZERO);
	template<class T> const Vec<4, T> Vec<4, T>::UNIT_SCALE(Math::EDX_ONE);
	template<class T> const Vec<4, T> Vec<4, T>::UNIT_X(Math::EDX_ONE, Math::EDX_ZERO, Math::EDX_ZERO, Math::EDX_ZERO);
	template<class T> const Vec<4, T> Vec<4, T>::UNIT_Y(Math::EDX_ZERO, Math::EDX_ONE, Math::EDX_ZERO, Math::EDX_ZERO);
	template<class T> const Vec<4, T> Vec<4, T>::UNIT_Z(Math::EDX_ZERO, Math::EDX_ZERO, Math::EDX_ONE, Math::EDX_ZERO);
	template<class T> const Vec<4, T> Vec<4, T>::UNIT_W(Math::EDX_ZERO, Math::EDX_ZERO, Math::EDX_ZERO, Math::EDX_ONE);
	template<class T> const Vec<4, T> Vec<4, T>::UNIT[4] =
	{
		Vec<4, T>(Math::EDX_ONE, Math::EDX_ZERO, Math::EDX_ZERO, EDX_ZERO),
		Vec<4, T>(Math::EDX_ZERO, Math::EDX_ONE, Math::EDX_ZERO, EDX_ZERO),
		Vec<4, T>(Math::EDX_ZERO, Math::EDX_ZERO, Math::EDX_ONE, EDX_ZERO),
		Vec<4, T>(Math::EDX_ZERO, Math::EDX_ZERO, Math::EDX_ZERO, EDX_ONE)
	};

	//----------------------------------------------------------------------------------------------
	// Binary Operators
	//----------------------------------------------------------------------------------------------
	template<class T> __forceinline Vec<4, T> operator * (const T& lhs, const Vec<4, T>& rhs) { return rhs * lhs; }

	//----------------------------------------------------------------------------------------------
	// Output Operators
	//----------------------------------------------------------------------------------------------
	template<typename T> __forceinline std::ostream& operator << (std::ostream& out, const Vec<4, T>& rhs)
	{
		return out << "(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")";
	}

	typedef Vec<4, float>	Vector4;
	typedef Vec<4, int>		Vector4i;
	typedef Vec<4, bool>	Vector4b;

	namespace Math
	{
		template<class T>
		__forceinline T Min(const Vec<4, T>& v)
		{
			return Math::Min(Math::Min(Math::Min(v.x, v.y), v.z), v.w);
		}
		template<class T>
		__forceinline T Max(const Vec<4, T>& v)
		{
			return Math::Max(Math::Max(Math::Max(v.x, v.y), v.z), v.w);
		}
	}
}