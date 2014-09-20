#include "Windows/Window.h"
#include "Windows/Application.h"
#include "Graphics/EDXGui.h"

#include "Graphics/OpenGL.h"

using namespace EDX;
using namespace EDX::GUI;

EDXDialog		gDialog;

void OnInit(Object* pSender, EventArgs args)
{
	glClearColor(0.4f, 0.5f, 0.65f, 1.0f);
	gDialog.Init(1280, 800);

	gDialog.AddText(0, "GUI Test");
	gDialog.AddCheckBox(1, true, "Check Box");
	gDialog.AddButton(2, "Button");
	gDialog.AddSlider(2, 0.0f, 1.0f, 0.5f);
}

void OnRender(Object* pSender, EventArgs args)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gDialog.Render();
}

void OnResize(Object* pSender, ResizeEventArgs args)
{
	glViewport(0, 0, args.Width, args.Height);

	gDialog.Resize(args.Width, args.Height);
}

void OnMouseEvent(Object* pSender, MouseEventArgs args)
{
	if (gDialog.MsgProc(args))
		return;
}

void OnRelease(Object* pSender, EventArgs args)
{
	gDialog.Release();
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdArgs, int cmdShow)
{
	Application::Init(hInst);

	Window* mainWindow = new GLWindow;
	mainWindow->SetMainLoop(NotifyEvent(OnRender));
	mainWindow->SetInit(NotifyEvent(OnInit));
	mainWindow->SetResize(ResizeEvent(OnResize));
	mainWindow->SetMouseHandler(MouseEvent(OnMouseEvent));
	mainWindow->SetRelease(NotifyEvent(OnRelease));

	mainWindow->Create(L"EDXGui", 1280, 800);

	Application::Run(mainWindow);

	return 0;
}