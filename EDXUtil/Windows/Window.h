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
			const uint iResY)
		{
			mstrTitle = strTitle;
			miResX = iResX;
			miResY = iResY;

			WNDCLASSEX winClass;
			winClass.cbSize = sizeof(WNDCLASSEX);
			winClass.style = CS_OWNDC;
			winClass.lpfnWndProc = WndProc;
			winClass.cbClsExtra = 0;
			winClass.cbWndExtra = 0;
			winClass.hInstance = (HINSTANCE)GetModuleHandle(NULL);
			winClass.hIcon = LoadIcon(0, IDI_APPLICATION);
			winClass.hCursor = LoadCursor(0, IDC_ARROW);
			winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
			winClass.lpszMenuName = 0;
			winClass.lpszClassName = L"EDXClass";
			winClass.hIconSm = LoadIcon(0, IDI_APPLICATION);
		}
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