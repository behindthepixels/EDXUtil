#pragma once

#include "Constants.h"
#include "VecBase.h"
#include <iostream>

namespace EDX
{
	template<class T>
	class Vec<2, T>
	{
	public:
		union
		{
			struct { T x, y; };
			struct { T u, v; };
		};

	public:
		//----------------------------------------------------------------------------------------------
		// Constructors, Assignment & Cast Operators
		//----------------------------------------------------------------------------------------------
		inline Vec()
			: x(Math::EDX_ZERO), y(Math::EDX_ZERO) {}

		inline Vec(const Vec& vCopyFrom)
			: x(vCopyFrom.x), y(vCopyFrom.y) {}

		inline Vec(const T& tVal)
			: x(tVal), y(tVal) {}

		inline Vec(const T& tx, const T& ty)
			: x(tx), y(ty) {}

		template<class T1>
		inline Vec(const Vec<2, T1>& vConvertFrom)
			: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)) {}

		template<class T1>
		inline Vec& operator = (const Vec<2, T1>& vOther)
		{
			x = T(vOther.x); y = T(vOther.y);
			return *this;
		}

		~Vec() {}

		inline T Sum() const { return x + y; }
		inline T Product() const { return x * y; }

		inline const T& operator [] (const size_t idx) const { assert(idx < 2); return (&x)[idx]; }
		inline		 T& operator [] (const size_t idx)		 { assert(idx < 2); return (&x)[idx]; }

		//----------------------------------------------------------------------------------------------
		// Unary Operators
		//----------------------------------------------------------------------------------------------
		inline Vec operator + () const { return Vec(+x, +y); }
		inline Vec operator - () const { return Vec(-x, -y); }

		//----------------------------------------------------------------------------------------------
		// Binary Operators
		//----------------------------------------------------------------------------------------------
		inline Vec operator + (const Vec& rhs) const { return Vec(x + rhs.x, y + rhs.y); }
		inline Vec operator - (const Vec& rhs) const { return Vec(x - rhs.x, y - rhs.y); }
		inline Vec operator * (const Vec& rhs) const { return Vec(x * rhs.x, y * rhs.y); }
		inline Vec operator * (const T& rhs) const { return Vec(x * rhs, y * rhs); }
		inline Vec operator / (const Vec& rhs) const { return Vec(x / rhs.x, y / rhs.y); }
		inline Vec operator / (const T& rhs) const { return Vec(x / rhs, y / rhs); }

		//----------------------------------------------------------------------------------------------
		// Assignment Operators
		//----------------------------------------------------------------------------------------------
		inline const Vec& operator += (const Vec& rhs) { x += rhs.x; y += rhs.y; return *this; }
		inline const Vec& operator -= (const Vec& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
		inline const Vec& operator *= (const T& rhs) { x *= rhs; y *= rhs; return *this; }
		inline const Vec& operator /= (const T& rhs) { x /= rhs; y /= rhs; return *this; }
		inline const Vec& operator *= (const Vec& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
		inline const Vec& operator /= (const Vec& rhs) { x /= rhs.x; y /= rhs.y; return *this; }

		//----------------------------------------------------------------------------------------------
		// Comparison Operators
		//----------------------------------------------------------------------------------------------
		inline bool operator == (const Vec& rhs) const { return x == rhs.x && y == rhs.y; }
		inline bool operator != (const Vec& rhs) const { return x != rhs.x || y != rhs.y; }

		static const Vec ZERO;
		static const Vec UNIT_SCALE;
		static const Vec UNIT_X;
		static const Vec UNIT_Y;
		static const Vec UNIT[2];
	};

	template<class T> const Vec<2, T> Vec<2, T>::ZERO(Math::EDX_ZERO);
	template<class T> const Vec<2, T> Vec<2, T>::UNIT_SCALE(Math::EDX_ONE);
	template<class T> const Vec<2, T> Vec<2, T>::UNIT_X(Math::EDX_ONE, Math::EDX_ZERO);
	template<class T> const Vec<2, T> Vec<2, T>::UNIT_Y(Math::EDX_ZERO, Math::EDX_ONE);
	template<class T> const Vec<2, T> Vec<2, T>::UNIT[2] =
	{
		Vec<2, T>(Math::EDX_ONE, Math::EDX_ZERO),
		Vec<2, T>(Math::EDX_ZERO, Math::EDX_ONE)
	};

	//----------------------------------------------------------------------------------------------
	// Binary Operators
	//----------------------------------------------------------------------------------------------
	template<class T> inline Vec<2, T> operator * (const T& lhs, const Vec<2, T>& rhs) { return rhs * lhs; }

	//----------------------------------------------------------------------------------------------
	// Output Operators
	//----------------------------------------------------------------------------------------------
	template<typename T> inline std::ostream& operator << (std::ostream& out, const Vec<2, T>& rhs)
	{
		return out << "(" << rhs.x << ", " << rhs.y << ")";
	}

	typedef Vec<2, float>	Vector2;
	typedef Vec<2, int>		Vector2i;
	typedef Vec<2, bool>	Vector2b;
}