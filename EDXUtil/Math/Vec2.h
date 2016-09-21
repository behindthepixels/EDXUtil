#pragma once

#include "Constants.h"
#include "VecBase.h"
#include "EDXMath.h"
#include <iostream>

namespace EDX
{
	template<class T>
	class Vec<2, T>
	{
	public:
		union
		{
			struct { T u, v; };
			struct { T x, y; };
		};

	public:
		//----------------------------------------------------------------------------------------------
		// Constructors, Assignment & Cast Operators
		//----------------------------------------------------------------------------------------------
		__forceinline Vec()
			: x(Math::EDX_ZERO), y(Math::EDX_ZERO) {}

		__forceinline Vec(const Vec& vCopyFrom)
			: x(vCopyFrom.x), y(vCopyFrom.y) {}

		__forceinline Vec(const T& tVal)
			: x(tVal), y(tVal) {}

		__forceinline Vec(const T& tx, const T& ty)
			: x(tx), y(ty) {}

		template<class T1>
		__forceinline Vec(const Vec<2, T1>& vConvertFrom)
			: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)) {}

		template<class T1>
		__forceinline Vec& operator = (const Vec<2, T1>& vOther)
		{
			x = T(vOther.x); y = T(vOther.y);
			return *this;
		}

		~Vec() {}

		__forceinline T Sum() const { return x + y; }
		__forceinline T Product() const { return x * y; }

		__forceinline const T& operator [] (const size_t idx) const { Assert(idx < 2); return (&x)[idx]; }
		__forceinline		 T& operator [] (const size_t idx)		 { Assert(idx < 2); return (&x)[idx]; }

		//----------------------------------------------------------------------------------------------
		// Unary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline Vec operator + () const { return Vec(+x, +y); }
		__forceinline Vec operator - () const { return Vec(-x, -y); }

		//----------------------------------------------------------------------------------------------
		// Binary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline Vec operator + (const Vec& rhs) const { return Vec(x + rhs.x, y + rhs.y); }
		__forceinline Vec operator - (const Vec& rhs) const { return Vec(x - rhs.x, y - rhs.y); }
		__forceinline Vec operator * (const Vec& rhs) const { return Vec(x * rhs.x, y * rhs.y); }
		__forceinline Vec operator * (const T& rhs) const { return Vec(x * rhs, y * rhs); }
		__forceinline Vec operator / (const Vec& rhs) const { return Vec(x / rhs.x, y / rhs.y); }
		__forceinline Vec operator / (const T& rhs) const { return Vec(x / rhs, y / rhs); }
		__forceinline Vec operator << (const int shift) const { return Vec(x << shift, y << shift); }
		__forceinline Vec operator >> (const int shift) const { return Vec(x >> shift, y >> shift); }
		__forceinline Vec operator & (const int bits) const { return Vec(x & bits, y & bits); }
		__forceinline Vec operator | (const int bits) const { return Vec(x | bits, y | bits); }

		//----------------------------------------------------------------------------------------------
		// Assignment Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const Vec& operator += (const Vec& rhs) { x += rhs.x; y += rhs.y; return *this; }
		__forceinline const Vec& operator -= (const Vec& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
		__forceinline const Vec& operator *= (const T& rhs) { x *= rhs; y *= rhs; return *this; }
		__forceinline const Vec& operator /= (const T& rhs) { x /= rhs; y /= rhs; return *this; }
		__forceinline const Vec& operator *= (const Vec& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
		__forceinline const Vec& operator /= (const Vec& rhs) { x /= rhs.x; y /= rhs.y; return *this; }
		__forceinline const Vec& operator <<= (const int shift) { x <<= shift; y <<= shift; return *this; }
		__forceinline const Vec& operator >>= (const int shift) { x >>= shift; y >>= shift; return *this; }
		__forceinline const Vec& operator &= (const int bits) const { x &= bits, y &= bits; return *this; }
		__forceinline const Vec& operator |= (const int bits) const { x |= bits, y |= bits; return *this; }

		//----------------------------------------------------------------------------------------------
		// Comparison Operators
		//----------------------------------------------------------------------------------------------
		__forceinline bool operator == (const Vec& rhs) const { return x == rhs.x && y == rhs.y; }
		__forceinline bool operator != (const Vec& rhs) const { return x != rhs.x || y != rhs.y; }

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
	template<class T> __forceinline Vec<2, T> operator * (const T& lhs, const Vec<2, T>& rhs) { return rhs * lhs; }

	//----------------------------------------------------------------------------------------------
	// Output Operators
	//----------------------------------------------------------------------------------------------
	template<typename T> __forceinline std::ostream& operator << (std::ostream& out, const Vec<2, T>& rhs)
	{
		return out << "(" << rhs.x << ", " << rhs.y << ")";
	}

	typedef Vec<2, float>	Vector2;
	typedef Vec<2, int>		Vector2i;
	typedef Vec<2, bool>	Vector2b;

	namespace Math
	{
		template<class T>
		__forceinline T Dot(const Vec<2, T>& vec1, const Vec<2, T>& vec2)
		{
			return vec1.x * vec2.x + vec1.y * vec2.y;
		}
		template<class T>
		__forceinline T AbsDot(const Vec<2, T>& vec1, const Vec<2, T>& vec2)
		{
			T ret = Dot(vec1, vec2);
			return ret >= 0 ? ret : -ret;
		}
		template<class T>
		__forceinline T Cross(const Vec<2, T>& vec1, const Vec<2, T>& vec2)
		{
			return vec1.x * vec2.y - vec1.y * vec2.x;
		}
		template<class T>
		__forceinline T Curl(const Vec<2, T>& vDvdx, const Vec<2, T>& vDvdy)
		{
			return vDvdx.y - vDvdy.x;
		}
		template<class T>
		__forceinline T LengthSquared(const Vec<2, T>& vec)
		{
			return Dot(vec, vec);
		}
		template<class T>
		__forceinline float Length(const Vec<2, T>& vec)
		{
			return Math::Sqrt(LengthSquared(vec));
		}
		__forceinline Vector2 Normalize(const Vector2& v)
		{
			return v / Length(v);
		}
		__forceinline float Distance(const Vector2& p1, const Vector2& p2)
		{
			return Length(p1 - p2);
		}
		__forceinline float DistanceSquared(const Vector2& p1, const Vector2& p2)
		{
			return LengthSquared(p1 - p2);
		}
		template<class T>
		__forceinline T Min(const Vec<2, T>& v)
		{
			return Math::Min(v.x, v.y);
		}
		template<class T>
		__forceinline T Max(const Vec<2, T>& v)
		{
			return Math::Max(v.x, v.y);
		}
	}
}