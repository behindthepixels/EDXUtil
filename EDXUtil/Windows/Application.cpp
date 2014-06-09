#include "Application.h"
#include "Window.h"

namespace EDX
{
	HINSTANCE Application::InstHandle = NULL;
	const wchar_t* Application::WinClassName = L"EDXWinClass";
	Window* Application::pPrimaryWindow = nullptr;

	int Application::Run(Window* pWindow)
	{
		HWND hWnd = pWindow->GetHandle();
		pPrimaryWindow = pWindow;
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);

		pPrimaryWindow->mInitializeEvent.Invoke(pPrimaryWindow, EventArgs());

		MSG msg;
		while (true)
		{
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message != WM_QUIT)
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
					break;
			}
			else if (pWindow->IsActive())
			{
				pPrimaryWindow->InvokeMainLoop();
			}
		}

		return (int)msg.wParam;
	}
}