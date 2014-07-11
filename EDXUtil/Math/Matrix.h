#pragma once

#include "Vec2.h"
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

		bool operator == (const Matrix& mat) const
		{
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					if (m[i][j] != mat.m[i][j])
						return false;
			return true;
		}
		bool operator != (const Matrix& mat) const
		{
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					if (m[i][j] != mat.m[i][j])
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

		static Matrix Mul(const Matrix& mat1, const Matrix& mat2)
		{
			Matrix r;
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					r.m[i][j] =
						mat1.m[i][0] * mat2.m[0][j] +
						mat1.m[i][1] * mat2.m[1][j] +
						mat1.m[i][2] * mat2.m[2][j] +
						mat1.m[i][3] * mat2.m[3][j];
				}
			}
			return r;
		}

		static Matrix Transpose(const Matrix& mat);
		static Matrix Inverse(const Matrix& mat);
		static Matrix Translate(const Vector3& vDelta);
		static Matrix Scale(float fX, float fY, float fZ);
		static Matrix Rotate(float fAngle, const Vector3& vAxis);
		static Matrix LookAt(const Vector3& ptEye, const Vector3& ptTarget, const Vector3& vUp);
		static Matrix Perspective(float fFov, float fRatio, float fNearClip, float fFarClip);
		static Matrix YawPitchRow(float fYaw, float fPitch, float fRoll);
		static void CalcTransform(const Vector3& vPos, const Vector3 vScl, const Vector3& vRot, Matrix* pmWorld, Matrix* pmWorldInv);

		static inline Vector3 TransformPoint(const Vector3& ptPt, const Matrix& mat);
		static inline Vector4 TransformPoint(const Vector4& ptPt, const Matrix& mat);
		static inline Vector3 Transformector(const Vector3& vVec, const Matrix& mat);
		static inline Vector3 TransformNormal(const Vector3& nNorm, const Matrix& mat);
		static inline Ray TransformRay(const Ray& rRay, const Matrix& mat);
		static inline RayDifferential TransformRayDiff(const RayDifferential& rRay, const Matrix& mat);
		static inline BoundingBox TransformBBox(const BoundingBox& bbox, const Matrix& mat);

		static const Matrix IDENTITY;
	};

	inline Matrix operator * (const Matrix& mat1, const Matrix& mat2)
	{
		return Matrix::Mul(mat1, mat2);
	}

	inline Vector3 Matrix::TransformPoint(const Vector3& ptPt, const Matrix& mat)
	{
		float x = ptPt.x, y = ptPt.y, z = ptPt.z;
		float xp = mat.m[0][0] * x + mat.m[0][1] * y + mat.m[0][2] * z + mat.m[0][3];
		float yp = mat.m[1][0] * x + mat.m[1][1] * y + mat.m[1][2] * z + mat.m[1][3];
		float zp = mat.m[2][0] * x + mat.m[2][1] * y + mat.m[2][2] * z + mat.m[2][3];
		float wp = mat.m[3][0] * x + mat.m[3][1] * y + mat.m[3][2] * z + mat.m[3][3];
		assert(wp != 0);
		if (wp == 1.0f)
			return Vector3(xp, yp, zp);
		else
			return Vector3(xp, yp, zp) / wp;
	}

	inline Vector4 Matrix::TransformPoint(const Vector4& ptPt, const Matrix& mat)
	{
		float x = ptPt.x, y = ptPt.y, z = ptPt.z, w = ptPt.w;
		float xp = mat.m[0][0] * x + mat.m[0][1] * y + mat.m[0][2] * z + mat.m[0][3] * w;
		float yp = mat.m[1][0] * x + mat.m[1][1] * y + mat.m[1][2] * z + mat.m[1][3] * w;
		float zp = mat.m[2][0] * x + mat.m[2][1] * y + mat.m[2][2] * z + mat.m[2][3] * w;
		float wp = mat.m[3][0] * x + mat.m[3][1] * y + mat.m[3][2] * z + mat.m[3][3] * w;

		return Vector4(xp, yp, zp, wp);
	}

	inline Vector3 Matrix::Transformector(const Vector3& vVec, const Matrix& mat)
	{
		float x = vVec.x, y = vVec.y, z = vVec.z;
		return Vector3(mat.m[0][0] * x + mat.m[0][1] * y + mat.m[0][2] * z,
			mat.m[1][0] * x + mat.m[1][1] * y + mat.m[1][2] * z,
			mat.m[2][0] * x + mat.m[2][1] * y + mat.m[2][2] * z);
	}

	inline Vector3 Matrix::TransformNormal(const Vector3& nNorm, const Matrix& mnv)
	{
		float x = nNorm.x, y = nNorm.y, z = nNorm.z;
		//Matrix mnv = Inverse(mat);
		return Vector3(mnv.m[0][0] * x + mnv.m[1][0] * y + mnv.m[2][0] * z,
			mnv.m[0][1] * x + mnv.m[1][1] * y + mnv.m[2][1] * z,
			mnv.m[0][2] * x + mnv.m[1][2] * y + mnv.m[2][2] * z);
	}

	inline Ray Matrix::TransformRay(const Ray& rRay, const Matrix& mat)
	{
		Ray ray = rRay;
		ray.mOrg = TransformPoint(ray.mOrg, mat);
		ray.mDir = Transformector(ray.mDir, mat);

		return ray;
	}

	inline RayDifferential Matrix::TransformRayDiff(const RayDifferential& rRay, const Matrix& mat)
	{
		RayDifferential ray = RayDifferential(TransformRay(Ray(rRay), mat));
		ray.mDxOrg = TransformPoint(rRay.mDxOrg, mat);
		ray.mDyOrg = TransformPoint(rRay.mDyOrg, mat);
		ray.mDxDir = Transformector(rRay.mDxDir, mat);
		ray.mDyDir = Transformector(rRay.mDyDir, mat);
		ray.mbHasDifferential = rRay.mbHasDifferential;

		return ray;
	}

	inline BoundingBox Matrix::TransformBBox(const BoundingBox& bbox, const Matrix& mat)
	{
		BoundingBox boxRet(Matrix::TransformPoint(Vector3(bbox.mMin.x, bbox.mMin.y, bbox.mMin.z), mat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mMax.x, bbox.mMin.y, bbox.mMin.z), mat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mMin.x, bbox.mMax.y, bbox.mMin.z), mat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mMax.x, bbox.mMax.y, bbox.mMin.z), mat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mMin.x, bbox.mMin.y, bbox.mMax.z), mat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mMax.x, bbox.mMin.y, bbox.mMax.z), mat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mMin.x, bbox.mMax.y, bbox.mMax.z), mat));
		boxRet = Math::Union(boxRet, Matrix::TransformPoint(Vector3(bbox.mMax.x, bbox.mMax.y, bbox.mMax.z), mat));

		return boxRet;
	}
}