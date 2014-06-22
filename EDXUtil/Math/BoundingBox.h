#pragma once

#include "Vec3.h"
#include "EDXMath.h"

namespace EDX
{
	class BoundingBox
	{
	public:
		Vector3 mptMin, mptMax;

	public:
		BoundingBox()
			: mptMin(float(Math::EDX_INFINITY), float(Math::EDX_INFINITY), float(Math::EDX_INFINITY))
			, mptMax(float(Math::EDX_NEG_INFINITY), float(Math::EDX_NEG_INFINITY), float(Math::EDX_NEG_INFINITY))
		{}

		BoundingBox(const Vector3& pt)
			: mptMin(pt)
			, mptMax(pt)
		{}

		BoundingBox(
			const Vector3& pt1,
			const Vector3& pt2)
		{
			mptMin = Vector3(Math::Min(pt1.x, pt2.x), Math::Min(pt1.y, pt2.y), Math::Min(pt1.z, pt2.z));
			mptMax = Vector3(Math::Max(pt1.x, pt2.x), Math::Max(pt1.y, pt2.y), Math::Max(pt1.z, pt2.z));
		}

		bool Occluded(const Ray& ray, float* pfHitNear = NULL, float* pfHitFar = NULL) const;

		inline bool Inside(const Vector3& point) const
		{
			return point.x >= mptMin.x && point.x <= mptMax.x &&
				point.y >= mptMin.y && point.y <= mptMax.y &&
				point.z >= mptMin.z && point.z <= mptMax.z;
		}

		inline bool Overlaps(const BoundingBox& bbox) const
		{
			return mptMin.x <= bbox.mptMax.x && mptMax.x >= bbox.mptMin.x &&
				mptMin.y <= bbox.mptMax.y && mptMax.y >= bbox.mptMin.y &&
				mptMin.z <= bbox.mptMax.z && mptMax.z >= bbox.mptMin.x;
		}

		inline int MaximumExtent() const
		{
			Vector3 vDiag = mptMax - mptMin;
			if (vDiag.x > vDiag.y && vDiag.x > vDiag.z)
				return 0;
			else if (vDiag.y > vDiag.z)
				return 1;
			else
				return 2;
		}

		inline Vector3 Centroid() const
		{
			return 0.5f * (mptMin + mptMax);
		}

		inline Vector3 Offset(const Vector3& pt) const
		{
			return Vector3((pt.x - mptMin.x) / (mptMax.x - mptMin.x),
				(pt.y - mptMin.y) / (mptMax.y - mptMin.y),
				(pt.z - mptMin.z) / (mptMax.z - mptMin.z));
		}

		inline float Area() const
		{
			Vector3 vDiag = mptMax - mptMin;
			return 2.0f * (vDiag.x * vDiag.y + vDiag.y * vDiag.z + vDiag.x * vDiag.z);
		}

		inline float Volume() const
		{
			Vector3 vDiag = mptMax - mptMin;
			return vDiag.x * vDiag.y * vDiag.z;
		}

		inline void BoundingSphere(Vector3* pvCenter, float* pfRadius)
		{
			*pvCenter = 0.5f * (mptMin + mptMax);
			*pfRadius = Inside(*pvCenter) ? Math::Distance(*pvCenter, mptMax) : 0.0f;
		}

		inline void Expand(const float fDelta)
		{
			mptMin -= Vector3(fDelta);
			mptMax += Vector3(fDelta);
		}

		inline bool operator == (const BoundingBox& bbox)
		{
			return mptMin == bbox.mptMin && mptMax == bbox.mptMax;
		}

		inline bool operator != (const BoundingBox& bbox)
		{
			return mptMin != bbox.mptMin || mptMax != bbox.mptMax;
		}

		inline const Vector3& operator [] (int i) const
		{
			assert(i == 0 || i == 1);
			return (&mptMin)[i];
		}

		inline Vector3 operator [] (int i)
		{
			assert(i == 0 || i == 1);
			return (&mptMin)[i];
		}

		void RenderInGL() const;
	};

	namespace Math
	{
		inline BoundingBox Union(const BoundingBox& bbox1, const BoundingBox& bbox2)
		{
			BoundingBox ret;

			ret.mptMin.x = Math::Min(bbox1.mptMin.x, bbox2.mptMin.x);
			ret.mptMin.y = Math::Min(bbox1.mptMin.y, bbox2.mptMin.y);
			ret.mptMin.z = Math::Min(bbox1.mptMin.z, bbox2.mptMin.z);
			ret.mptMax.x = Math::Max(bbox1.mptMax.x, bbox2.mptMax.x);
			ret.mptMax.y = Math::Max(bbox1.mptMax.y, bbox2.mptMax.y);
			ret.mptMax.z = Math::Max(bbox1.mptMax.z, bbox2.mptMax.z);

			return ret;
		}

		inline BoundingBox Union(const BoundingBox& bbox, const Vector3& point)
		{
			BoundingBox ret;

			ret.mptMin.x = Math::Min(bbox.mptMin.x, point.x);
			ret.mptMin.y = Math::Min(bbox.mptMin.y, point.y);
			ret.mptMin.z = Math::Min(bbox.mptMin.z, point.z);
			ret.mptMax.x = Math::Max(bbox.mptMax.x, point.x);
			ret.mptMax.y = Math::Max(bbox.mptMax.y, point.y);
			ret.mptMax.z = Math::Max(bbox.mptMax.z, point.z);

			return ret;
		}
	}
}