#include "Windows/Window.h"
#include "Windows/Application.h"
#include "Graphics/EDXGui.h"
#include "Windows/Bitmap.h"

#include "Graphics/OpenGL.h"

using namespace EDX;
using namespace EDX::GUI;

EDXDialog		gDialog;

double PI = 3.141592653f;
double mAmp = 250;
float mPhase = 0;
double mPeriod = 500;
double mConst = 500;
double mTotalPeriod = 500;
float mStraightLength = 0;
float mHalfRatio = 0.5;
bool mMod;

float gSlide;
bool gChecked;
int gCombo;

_byte* gImage;
int gWidth = 0, gHeight = 0;

void OnInit(Object* pSender, EventArgs args)
{
	GL::LoadGLExtensions();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	gDialog.Init(1280, 800);

	gDialog.AddText(0, "GUI Test");
	gDialog.AddCheckBox(1, mMod, &mMod, "Check box");
	gDialog.AddCheckBox(1, mMod, &mMod, "Check box 2");
	gDialog.AddButton(2, "Button");
	gDialog.AddSlider(3, 0.0f, 6.28f, mPhase, &mPhase, "Slider 1: ");
	gDialog.AddSlider(3, 0.0f, 1.0f, mHalfRatio, &mHalfRatio, "Slider 2: ");
	gDialog.AddSlider(4, 0.0f, mTotalPeriod, mStraightLength, &mStraightLength, "Slider 3: ");

	ComboBoxItem items[] = {
			{ 0, "Item 1" },
			{ 1, "Item 2" },
			{ 2, "Item 3" },
			{ 3, "Item 4" },
			{ 4, "Item 5" },
	};

	gDialog.AddComboBox(4, 0, &gCombo, items, 5);
	gDialog.AddComboBox(5, 0, &gCombo, items, 5);

	gDialog.AddButton(2, "Button 2");

	int channel;
	string c = Application::GetBaseDirectory();
	c += "\\Sponza.jpg";
	gImage = Bitmap::ReadFromFileByte(c.c_str(), &gWidth, &gHeight, &channel);
	assert(gImage);
}

void OnRender(Object* pSender, EventArgs args)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRasterPos3f(0.0f, 0.0f, 1.0f);
	glDrawPixels(gWidth, gHeight, GL_RGBA, GL_UNSIGNED_BYTE, gImage);

	gDialog.Render();
}

void OnResize(Object* pSender, ResizeEventArgs args)
{
	glViewport(0, 0, args.Width, args.Height);

	// Set opengl params
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, args.Width, 0, args.Height, -1, 1);

	glMatrixMode(GL_MODELVIEW);

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