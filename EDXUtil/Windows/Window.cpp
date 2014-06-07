#include "Window.h"
#include "Application.h"

namespace EDX
{
	const wchar_t* Application::WinClassName = L"EDXWinClass";

	bool Window::Create(const wstring& strTitle,
		const uint iResX,
		const uint iResY)
	{
		mstrTitle = strTitle;
		miResX = iResX;
		miResY = iResY;

		RegisterWindowClass();

		DWORD winStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		DWORD winStyleEX = WS_EX_CLIENTEDGE;
	}

	bool Window::RegisterWindowClass()
	{
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
		winClass.lpszClassName = Application::WinClassName;
		winClass.hIconSm = LoadIcon(0, IDI_APPLICATION);

		if (!::RegisterClassEx(&winClass))
			return false;

		return true;
	}
}