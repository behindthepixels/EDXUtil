#pragma once

#include "../EDXPrerequisites.h"
#include "Base.h"

#include <Windows.h>

namespace EDX
{
	class Window;

	class Application : public Object
	{
	private:
		static HINSTANCE InstHandle;
		static Window* pPrimaryWindow;

	public:
		static const wchar_t* WinClassName;

	public:
		static void Init(HINSTANCE inst) { InstHandle = inst; }
		static HINSTANCE GetInstanceHandle() { return InstHandle; }
		static Window* GetPrimaryWindow() { return pPrimaryWindow; }

		static int Run(Window* pWindow);
		static void Dispose();
	};
}