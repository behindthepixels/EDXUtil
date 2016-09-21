#pragma once

#include "../Core/Types.h"
#include "Base.h"

namespace EDX
{
	class Window;

	class Application : public Object
	{
	private:
		static HINSTANCE InstHandle;
		static Window* pMainWindow;
		static char BaseDirectory[MAX_PATH];

	public:
		static const wchar_t* WinClassName;

	public:
		static void Init(HINSTANCE inst) { InstHandle = inst; GetCurrentDirectoryA(MAX_PATH, BaseDirectory); }
		static HINSTANCE GetInstanceHandle() { return InstHandle; }
		static Window* GetMainWindow() { return pMainWindow; }
		static const char* GetBaseDirectory() { return BaseDirectory; }

		static int Run(Window* pWindow);
		static void Dispose();
	};
}