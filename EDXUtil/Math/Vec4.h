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
		inline Vec()
			: x(Math::EDX_ZERO), y(Math::EDX_ZERO), z(Math::EDX_ZERO), w(Math::EDX_ZERO) {}

		inline Vec(const Vec& vCopyFrom)
			: x(vCopyFrom.x), y(vCopyFrom.y), z(vCopyFrom.z), w(vCopyFrom.w) {}

		inline Vec(const T& tVal)
			: x(tVal), y(tVal), z(tVal), w(tVal) {}

		inline Vec(const T& tx, const T& ty, const T& tz, const T& tw)
			: x(tx), y(ty), z(tz), w(tw) {}

		//inline Vec(const Vec<2, T>& vCopyFrom, T valZ = T(Math::EDX_ZERO), T valW = T(Math::EDX_ZERO))
		//	: x(vCopyFrom.x), y(vCopyFrom.y), z(valZ), w(valW) {}

		//inline Vec(const Vec<3, T>& vCopyFrom, T val = T(Math::EDX_ZERO))
		//	: x(vCopyFrom.x), y(vCopyFrom.y), z(vCopyFrom.z), w(val) {}

		template<class T1>
		inline Vec(const Vec<4, T1>& vConvertFrom)
			: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(vConvertFrom.z)), w(T(vConvertFrom.w)) {}

		//template<class T1>
		//inline Vec(const Vec<2, T1>& vConvertFrom, T1 valZ, T1 valW)
		//	: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(valZ)), w(T1(valW)) {}

		//template<class T1>
		//inline Vec(const Vec<3, T1>& vConvertFrom, T1 valW)
		//	: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(vConvertFrom.z)), w(T1(valW)) {}

		template<class T1>
		inline Vec& operator = (const Vec<4, T1>& vOther)
		{
			x = T(vOther.x); y = T(vOther.y); z = T(vOther.z); w = T(vOther.w);
			return *this;
		}

		~Vec() {}

		inline T Sum() const { return x + y + z + w; }
		inline T Product() const { return x * y * z * w; }
		inline Vec<3, T> xyz() const { return Vec<3, T>(x, y, z); }

		inline const T& operator [] (const size_t idx) const { assert(idx < 4); return (&x)[idx]; }
		inline		 T& operator [] (const size_t idx)		 { assert(idx < 4); return (&x)[idx]; }

		//----------------------------------------------------------------------------------------------
		// Unary Operators
		//----------------------------------------------------------------------------------------------
		inline Vec operator + () const { return Vec(+x, +y, +z, +w); }
		inline Vec operator - () const { return Vec(-x, -y, -z, -w); }

		//----------------------------------------------------------------------------------------------
		// Binary Operators
		//----------------------------------------------------------------------------------------------
		inline Vec operator + (const Vec& rhs) const { return Vec(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
		inline Vec operator - (const Vec& rhs) const { return Vec(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
		inline Vec operator * (const Vec& rhs) const { return Vec(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
		inline Vec operator * (const T& rhs) const { return Vec(x * rhs, y * rhs, z * rhs, w * rhs); }
		inline Vec operator / (const Vec& rhs) const { return Vec(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }
		inline Vec operator / (const T& rhs) const { return Vec(x / rhs, y / rhs, z / rhs, w / rhs); }

		//----------------------------------------------------------------------------------------------
		// Assignment Operators
		//----------------------------------------------------------------------------------------------
		inline const Vec& operator += (const Vec& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
		inline const Vec& operator -= (const Vec& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
		inline const Vec& operator *= (const T& rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
		inline const Vec& operator /= (const T& rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }
		inline const Vec& operator *= (const Vec& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
		inline const Vec& operator /= (const Vec& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

		//----------------------------------------------------------------------------------------------
		// Comparison Operators
		//----------------------------------------------------------------------------------------------
		inline bool operator == (const Vec& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
		inline bool operator != (const Vec& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w; }

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
	template<class T> const Vec<4, T> Vec<4, T>::UNIT_X(Math::EDX_ONE, Math::EDX_ZERO, Math::EDX_ZERO, EDX_ZERO);
	template<class T> const Vec<4, T> Vec<4, T>::UNIT_Y(Math::EDX_ZERO, Math::EDX_ONE, Math::EDX_ZERO, EDX_ZERO);
	template<class T> const Vec<4, T> Vec<4, T>::UNIT_Z(Math::EDX_ZERO, Math::EDX_ZERO, Math::EDX_ONE, EDX_ZERO);
	template<class T> const Vec<4, T> Vec<4, T>::UNIT_W(Math::EDX_ZERO, Math::EDX_ZERO, Math::EDX_ZERO, EDX_ONE);
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
	template<class T> inline Vec<4, T> operator * (const T& lhs, const Vec<4, T>& rhs) { return rhs * lhs; }

	//----------------------------------------------------------------------------------------------
	// Output Operators
	//----------------------------------------------------------------------------------------------
	template<typename T> inline std::ostream& operator << (std::ostream& out, const Vec<4, T>& rhs)
	{
		return out << "(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")";
	}

	typedef Vec<4, float>	Vector4;
	typedef Vec<4, int>		Vector4i;
	typedef Vec<4, bool>	Vector4b;
}