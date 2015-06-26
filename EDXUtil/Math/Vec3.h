#pragma once

#include "Constants.h"
#include "VecBase.h"
#include "EDXMath.h"
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
		__forceinline Vec()
			: x(Math::EDX_ZERO), y(Math::EDX_ZERO), z(Math::EDX_ZERO)
		{
		}

		__forceinline Vec(const Vec& vCopyFrom)
			: x(vCopyFrom.x), y(vCopyFrom.y), z(vCopyFrom.z)
		{
			NumericValid();
		}

		__forceinline Vec(const T& tVal)
			: x(tVal), y(tVal), z(tVal)
		{
			NumericValid();
		}

		__forceinline Vec(const T& tx, const T& ty, const T& tz)
			: x(tx), y(ty), z(tz)
		{
			NumericValid();
		}

		//__forceinline Vec(const Vec<2, T>& vCopyFrom, T val = T(Math::EDX_ZERO))
		//	: x(vCopyFrom.x), y(vCopyFrom.y), z(val) {}

		template<class T1>
		__forceinline Vec(const Vec<3, T1>& vConvertFrom)
			: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(vConvertFrom.z))
		{
			NumericValid();
		}

		//template<class T1>
		//__forceinline Vec(const Vec<2, T1>& vConvertFrom, T1 val)
		//	: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(val)) {}

		template<class T1>
		__forceinline Vec& operator = (const Vec<3, T1>& vOther)
		{
			x = T(vOther.x); y = T(vOther.y); z = T(vOther.z);
			NumericValid();

			return *this;
		}

		~Vec() {}

		__forceinline T Sum() const { return x + y + z; }
		__forceinline T Product() const { return x * y * z; }

		__forceinline const T& operator [] (const size_t idx) const { assert(idx < 3); return (&x)[idx]; }
		__forceinline		T& operator [] (const size_t idx)		{ assert(idx < 3); return (&x)[idx]; }

		//----------------------------------------------------------------------------------------------
		// Unary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline Vec operator + () const { return Vec(+x, +y, +z); }
		__forceinline Vec operator - () const { return Vec(-x, -y, -z); }

		//----------------------------------------------------------------------------------------------
		// Binary Operators
		//----------------------------------------------------------------------------------------------
		__forceinline Vec operator + (const Vec& rhs) const { return Vec(x + rhs.x, y + rhs.y, z + rhs.z); }
		__forceinline Vec operator - (const Vec& rhs) const { return Vec(x - rhs.x, y - rhs.y, z - rhs.z); }
		__forceinline Vec operator * (const Vec& rhs) const { return Vec(x * rhs.x, y * rhs.y, z * rhs.z); }
		__forceinline Vec operator * (const T& rhs) const { return Vec(x * rhs, y * rhs, z * rhs); }
		__forceinline Vec operator / (const Vec& rhs) const { return Vec(x / rhs.x, y / rhs.y, z / rhs.z); }
		__forceinline Vec operator / (const T& rhs) const { return Vec(x / rhs, y / rhs, z / rhs); }
		__forceinline Vec operator << (const int shift) const { return Vec(x << shift, y << shift, z << shift); }
		__forceinline Vec operator >> (const int shift) const { return Vec(x >> shift, y >> shift, z >> shift); }
		__forceinline Vec operator & (const int bits) const { return Vec(x & bits, y & bits, z & bits); }
		__forceinline Vec operator | (const int bits) const { return Vec(x | bits, y | bits, z | bits); }

		//----------------------------------------------------------------------------------------------
		// Assignment Operators
		//----------------------------------------------------------------------------------------------
		__forceinline const Vec& operator += (const Vec& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; NumericValid(); return *this; }
		__forceinline const Vec& operator -= (const Vec& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; NumericValid(); return *this; }
		__forceinline const Vec& operator *= (const T& rhs) { x *= rhs; y *= rhs; z *= rhs; NumericValid(); return *this; }
		__forceinline const Vec& operator /= (const T& rhs) { x /= rhs; y /= rhs; z /= rhs; NumericValid(); return *this; }
		__forceinline const Vec& operator *= (const Vec& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; NumericValid(); return *this; }
		__forceinline const Vec& operator /= (const Vec& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; NumericValid(); return *this; }
		__forceinline const Vec& operator <<= (const int shift) { x <<= shift; y <<= shift; z <<= shift; NumericValid(); return *this; }
		__forceinline const Vec& operator >>= (const int shift) { x >>= shift; y >>= shift; z >>= shift; NumericValid(); return *this; }
		__forceinline const Vec& operator &= (const int bits) const { x &= bits, y &= bits; z &= bits; NumericValid(); return *this; }
		__forceinline const Vec& operator |= (const int bits) const { x |= bits, y |= bits; z &= bits; NumericValid(); return *this; }

		//----------------------------------------------------------------------------------------------
		// Comparison Operators
		//----------------------------------------------------------------------------------------------
		__forceinline bool operator == (const Vec& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
		__forceinline bool operator != (const Vec& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }

		__forceinline void NumericValid() const
		{
			assert(!Math::IsNAN(x));
			assert(!Math::IsNAN(y));
			assert(!Math::IsNAN(z));
		}

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
	template<class T> __forceinline Vec<3, T> operator * (const T& lhs, const Vec<3, T>& rhs) { return rhs * lhs; }

	//----------------------------------------------------------------------------------------------
	// Output Operators
	//----------------------------------------------------------------------------------------------
	template<typename T> __forceinline std::ostream& operator << (std::ostream& out, const Vec<3, T>& rhs)
	{
		return out << "(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")";
	}

	typedef Vec<3, float>	Vector3;
	typedef Vec<3, int>		Vector3i;
	typedef Vec<3, bool>	Vector3b;

	namespace Math
	{
		template<class T>
		__forceinline Vec<3, T> ToVec3(const Vec<2, T>& vCopyFrom, T val = T(Math::EDX_ZERO))
		{
			return Vec<3, T>(vCopyFrom.x, vCopyFrom.y, val);
		}
		template<class T>
		__forceinline Vec<3, T> ToVec3(const Vec<3, T>& vCopyFrom)
		{
			return vCopyFrom;
		}
		template<class T>
		__forceinline T Dot(const Vec<3, T>& vec1, const Vec<3, T>& vec2)
		{
			return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
		}
		template<class T>
		__forceinline T AbsDot(const Vec<3, T>& vec1, const Vec<3, T>& vec2)
		{
			T ret = Dot(vec1, vec2); return ret >= 0 ? ret : -ret;
		}
		template<class T>
		__forceinline Vec<3, T> Cross(const Vec<3, T>& vec1, const Vec<3, T>& vec2)
		{
			return Vec<3, T>(vec1.y * vec2.z - vec1.z * vec2.y,
				vec1.z * vec2.x - vec1.x * vec2.z,
				vec1.x * vec2.y - vec1.y * vec2.x);
		}
		template<class T>
		__forceinline Vec<3, T> Curl(const Vec<3, T>& vDvdx, const Vec<3, T>& vDvdy, const Vec<3, T>& vDvdz)
		{
			return Vec<3, T>(vDvdy.z - vDvdz.y, vDvdz.x - vDvdx.z, vDvdx.y - vDvdy.x);
		}
		template<class T>
		__forceinline T LengthSquared(const Vec<3, T>& vec)
		{
			return Dot(vec, vec);
		}
		template<class T>
		__forceinline T Min(const Vec<3, T>& v)
		{
			return Math::Min(Math::Min(v.x, v.y), v.z);
		}
		template<class T>
		__forceinline T Max(const Vec<3, T>& v)
		{
			return Math::Max(Math::Max(v.x, v.y), v.z);
		}
		template<class T>
		__forceinline float Length(const Vec<3, T>& vec)
		{
			return Math::Sqrt(LengthSquared(vec));
		}
		__forceinline Vector3 Normalize(const Vector3& v)
		{
			return v * Math::Rsqrt(Math::LengthSquared(v));
		}
		__forceinline float Distance(const Vector3& p1, const Vector3& p2)
		{
			return Length(p1 - p2);
		}
		__forceinline float DistanceSquared(const Vector3& p1, const Vector3& p2)
		{
			return LengthSquared(p1 - p2);
		}
		__forceinline Vector3 Reflect(const Vector3& vInci, const Vector3& nNorm)
		{
			return vInci + Vector3(2 * Dot(-vInci, nNorm) * nNorm);
		}
		__forceinline Vector3 Refract(const Vector3& wi, const Vector3& n, float eta)
		{
			float cosThetaI = Dot(wi, n);
			if (cosThetaI < 0)
				eta = 1.0f / eta;

			float cosThetaTSqr = 1 - (1 - cosThetaI*cosThetaI) * (eta*eta);

			if (cosThetaTSqr <= 0.0f)
				return Vector3::ZERO;

			float sign = cosThetaI >= 0.0f ? 1.0f : -1.0f;
			return n * (-cosThetaI * eta + sign * Sqrt(cosThetaTSqr)) + wi * eta;
		}
		__forceinline Vector3 FaceForward(const Vector3& n, const Vector3& v)
		{
			return (Dot(n, v) < 0.0f) ? -n : n;
		}
		__forceinline Vector3 SphericalDirection(float fSinTheta, float fCosTheta, float fPhi)
		{
			return Vector3(fSinTheta * Math::Cos(fPhi),
				fCosTheta,
				fSinTheta * Math::Sin(fPhi));
		}
		__forceinline Vector3 SphericalDirection(float fSinTheta, float fCosTheta,
			float fPhi, const Vector3& vX,
			const Vector3& vY, const Vector3& vZ)
		{
			return fSinTheta * Math::Cos(fPhi) * vX +
				fCosTheta * vY + fSinTheta * Math::Sin(fPhi) * vZ;
		}
		__forceinline float SphericalTheta(const Vector3& vec)
		{
			return acosf(Math::Clamp(vec.y, -1.0f, 1.0f));
		}
		__forceinline float SphericalPhi(const Vector3& vec)
		{
			float p = Math::Atan2(-vec.z, vec.x);
			return (p < 0.0f) ? p + float(Math::EDX_TWO_PI) : p;
		}

		template<uint Dimension>
		__forceinline Vec<Dimension, int> FloorToInt(const Vec<Dimension, float>& vec)
		{
			Vec<Dimension, int> vRet;
			for (auto d = 0; d < Dimension; d++)
				vRet[d] = FloorToInt(vec[d]);
			return vRet;
		}
		template<uint Dimension>
		__forceinline Vec<Dimension, int> RoundToInt(const Vec<Dimension, float>& vec)
		{
			Vec<Dimension, int> vRet;
			for (auto d = 0; d < Dimension; d++)
				vRet[d] = RoundToInt(vec[d]);
			return vRet;
		}

		__forceinline void CoordinateSystem(const Vector3& vec1, Vector3* vec2, Vector3* vec3)
		{
			if (Math::Abs(vec1.x) > Math::Abs(vec1.y))
			{
				float fInvLen = 1.0f / Math::Sqrt(vec1.x * vec1.x + vec1.z * vec1.z);
				*vec2 = Vector3(-vec1.z * fInvLen, 0.0f, vec1.x * fInvLen);
			}
			else
			{
				float fInvLen = 1.0f / Math::Sqrt(vec1.y * vec1.y + vec1.z * vec1.z);
				*vec2 = Vector3(0.0f, vec1.z * fInvLen, -vec1.y * fInvLen);
			}
			*vec3 = Cross(vec1, *vec2);
		}
	}
}