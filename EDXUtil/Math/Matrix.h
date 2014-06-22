#pragma once

#include "Vec3.h"
#include "Vec4.h"
#include "EDXMath.h"
#include "Ray.h"
#include "BoundingBox.h"

#define MAT_IDENTITY EDX::Matrix()

namespace EDX
{
	class Matrix
	{
	public:
		float m[4][4];

	public:
		Matrix()
		{
			m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.f;
			m[0][1] = m[0][2] = m[0][3] = m[1][0] =
				m[1][2] = m[1][3] = m[2][0] = m[2][1] =
				m[2][3] = m[3][0] = m[3][1] = m[3][2] = 0.f;
		}
		Matrix(float mat[4][4]);

		Matrix(float fM00, float fM01, float fM02, float fM03,
			float fM10, float fM11, float fM12, float fM13,
			float fM20, float fM21, float fM22, float fM23,
			float fM30, float fM31, float fM32, float fM33);

		~Matrix()
		{
		}

		bool operator == (const Matrix& mMat) const
		{
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					if (m[i][j] != mMat.m[i][j])
						return false;
			return true;
		}
		bool operator != (const Matrix& mMat) const
		{
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					if (m[i][j] != mMat.m[i][j])
						return true;
			return false;
		}

		bool IsIdentity() const
		{
			return (m[0][0] == 1.f && m[0][1] == 0.f &&
				m[0][2] == 0.f && m[0][3] == 0.f &&
				m[1][0] == 0.f && m[1][1] == 1.f &&
				m[1][2] == 0.f && m[1][3] == 0.f &&
				m[2][0] == 0.f && m[2][1] == 0.f &&
				m[2][2] == 1.f && m[2][3] == 0.f &&
				m[3][0] == 0.f && m[3][1] == 0.f &&
				m[3][2] == 0.f && m[3][3] == 1.f);
		}

		static Matrix Mul(const Matrix& mMat1, const Matrix& mMat2)
		{
			Matrix r;
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					r.m[i][j] =
						mMat1.m[i][0] * mMat2.m[0][j] +
						mMat1.m[i][1] * mMat2.m[1][j] +
						mMat1.m[i][2] * mMat2.m[2][j] +
						mMat1.m[i][3] * mMat2.m[3][j];
				}
			}
			return r;
		}

		static Matrix Transpose(const Matrix& mMat);
		static Matrix Inverse(const Matrix& mMat);
		static Matrix Translate(const Vector3& vDelta);
		static Matrix Scale(float fX, float fY, float fZ);
		static Matrix Rotate(float fAngle, const Vector3& vAxis);
		static Matrix LookAt(const Vector3& ptEye, const Vector3& ptTarget, const Vector3& vUp);
		static Matrix Perspective(float fFov, float fRatio, float fNearClip, float fFarClip);
		static Matrix YawPitchRow(float fYaw, float fPitch, float fRoll);
		static void CalcTransform(const Vector3& vPos, const Vector3 vScl, const Vector3& vRot, Matrix* pmWorld, Matrix* pmWorldInv);

		static inline Vector3 TransformPoint(const Vector3& ptPt, const Matrix& mMat);
		static inline Vector4 TransformPoint(const Vector4& ptPt, const Matrix& mMat);
		static inline Vector3 TransformVector(const Vector3& vVec, const Matrix& mMat);
		static inline Vector3 TransformNormal(const Vector3& nNorm, const Matrix& mMat);
		static inline Ray TransformRay(const Ray& rRay, const Matrix& mMat);
		static inline RayDifferential TransformRayDiff(const RayDifferential& rRay, const Matrix& mMat);
		static inline BoundingBox TransformBBox(const BoundingBox& bbox, const Matrix& mMat);

		static const Matrix IDENTITY;
	};

	inline Matrix operator * (const Matrix& mMat1, const Matrix& mMat2)
	{
		return Matrix::Mul(mMat1, mMat2);
	}

	inline Vector3 Matrix::TransformPoint(const Vector3& ptPt, const Matrix& mMat)
	{
		float x = ptPt.x, y = ptPt.y, z = ptPt.z;
		float xp = mMat.m[0][0] * x + mMat.m[0][1] * y + mMat.m[0][2] * z + mMat.m[0][3];
		float yp = mMat.m[1][0] * x + mMat.m[1][1] * y + mMat.m[1][2] * z + mMat.m[1][3];
		float zp = mMat.m[2][0] * x + mMat.m[2][1] * y + mMat.m[2][2] * z + mMat.m[2][3];
		float wp = mMat.m[3][0] * x + mMat.m[3][1] * y + mMat.m[3][2] * z + mMat.m[3][3];
		assert(wp != 0);
		if (wp == 1.0f)
			return Vector3(xp, yp, zp);
		else
			return Vector3(xp, yp, zp) / wp;
	}

	inline Vector4 Matrix::TransformPoint(const Vector4& ptPt, const Matrix& mMat)
	{
		float x = ptPt.x, y = ptPt.y, z = ptPt.z, w = ptPt.w;
		float xp = mMat.m[0][0] * x + mMat.m[0][1] * y + mMat.m[0][2] * z + mMat.m[0][3] * w;
		float yp = mMat.m[1][0] * x + mMat.m[1][1] * y + mMat.m[1][2] * z + mMat.m[1][3] * w;
		float zp = mMat.m[2][0] * x + mMat.m[2][1] * y + mMat.m[2][2] * z + mMat.m[2][3] * w;
		float wp = mMat.m[3][0] * x + mMat.m[3][1] * y + mMat.m[3][2] * z + mMat.m[3][3] * w;

		return Vector4(xp, yp, zp, wp);
	}

	inline Vector3 Matrix::TransformVector(const Vector3& vVec, const Matrix& mMat)
	{
		float x = vVec.x, y = vVec.y, z = vVec.z;
		return Vector3(mMat.m[0][0] * x + mMat.m[0][1] * y + mMat.m[0][2] * z,
			mMat.m[1][0] * x + mMat.m[1][1] * y + mMat.m[1][2] * z,
			mMat.m[2][0] * x + mMat.m[2][1] * y + mMat.m[2][2] * z);
	}

	inline Vector3 Matrix::TransformNormal(const Vector3& nNorm, const Matrix& mInv)
	{
		float x = nNorm.x, y = nNorm.y, z = nNorm.z;
		//Matrix mInv = Inverse(mMat);
		return Vector3(mInv.m[0][0] * x + mInv.m[1][0] * y + mInv.m[2][0] * z,
			mInv.m[0][1] * x + mInv.m[1][1] * y + mInv.m[2][1] * z,
			mInv.m[0][2] * x + mInv.m[1][2] * y + mInv.m[2][2] * z);
	}

	inline Ray Matrix::TransformRay(const Ray& rRay, const Matrix& mMat)
	{
		Ray ray = rRay;
		ray.mptOrg = TransformPoint(ray.mptOrg, mMat);
		ray.mvDir = TransformVector(ray.mvDir, mMat);

		return ray;
	}

	inline RayDifferential Matrix::TransformRayDiff(const RayDifferential& rRay, const Matrix& mMat)
	{
		RayDifferential ray = RayDifferential(TransformRay(Ray(rRay), mMat));
		ray.mvDxOrg = TransformPoint(rRay.mvDxOrg, mMat);
		ray.mvDyOrg = TransformPoint(rRay.mvDyOrg, mMat);
		ray.mvDxDir = TransformVector(rRay.mvDxDir, mMat);
		ray.mvDyDir = TransformVector(rRay.mvDyDir, mMat);
		ray.mbHasDifferential = rRay.mbHasDifferential;

		return ray;
	}

	inline BoundingBox Matrix::TransformBBox(const BoundingBox& bbox, const Matrix& mMat)
	{
		BoundingBox boxRet(Matrix::TransformPoint(Vector3(bbox.mptMin.x, bbox.mptMin.y, bbox.mptMin.z), mMat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mptMax.x, bbox.mptMin.y, bbox.mptMin.z), mMat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mptMin.x, bbox.mptMax.y, bbox.mptMin.z), mMat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mptMax.x, bbox.mptMax.y, bbox.mptMin.z), mMat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mptMin.x, bbox.mptMin.y, bbox.mptMax.z), mMat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mptMax.x, bbox.mptMin.y, bbox.mptMax.z), mMat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mptMin.x, bbox.mptMax.y, bbox.mptMax.z), mMat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mptMax.x, bbox.mptMax.y, bbox.mptMax.z), mMat));

		return boxRet;
	}
}