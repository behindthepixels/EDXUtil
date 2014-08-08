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
		inline Vec()
			: x(Math::EDX_ZERO), y(Math::EDX_ZERO), z(Math::EDX_ZERO) {}

		inline Vec(const Vec& vCopyFrom)
			: x(vCopyFrom.x), y(vCopyFrom.y), z(vCopyFrom.z) {}

		inline Vec(const T& tVal)
			: x(tVal), y(tVal), z(tVal) {}

		inline Vec(const T& tx, const T& ty, const T& tz)
			: x(tx), y(ty), z(tz) {}

		//inline Vec(const Vec<2, T>& vCopyFrom, T val = T(Math::EDX_ZERO))
		//	: x(vCopyFrom.x), y(vCopyFrom.y), z(val) {}

		template<class T1>
		inline Vec(const Vec<3, T1>& vConvertFrom)
			: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(vConvertFrom.z)) {}

		//template<class T1>
		//inline Vec(const Vec<2, T1>& vConvertFrom, T1 val)
		//	: x(T(vConvertFrom.x)), y(T(vConvertFrom.y)), z(T(val)) {}

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

	namespace Math
	{
		template<class T>
		inline T Dot(const Vec<3, T>& vVec1, const Vec<3, T>& vVec2)
		{
			return vVec1.x * vVec2.x + vVec1.y * vVec2.y + vVec1.z * vVec2.z;
		}
		template<class T>
		inline T AbsDot(const Vec<3, T>& vVec1, const Vec<3, T>& vVec2)
		{
			T ret = Dot(vVec1, vVec2); return ret >= 0 ? ret : -ret;
		}
		template<class T>
		inline Vec<3, T> Cross(const Vec<3, T>& vVec1, const Vec<3, T>& vVec2)
		{
			return Vec<3, T>(vVec1.y * vVec2.z - vVec1.z * vVec2.y,
				vVec1.z * vVec2.x - vVec1.x * vVec2.z,
				vVec1.x * vVec2.y - vVec1.y * vVec2.x);
		}
		template<class T>
		inline Vec<3, T> Curl(const Vec<3, T>& vDvdx, const Vec<3, T>& vDvdy, const Vec<3, T>& vDvdz)
		{
			return Vec<3, T>(vDvdy.z - vDvdz.y, vDvdz.x - vDvdx.z, vDvdx.y - vDvdy.x);
		}
		template<class T>
		inline T LengthSquared(const Vec<3, T>& vVec)
		{
			return Dot(vVec, vVec);
		}
		inline float Length(const Vector3& vVec)
		{
			return Math::Sqrt(LengthSquared(vVec));
		}
		inline Vector3 Normalize(const Vector3& v)
		{
			return v / Length(v);
		}
		inline float Distance(const Vector3& p1, const Vector3& p2)
		{
			return Length(p1 - p2);
		}
		inline float DistanceSquared(const Vector3& p1, const Vector3& p2)
		{
			return LengthSquared(p1 - p2);
		}
		inline Vector3 Reflect(const Vector3& vInci, const Vector3& nNorm)
		{
			return vInci + Vector3(2 * Dot(-vInci, nNorm) * nNorm);
		}
		inline Vector3 Refract(const Vector3& vInci, const Vector3& nNorm, float eta)
		{
			float NDotI = Dot(nNorm, vInci);
			float k = 1.0f - eta * eta * (1.0f - NDotI * NDotI);
			if (k < 0.0f)
				return Vector3::ZERO;
			else
				return eta * vInci - (eta * NDotI + Math::Sqrt(k)) * Vector3(nNorm);
		}
		inline Vector3 FaceForward(const Vector3& n, const Vector3& v)
		{
			return (Dot(n, v) < 0.0f) ? -n : n;
		}
		inline Vector3 SphericalDirection(float fSinTheta, float fCosTheta, float fPhi)
		{
			return Vector3(fSinTheta * Math::Cos(fPhi),
				fCosTheta,
				fSinTheta * Math::Sin(fPhi));
		}
		inline Vector3 SphericalDirection(float fSinTheta, float fCosTheta,
			float fPhi, const Vector3& vX,
			const Vector3& vY, const Vector3& vZ)
		{
			return fSinTheta * Math::Cos(fPhi) * vX +
				fCosTheta * vY + fSinTheta * Math::Sin(fPhi) * vZ;
		}
		inline float SphericalTheta(const Vector3& vVec)
		{
			return Math::Sin(Math::Clamp(vVec.y, -1.0f, 1.0f));
		}
		inline float SphericalPhi(const Vector3& vVec)
		{
			float p = Math::Atan2(vVec.z, vVec.x);
			return (p < 0.0f) ? p + 2.0f * float(float(Math::EDX_PI)) : p;
		}

		template<uint Dimension>
		inline Vec<Dimension, int> FloorToInt(const Vec<Dimension, float>& vec)
		{
			Vec<Dimension, int> vRet;
			for (auto d = 0; d < Dimension; d++)
				vRet[d] = FloorToInt(vec[d]);
			return vRet;
		}
		template<uint Dimension>
		inline Vec<Dimension, int> RoundToInt(const Vec<Dimension, float>& vec)
		{
			Vec<Dimension, int> vRet;
			for (auto d = 0; d < Dimension; d++)
				vRet[d] = RoundToInt(vec[d]);
			return vRet;
		}

		inline void CoordinateSystem(const Vector3& vVec1, Vector3* vVec2, Vector3* vVec3)
		{
			if (Math::Abs(vVec1.x) > Math::Abs(vVec1.y))
			{
				float fInvLen = 1.0f / Math::Sqrt(vVec1.x * vVec1.x + vVec1.z * vVec1.z);
				*vVec2 = Vector3(-vVec1.z * fInvLen, 0.0f, vVec1.x * fInvLen);
			}
			else
			{
				float fInvLen = 1.0f / Math::Sqrt(vVec1.y * vVec1.y + vVec1.z * vVec1.z);
				*vVec2 = Vector3(0.0f, vVec1.z * fInvLen, -vVec1.y * fInvLen);
			}
			*vVec3 = Cross(vVec1, *vVec2);
		}
	}
}