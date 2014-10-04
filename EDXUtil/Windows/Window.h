#pragma once

#include "../EDXPrerequisites.h"
#include "Base.h"
#include "Event.h"

namespace EDX
{
	class Window : public Object
	{
	protected:
		HWND mhWnd;
		uint mWidth, mHeight;
		wstring mstrTitle;
		bool mbActive;

		POINTS mMousePt, mMousePrevPt;
		bool mLBtnDown, mRBtnDown;

	public:
		NotifyEvent mMainLoopEvent;
		NotifyEvent mInitializeEvent;
		NotifyEvent mReleaseEvent;
		ResizeEvent	mResizeEvent;
		MouseEvent mMouseEvent;
		KeyboardEvent mKeyboardEvent;

	public:
		Window()
			: mLBtnDown(false)
			, mRBtnDown(false)
		{
		}

		virtual bool Create(const wstring& strTitle,
			const uint iWidth,
			const uint iHeight);

		virtual void Destroy();
		
		HWND GetHandle() const { return mhWnd; }

		bool IsActive() const { return mbActive; }
		void SetActive(const bool active) { mbActive = active; }
		const uint GetWindowWidth() const { return mWidth; }
		const uint GetWindowHeight() const { return mHeight; }

		virtual void InvokeMainLoop() { mMainLoopEvent.Invoke(this, EventArgs()); }

		// Event handlers
		void SetMainLoop(const NotifyEvent& mainLoopEvent) { mMainLoopEvent = mainLoopEvent; }
		void SetInit(const NotifyEvent& initEvent) { mInitializeEvent = initEvent; }
		void SetResize(const ResizeEvent& resizeEvent) { mResizeEvent = resizeEvent; }
		void SetRelease(const NotifyEvent& releaseEvent) { mReleaseEvent = releaseEvent; }
		void SetMouseHandler(const MouseEvent& mouseEvent) { mMouseEvent = mouseEvent; }
		void SetkeyboardHandler(const KeyboardEvent& keyboardEvent) { mKeyboardEvent = keyboardEvent; }

		bool OpenFileDialog(const char* initDir, const char* ext, const char* filter, char outPath[MAX_PATH]) const;

		virtual bool ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected:
		bool RegisterWindowClass();
	};

	class GLWindow : public Window
	{
	protected:
		HGLRC mhRC;
		HDC mhDC;

		bool Create(const wstring& strTitle,
			const uint iWidth,
			const uint iHeight);
		virtual void Destroy();

	public:
		void InvokeMainLoop() { mMainLoopEvent.Invoke(this, EventArgs()); SwapBuffers(mhDC); }
	};

}