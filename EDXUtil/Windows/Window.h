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
		uint miWidth, miHeight;
		wstring mstrTitle;
		bool mbActive;

	public:
		NotifyEvent mMainLoopEvent;
		NotifyEvent mInitializeEvent;
		NotifyEvent mReleaseEvent;
		ResizeEvent	mResizeEvent;
		MouseEvent mMouseEvent;

	public:
		Window()
		{
		}

		virtual bool Create(const wstring& strTitle,
			const uint iWidth,
			const uint iHeight);

		virtual void Destroy();
		
		HWND GetHandle() const { return mhWnd; }

		bool IsActive() const { return mbActive; }
		void SetActive(const bool active) { mbActive = active; }

		virtual void InvokeMainLoop() { mMainLoopEvent.Invoke(this, EventArgs()); }

		// Event handlers
		void SetMainLoop(const NotifyEvent& mainLoopEvent) { mMainLoopEvent = mainLoopEvent; }
		void SetInit(const NotifyEvent& initEvent) { mInitializeEvent = initEvent; }
		void SetResize(const ResizeEvent& resizeEvent) { mResizeEvent = resizeEvent; }
		void SetRelease(const NotifyEvent& releaseEvent) { mReleaseEvent = releaseEvent; }

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