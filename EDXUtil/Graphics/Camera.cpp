#include "Camera.h"

namespace EDX
{
	Camera::Camera()
	{
		Init(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), 1024, 640, float(Math::EDX_PI_4), 1.0f, 1000.0f);
	}

	Camera::Camera(const Vector3& vPos, const Vector3& vTar, const Vector3& vUp, int iResX, int iResY, float fFOV, float fNear, float fFar)
	{
		Init(vPos, vTar, vUp, iResX, iResY, fFOV, fNear, fFar);
	}

	void Camera::Init(const Vector3& vPos, const Vector3& vTar, const Vector3& vUp, int iResX, int iResY,
		float fFOV, float fNear, float fFar)
	{
		mPos = vPos;
		mTarget = vTar;
		mUp = vUp;
		mFilmResX = iResX;
		mFilmResY = iResY;
		mFOV = fFOV;
		mNearClip = fNear;
		mFarClip = fFar;

		mMoveScaler = 0.25f;
		mRotateScaler = 0.03f;

		mRatio = mFilmResX / float(mFilmResY);
		mFOV_2 = mFOV / 2.0f;

		mDir = Math::Normalize(mTarget - mPos);

		mYaw = Math::Atan2(mDir.x, mDir.z);
		mPitch = Math::Atan2(mDir.y, Math::Sqrt(mDir.x * mDir.x + mDir.z * mDir.z));

		mView = Matrix::LookAt(mPos, mTarget, mUp);
		mViewInv = Matrix::Inverse(mView);
		mProj = Matrix::Perspective(mFOV, mRatio, mNearClip, mFarClip);
		mScreenToRaster = Matrix::Scale(float(mFilmResX), float(mFilmResY), 1.0f) *
			Matrix::Scale(0.5f, -0.5f, 1.0f) *
			Matrix::Translate(Vector3(1.0f, -1.0f, 0.0f));
		mRasterToCamera = Matrix::Mul(Matrix::Inverse(mProj), Matrix::Inverse(mScreenToRaster));
		mCameraToRaster = Matrix::Inverse(mRasterToCamera);
		mRasterToWorld = Matrix::Mul(mViewInv, mRasterToCamera);
		mWorldToRaster = Matrix::Inverse(mRasterToWorld);

		Transform();
	}

	void Camera::Resize(int iWidth, int iHeight)
	{
		mFilmResX = iWidth;
		mFilmResY = iHeight;

		mRatio = mFilmResX / float(mFilmResY);

		mProj = Matrix::Perspective(mFOV, mRatio, mNearClip, mFarClip);
		mScreenToRaster = Matrix::Scale(float(mFilmResX), float(mFilmResY), 1.0f) *
			Matrix::Scale(0.5f, -0.5f, 1.0f) *
			Matrix::Translate(Vector3(1.0f, -1.0f, 0.0f));
		mRasterToCamera = Matrix::Mul(Matrix::Inverse(mProj), Matrix::Inverse(mScreenToRaster));
		mCameraToRaster = Matrix::Inverse(mRasterToCamera);
		mRasterToWorld = Matrix::Mul(mViewInv, mRasterToCamera);
		mWorldToRaster = Matrix::Inverse(mRasterToWorld);
	}

	void Camera::Transform()
	{
		Matrix mCamRotate = Matrix::YawPitchRow(Math::ToDegrees(mYaw), Math::ToDegrees(mPitch), 0.0f);

		Vector3 vWorldUp, vWorldAhead;
		Vector3 vLocalUp = Vector3::UNIT_Y, vLocalAhead = Vector3::UNIT_Z;

		vWorldUp = Matrix::Transformector(vLocalUp, mCamRotate);
		vWorldAhead = Matrix::Transformector(vLocalAhead, mCamRotate);

		Vector3 vVelocity = Vector3::ZERO;
		if (Math::Length(mDirKB) > 0.0f)
		{
			vVelocity = Math::Normalize(mDirKB) * mMoveScaler;
		}

		Vector3 vWorldPosDelta = Matrix::Transformector(vVelocity, mCamRotate);

		mPos += vWorldPosDelta;
		mTarget = mPos + vWorldAhead;
		mDir = Math::Normalize(mTarget - mPos);
		mUp = vWorldUp;

		mView = Matrix::LookAt(mPos, mTarget, mUp);
		mViewInv = Matrix::Inverse(mView);
		mRasterToWorld = Matrix::Mul(mViewInv, mRasterToCamera);
		mWorldToRaster = Matrix::Inverse(mRasterToWorld);
	}

	bool Camera::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		static POINTS ptOldMousePos;
		static bool bIsLBtnDown = false;
		mDirKB = Vector3::ZERO;
		bool bViewChanged = false;

		switch (msg)
		{
		case WM_LBUTTONDOWN:
			ptOldMousePos = MAKEPOINTS(lParam);
			bIsLBtnDown = true;
			break;
		case WM_LBUTTONUP:
			ptOldMousePos = MAKEPOINTS(lParam);
			bIsLBtnDown = false;
			break;
		case WM_MOUSEMOVE:
			if (bIsLBtnDown)
			{
				POINTS ptCurMousePos;
				ptCurMousePos = MAKEPOINTS(lParam);

				Vector3 vMouseDelta;
				vMouseDelta.x = float(ptCurMousePos.x - ptOldMousePos.x);
				vMouseDelta.y = float(ptCurMousePos.y - ptOldMousePos.y);

				ptOldMousePos = ptCurMousePos;

				Vector3 vRotateVel = vMouseDelta * mRotateScaler;
				mYaw -= -vRotateVel.x;
				mPitch -= -vRotateVel.y;

				mPitch = Math::Max(mPitch, -180);
				mPitch = Math::Min(mPitch, float(Math::EDX_PI_2));

				bViewChanged = true;
			}
			break;
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
			case 'W':
				mDirKB.z -= 1.0f;
				bViewChanged = true;
				break;
			case 'S':
				mDirKB.z += 1.0f;
				bViewChanged = true;
				break;
			case 'A':
				mDirKB.x += 1.0f;
				bViewChanged = true;
				break;
			case 'D':
				mDirKB.x -= 1.0f;
				bViewChanged = true;
				break;
			case 'Q':
				mDirKB.y -= 1.0f;
				bViewChanged = true;
				break;
			case 'E':
				mDirKB.y += 1.0f;
				bViewChanged = true;
				break;
			}
		}
			break;
		}

		Transform();

		return bViewChanged;
	}
}