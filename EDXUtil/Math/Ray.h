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

		mutable float mMin, mMax;
		int mDepth;

	public:
		Ray()
			: mOrg(Vector3::ZERO), mDir(Vector3::UNIT_Z),
			mDepth(0), mMin(RAY_EPSI), mMax(float(Math::EDX_INFINITY))
		{
		}
		Ray(const Vector3& pt, const Vector3& dir, float max = float(Math::EDX_INFINITY), float min = 0.0f, int depth = 0)
			: mOrg(pt), mDir(dir), mDepth(depth), mMin(min + RAY_EPSI), mMax(max - RAY_EPSI)
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

		bool mHasDifferential;

	public:
		RayDifferential()
			: Ray(), mHasDifferential(false)
		{
		}
		RayDifferential(const Vector3& vOrig, const Vector3& vDir, float max = float(Math::EDX_INFINITY), float min = 0.0f, int depth = 0)
			: Ray(vOrig, vDir, max, min, depth), mHasDifferential(false)
		{
		}
		RayDifferential(const Ray& ray)
			: Ray(ray), mHasDifferential(false)
		{
		}
	};
}