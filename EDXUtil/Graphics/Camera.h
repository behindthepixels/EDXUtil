#pragma once

#include "../Windows/Base.h"
#include "../Windows/Event.h"
#include "../Math/Matrix.h"
#include "../Windows/Timer.h"

namespace EDX
{
	class Camera
	{
	public:
		// Camera parameters
		Vector3 mPos;
		Vector3 mTarget;
		Vector3 mUp;
		Vector3 mDir;

		// First person movement params
		float mMoveScaler;
		float mRotateScaler;
		float mYaw, mPitch;

		float mFOV;
		float mRatio;
		float mNearClip;
		float mFarClip;

	protected:
		float mFOV_2;

		// Screen resolution
		int mFilmResX;
		int mFilmResY;

		// Matrices
		Matrix mView;
		Matrix mViewInv;

		Matrix mProj;

		Matrix mScreenToRaster;
		Matrix mRasterToCamera;
		Matrix mCameraToRaster;
		Matrix mRasterToWorld;
		Matrix mWorldToRaster;

		// User input
		Vector3 mMovementVelocity;
		Vector3 mMovementImpulse;
		Vector2 mRotateVelocity;
		Timer mTimer;

	public:
		Camera();
		Camera(const Vector3& ptPos, const Vector3& ptTar, const Vector3& vUp, int iResX, int iResY,
			float fFOV = 35.0f, float fNear = 0.1f, float fFar = 1000.0f);

		virtual ~Camera(void)
		{
		}

		virtual void Init(const Vector3& ptPos, const Vector3& ptTar, const Vector3& vUp, int iResX, int iResY,
			float fFOV = 35.0f, float fNear = 0.1f, float fFar = 1000.0f);

		// Handling the resize event
		virtual void Resize(int width, int height);

		// Getters
		const Matrix& GetViewMatrix() const { return mView; }
		const Matrix& GetViewInvMatrix() const { return mViewInv; }
		const Matrix& GetProjMatrix() const { return mProj; }
		const Matrix& GetRasterMatrix() const { return mScreenToRaster; }

		// Hanlding the first person movement msg
		void HandleMouseMsg(const MouseEventArgs& args);
		void HandleKeyboardMsg(const KeyboardEventArgs& args);
		void Transform();

		// Given a point in world space, return the raster coordinate
		inline Vector3 WorldToRaster(const Vector3 ptWorld) const { return Matrix::TransformPoint(ptWorld, mWorldToRaster); }
		inline Vector3 RasterToWorld(const Vector3 ptRas) const { return Matrix::TransformPoint(ptRas, mRasterToWorld); }
		inline Vector3 RasterToCamera(const Vector3 ptRas) const { return Matrix::TransformPoint(ptRas, mRasterToCamera); }
		inline Vector3 CameraToRaster(const Vector3 ptCam) const { return Matrix::TransformPoint(ptCam, mCameraToRaster); }
		inline bool CheckRaster(const Vector3& ptRas) const { return ptRas.x < float(mFilmResX) && ptRas.x >= 0.0f && ptRas.y < float(mFilmResY) && ptRas.y >= 0.0f; }

		int GetFilmSizeX() const { return mFilmResX; }
		int GetFilmSizeY() const { return mFilmResY; }
	};
}