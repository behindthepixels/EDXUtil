#pragma once

#include "Constants.h"
#include "VecBase.h"
#include <iostream>

namespace EDX
{
	template<class T>
	class Vec<3, T>
	{
	public:
		T x, y, z;

	public:
		//----------------------------------------------------------------------------------------------
		// Constructors, Assignment & Cast Operators
		//----------------------------------------------------------------------------------------------
		inline Vec()
			: x(Math::EDX_ZERO), y(Math::EDX_ZERO), z(Math::EDX_ZERO) {}

		inline Vec(const Vec& vCopyFrom)
			: x(vCopyFrom.x), y(vCopyFrom.y), z(vCopyFrom.z) {}

		inline Vec(const T& tVal)
			: x(tVal), y(tVal), z(tVal) {}

		inline Vec(const T& tx, const T& ty, const T& tz)
			: x(tx), y(ty), z(tz) {}

		inline Vec(const Vec<2, T>& vCopyFrom, T val = T(Math::EDX_ZERO))
			: x(vCopyFrom.x), y(vCopyFrom.y), z(val) {}

		template<class T1>
		inline Vec(const Vec<3, T1>& vConvertFrom)
			: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(vConvertFrom.z)) {}

		template<class T1>
		inline Vec(const Vec<2, T1>& vConvertFrom, T1 val)
			: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(val)) {}

		template<class T1>
		inline Vec& operator = (const Vec<3, T1>& vOther)
		{
			x = T(vOther.x); y = T(vOther.y); z = T(vOther.z);
			return *this;
		}

		~Vec() {}

		inline T Sum() const { return x + y + z; }
		inline T Product() const { return x * y * z; }

		inline const T& operator [] (const size_t idx) const { assert(idx < 3); return (&x)[idx]; }
		inline		 T& operator [] (const size_t idx)		 { assert(idx < 3); return (&x)[idx]; }

		//----------------------------------------------------------------------------------------------
		// Unary Operators
		//----------------------------------------------------------------------------------------------
		inline Vec operator + () const { return Vec(+x, +y, +z); }
		inline Vec operator - () const { return Vec(-x, -y, -z); }

		//----------------------------------------------------------------------------------------------
		// Binary Operators
		//----------------------------------------------------------------------------------------------
		inline Vec operator + (const Vec& rhs) const { return Vec(x + rhs.x, y + rhs.y, z + rhs.z); }
		inline Vec operator - (const Vec& rhs) const { return Vec(x - rhs.x, y - rhs.y, z - rhs.z); }
		inline Vec operator * (const Vec& rhs) const { return Vec(x * rhs.x, y * rhs.y, z * rhs.z); }
		inline Vec operator * (const T& rhs) const { return Vec(x * rhs, y * rhs, z * rhs); }
		inline Vec operator / (const Vec& rhs) const { return Vec(x / rhs.x, y / rhs.y, z / rhs.z); }
		inline Vec operator / (const T& rhs) const { return Vec(x / rhs, y / rhs, z / rhs); }

		//----------------------------------------------------------------------------------------------
		// Assignment Operators
		//----------------------------------------------------------------------------------------------
		inline const Vec& operator += (const Vec& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
		inline const Vec& operator -= (const Vec& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
		inline const Vec& operator *= (const T& rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
		inline const Vec& operator /= (const T& rhs) { x /= rhs; y /= rhs; z /= rhs; return *this; }
		inline const Vec& operator *= (const Vec& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
		inline const Vec& operator /= (const Vec& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

		//----------------------------------------------------------------------------------------------
		// Comparison Operators
		//----------------------------------------------------------------------------------------------
		inline bool operator == (const Vec& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
		inline bool operator != (const Vec& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }

		static const Vec ZERO;
		static const Vec UNIT_SCALE;
		static const Vec UNIT_X;
		static const Vec UNIT_Y;
		static const Vec UNIT_Z;
		static const Vec UNIT[3];
	};

	template<class T> const Vec<3, T> Vec<3, T>::ZERO(Math::EDX_ZERO);
	template<class T> const Vec<3, T> Vec<3, T>::UNIT_SCALE(Math::EDX_ONE);
	template<class T> const Vec<3, T> Vec<3, T>::UNIT_X(Math::EDX_ONE, Math::EDX_ZERO, Math::EDX_ZERO);
	template<class T> const Vec<3, T> Vec<3, T>::UNIT_Y(Math::EDX_ZERO, Math::EDX_ONE, Math::EDX_ZERO);
	template<class T> const Vec<3, T> Vec<3, T>::UNIT_Z(Math::EDX_ZERO, Math::EDX_ZERO, Math::EDX_ONE);
	template<class T> const Vec<3, T> Vec<3, T>::UNIT[3] =
	{
		Vec<3, T>(Math::EDX_ONE, Math::EDX_ZERO, Math::EDX_ZERO),
		Vec<3, T>(Math::EDX_ZERO, Math::EDX_ONE, Math::EDX_ZERO),
		Vec<3, T>(Math::EDX_ZERO, Math::EDX_ZERO, Math::EDX_ONE)
	};

	//----------------------------------------------------------------------------------------------
	// Binary Operators
	//----------------------------------------------------------------------------------------------
	template<class T> inline Vec<3, T> operator * (const T& lhs, const Vec<3, T>& rhs) { return rhs * lhs; }

	//----------------------------------------------------------------------------------------------
	// Output Operators
	//----------------------------------------------------------------------------------------------
	template<typename T> inline std::ostream& operator << (std::ostream& out, const Vec<3, T>& rhs)
	{
		return out << "(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")";
	}

	typedef Vec<3, float>	Vector3;
	typedef Vec<3, int>		Vector3i;
	typedef Vec<3, bool>	Vector3b;
}