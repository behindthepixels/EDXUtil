#pragma once

#include "../EDXPrerequisites.h"
#include "Base.h"

namespace EDX
{
	class Window;

	class Application : public Object
	{
	private:
		static HINSTANCE InstHandle;
		static Window* pMainWindow;

	public:
		static const wchar_t* WinClassName;

	public:
		static void Init(HINSTANCE inst) { InstHandle = inst; }
		static HINSTANCE GetInstanceHandle() { return InstHandle; }
		static Window* GetMainWindow() { return pMainWindow; }

		static int Run(Window* pWindow);
		static void Dispose();
	};
}