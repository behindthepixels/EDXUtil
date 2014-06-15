#include "Application.h"
#include "Window.h"

namespace EDX
{
	HINSTANCE Application::InstHandle = NULL;
	const wchar_t* Application::WinClassName = L"EDXWinClass";
	Window* Application::pMainWindow = nullptr;

	int Application::Run(Window* pWindow)
	{
		HWND hWnd = pWindow->GetHandle();
		pMainWindow = pWindow;
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);

		pMainWindow->mInitializeEvent.Invoke(pMainWindow, EventArgs());

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

		Dispose();
		return (int)msg.wParam;
	}

	void Application::Dispose()
	{
		if (pMainWindow)
			delete pMainWindow;

		assert(!_CrtDumpMemoryLeaks());
	}
}