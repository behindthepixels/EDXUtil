#include "Application.h"
#include "Window.h"

namespace EDX
{
	HINSTANCE Application::InstHandle = NULL;
	const wchar_t* Application::WinClassName = L"EDXWinClass";
	Window* Application::pMainWindow = nullptr;
	char Application::BaseDirectory[MAX_PATH] = "";

	int Application::Run(Window* pWindow)
	{
		pMainWindow = pWindow;
		HWND hWnd = pMainWindow->GetHandle();
		pMainWindow->mInitializeEvent.Invoke(pMainWindow, EventArgs());

		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);

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
				pMainWindow->InvokeMainLoop();
			}
		}

		pMainWindow->mReleaseEvent.Invoke(pMainWindow, EventArgs());
		Dispose();

		return (int)msg.wParam;
	}

	void Application::Dispose()
	{
		if (pMainWindow)
			delete pMainWindow;

		Assertf(!_CrtDumpMemoryLeaks(), "Memory leak detected. See debug output for details");
	}
}