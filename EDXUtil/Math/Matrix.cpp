#include "Matrix.h"
#include "EDXMath.h"
#include "../Core/Template.h"
#include "../Core/Memory.h"

namespace EDX
{
	const Matrix Matrix::IDENTITY = Matrix();

	Matrix::Matrix(float mat[4][4])
	{
		Memory::Memcpy(m, mat, 16 *sizeof(float));
	}


	Matrix::Matrix( float t00, float t01, float t02, float t03,
					float t10, float t11, float t12, float t13,
					float t20, float t21, float t22, float t23,
					float t30, float t31, float t32, float t33)
	{
		m[0][0] = t00; m[0][1] = t01; m[0][2] = t02; m[0][3] = t03;
		m[1][0] = t10; m[1][1] = t11; m[1][2] = t12; m[1][3] = t13;
		m[2][0] = t20; m[2][1] = t21; m[2][2] = t22; m[2][3] = t23;
		m[3][0] = t30; m[3][1] = t31; m[3][2] = t32; m[3][3] = t33;
	}


	Matrix Matrix::Transpose(const Matrix& m)
	{
		return Matrix(m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
			m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1],
			m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2],
			m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]);
	}


	Matrix Matrix::Inverse(const Matrix& m)
	{
		int indxc[4], indxr[4];
		int ipiv[4] = { 0, 0, 0, 0 };
		float minv[4][4];
		Memory::Memcpy(minv, m.m, 4 * 4 * sizeof(float));
		for (int i = 0; i < 4; i++)
		{
			int irow = -1, icol = -1;
			float big = 0.0f;
			for (int j = 0; j < 4; j++)
			{
				if (ipiv[j] != 1)
				{
					for (int k = 0; k < 4; k++)
					{
						if (ipiv[k] == 0)
						{
							if (fabsf(minv[j][k]) >= big)
							{
								big = float(fabsf(minv[j][k]));
								irow = j;
								icol = k;
							}
						}
						Assert(ipiv[k] <= 1);
					}
				}
			}
			++ipiv[icol];
			if (irow != icol)
			{
				for (int k = 0; k < 4; ++k)
					Swap(minv[irow][k], minv[icol][k]);
			}
			indxr[i] = irow;
			indxc[i] = icol;
			Assert(minv[icol][icol] != 0.0f);

			float pivinv = 1.f / minv[icol][icol];
			minv[icol][icol] = 1.f;
			for (int j = 0; j < 4; j++)
				minv[icol][j] *= pivinv;

			for (int j = 0; j < 4; j++)
			{
				if (j != icol)
				{
					float save = minv[j][icol];
					minv[j][icol] = 0;
					for (int k = 0; k < 4; k++)
						minv[j][k] -= minv[icol][k] * save;
				}
			}
		}
		for (int j = 3; j >= 0; j--)
		{
			if (indxr[j] != indxc[j])
			{
				for (int k = 0; k < 4; k++)
					Swap(minv[k][indxr[j]], minv[k][indxc[j]]);
			}
		}
		return Matrix(minv);
	}

	Matrix Matrix::Translate(const Vector3& vDelta)
	{
		Matrix mat(1, 0, 0, vDelta.x,
					0, 1, 0, vDelta.y,
					0, 0, 1, vDelta.z,
					0, 0, 0,        1);
		return mat;
	}

	Matrix Matrix::Scale(float fX, float fY, float fZ)
	{
		Matrix mat(fX, 0, 0, 0,
					0, fY, 0, 0,
					0, 0, fZ, 0,
					0, 0, 0, 1);

		return mat;
	}

	Matrix Matrix::Rotate(float fAngle, const Vector3& vAxis)
	{
		Vector3 a = Math::Normalize(vAxis);
		float s = Math::Sin(Math::ToRadians(fAngle));
		float c = Math::Cos(Math::ToRadians(fAngle));
		float m[4][4];

		m[0][0] = a.x * a.x + (1.f - a.x * a.x) * c;
		m[0][1] = a.x * a.y * (1.f - c) - a.z * s;
		m[0][2] = a.x * a.z * (1.f - c) + a.y * s;
		m[0][3] = 0;

		m[1][0] = a.x * a.y * (1.f - c) + a.z * s;
		m[1][1] = a.y * a.y + (1.f - a.y * a.y) * c;
		m[1][2] = a.y * a.z * (1.f - c) - a.x * s;
		m[1][3] = 0;

		m[2][0] = a.x * a.z * (1.f - c) - a.y * s;
		m[2][1] = a.y * a.z * (1.f - c) + a.x * s;
		m[2][2] = a.z * a.z + (1.f - a.z * a.z) * c;
		m[2][3] = 0;

		m[3][0] = 0;
		m[3][1] = 0;
		m[3][2] = 0;
		m[3][3] = 1;

		return Matrix(m);
	}

	Matrix Matrix::LookAt(const Vector3& ptEye, const Vector3& ptTarget, const Vector3& vUp)
	{
		float mat[4][4];

		mat[0][3] = ptEye.x;
		mat[1][3] = ptEye.y;
		mat[2][3] = ptEye.z;
		mat[3][3] = 1.0f;

		Vector3 vDir = Math::Normalize(ptTarget - ptEye);
		Vector3 vRight = Math::Normalize(Math::Cross(Math::Normalize(vUp), vDir));
		Vector3 vNewUp = Math::Cross(vDir, vRight);
		mat[0][0] = vRight.x;
		mat[1][0] = vRight.y;
		mat[2][0] = vRight.z;
		mat[3][0] = 0.0f;
		mat[0][1] = vNewUp.x;
		mat[1][1] = vNewUp.y;
		mat[2][1] = vNewUp.z;
		mat[3][1] = 0.0f;
		mat[0][2] = vDir.x;
		mat[1][2] = vDir.y;
		mat[2][2] = vDir.z;
		mat[3][2] = 0.0f;

		Matrix mCamToWorld(mat);

		return Inverse(mCamToWorld);
	}

	Matrix Matrix::Perspective(float fFov, float fRatio, float fNear, float fFar)
	{
		Matrix mPersp = Matrix(	1, 0,						  0,						   0,
								0, 1,						  0,						   0,
								0, 0,		fFar / (fFar-fNear),  -fFar*fNear / (fFar-fNear),
								0, 0,						  1,						   0);

		float fInvTanAng = 1.f / Math::Tan(Math::ToRadians(fFov) / 2.0f);
		return Mul(Scale(fInvTanAng / fRatio, fInvTanAng, 1), Matrix(mPersp));
	}

	Matrix Matrix::YawPitchRow(float fYaw, float fPitch, float fRoll)
	{
		float afQuaternion[4];

		float num9 = Math::ToRadians(fRoll) * 0.5f;
		float num6 = Math::Sin(num9);
		float num5 = Math::Cos(num9);
		float num8 = Math::ToRadians(fPitch) * 0.5f;
		float num4 = Math::Sin(num8);
		float num3 = Math::Cos(num8);
		float num7 = Math::ToRadians(fYaw) * 0.5f;
		float num2 = Math::Sin(num7);
		float num = Math::Cos(num7);
		afQuaternion[0] = ((num * num4) * num5) + ((num2 * num3) * num6);
		afQuaternion[1] = ((num2 * num3) * num5) - ((num * num4) * num6);
		afQuaternion[2] = ((num * num3) * num6) - ((num2 * num4) * num5);
		afQuaternion[3] = ((num * num3) * num5) + ((num2 * num4) * num6);

		num9 = afQuaternion[0] * afQuaternion[0];
		num8 = afQuaternion[1] * afQuaternion[1];
		num7 = afQuaternion[2] * afQuaternion[2];
		num6 = afQuaternion[0] * afQuaternion[1];
		num5 = afQuaternion[2] * afQuaternion[3];
		num4 = afQuaternion[2] * afQuaternion[0];
		num3 = afQuaternion[1] * afQuaternion[3];
		num2 = afQuaternion[1] * afQuaternion[2];
		num = afQuaternion[0] * afQuaternion[3];

		float m[4][4];
		m[0][0] = 1.0f - (2.0f * (num8 + num7));
		m[1][0] = 2.0f * (num6 + num5);
		m[2][0] = 2.0f * (num4 - num3);
		m[3][0] = 0.0f;
		m[0][1] = 2.0f * (num6 - num5);
		m[1][1] = 1.0f - (2.0f * (num7 + num9));
		m[2][1] = 2.0f * (num2 + num);
		m[3][1] = 0.0f;
		m[0][2] = 2.0f * (num4 + num3);
		m[1][2] = 2.0f * (num2 - num);
		m[2][2] = 1.0f - (2.0f * (num8 + num9));
		m[3][2] = 0.0f;
		m[0][3] = 0.0f;
		m[1][3] = 0.0f;
		m[2][3] = 0.0f;
		m[3][3] = 1.0f;

		return Matrix(m);
	}
	
	void Matrix::CalcTransform(const Vector3& vPos, const Vector3 vScl, const Vector3& vRot, Matrix* pmWorld, Matrix* pmWorldInv)
	{
		// Calculate the world matrix of the instance, translation
		*pmWorld = Matrix::Translate(Vector3(vPos));
		// Scaling
		*pmWorld = Matrix::Mul(*pmWorld, Matrix::Scale(vScl.x, vScl.y, vScl.z));
		// Rotation
		*pmWorld = Matrix::Mul(*pmWorld, Matrix::YawPitchRow(vRot.y, vRot.x, vRot.z));
		// Calculate the world to object matrix
		*pmWorldInv = Matrix::Inverse(*pmWorld);
	}
}