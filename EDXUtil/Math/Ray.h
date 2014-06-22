#pragma once

#include "Vec3.h"

namespace EDX
{
#define RAY_EPSI 1e-3f
	class Ray
	{
	public:
		Vector3 mptOrg;
		Vector3 mvDir;

		mutable float mfMin, mfMax;
		int miDepth;

	public:
		Ray()
			: mptOrg(Vector3::ZERO), mvDir(Vector3::UNIT_Z),
			miDepth(0), mfMin(RAY_EPSI), mfMax(float(Math::EDX_INFINITY))
		{
		}
		Ray(const Vector3& ptOrig, const Vector3& vDir, float fMax = float(Math::EDX_INFINITY), float fMin = 0.0f, int iDepth = 0)
			: mptOrg(ptOrig), mvDir(vDir), miDepth(iDepth), mfMin(fMin + RAY_EPSI), mfMax(fMax - RAY_EPSI)
		{
		}
		~Ray()
		{
		}

		inline Vector3 CalcPoint(float fDist) const { return mptOrg + mvDir * fDist; }
	};

	class RayDifferential : public Ray
	{
	public:
		Vector3 mvDxOrg, mvDyOrg;
		Vector3 mvDxDir, mvDyDir;

		bool mbHasDifferential;

	public:
		RayDifferential()
			: Ray(), mbHasDifferential(false)
		{
		}
		RayDifferential(const Vector3& ptOrig, const Vector3& vDir, float fMax = float(Math::EDX_INFINITY), float fMin = 0.0f, int iDepth = 0)
			: Ray(ptOrig, vDir, fMax, fMin, iDepth), mbHasDifferential(false)
		{
		}
		RayDifferential(const Ray& ray)
			: Ray(ray), mbHasDifferential(false)
		{
		}
	};
}