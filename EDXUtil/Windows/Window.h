#pragma once

#include "../EDXPrerequisites.h"
#include "Base.h"

#include <Windows.h>

namespace EDX
{
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	class Window : public Object
	{
	private:
		HWND mhWnd;
		uint miResX, miResY;
		wstring mstrTitle;

	public:
		Window()
		{
		}

		bool Create(const wstring& strTitle,
			const uint iResX,
			const uint iResY);

		bool RegisterWindowClass();
	};

	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		return 0;
	}

}