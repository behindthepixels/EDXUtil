#pragma once

#include "Vec3.h"

namespace EDX
{
#define RAY_EPSI 1e-4f
	class Ray
	{
	public:
		Vector3 mOrg;
		Vector3 mDir;

		mutable float mn, max;
		int mDepth;

	public:
		Ray()
			: mOrg(Vector3::ZERO), mDir(Vector3::UNIT_Z),
			mDepth(0), mn(RAY_EPSI), max(float(Math::EDX_INFINITY))
		{
		}
		Ray(const Vector3& vOrig, const Vector3& vDir, float fMax = float(Math::EDX_INFINITY), float fmn = 0.0f, int iDepth = 0)
			: mOrg(vOrig), mDir(vDir), mDepth(iDepth), mn(fmn + RAY_EPSI), max(fMax - RAY_EPSI)
		{
		}
		~Ray()
		{
		}

		inline Vector3 CalcPoint(float fDist) const { return mOrg + mDir * fDist; }
	};

	class RayDifferential : public Ray
	{
	public:
		Vector3 mDxOrg, mDyOrg;
		Vector3 mDxDir, mDyDir;

		bool mbHasDifferential;

	public:
		RayDifferential()
			: Ray(), mbHasDifferential(false)
		{
		}
		RayDifferential(const Vector3& vOrig, const Vector3& vDir, float fMax = float(Math::EDX_INFINITY), float fmn = 0.0f, int iDepth = 0)
			: Ray(vOrig, vDir, fMax, fmn, iDepth), mbHasDifferential(false)
		{
		}
		RayDifferential(const Ray& ray)
			: Ray(ray), mbHasDifferential(false)
		{
		}
	};
}