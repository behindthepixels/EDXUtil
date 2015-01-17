#include "Windows/Window.h"
#include "Windows/Application.h"
#include "Graphics/EDXGui.h"
#include "Windows/Bitmap.h"

#include "Graphics/OpenGL.h"

using namespace EDX;
using namespace EDX::GUI;

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
	OpenGL::InitializeOpenGLExtensions();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);

	int channel;
	string c = Application::GetBaseDirectory();
	c += "\\Sponza.jpg";
	gImage = (_byte*)Bitmap::ReadFromFile<Color4b>(c.c_str(), &gWidth, &gHeight, &channel);
	assert(gImage);
}

void OnRender(Object* pSender, EventArgs args)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRasterPos3f(0.0f, 0.0f, 1.0f);
	glDrawPixels(gWidth, gHeight, GL_RGBA, GL_UNSIGNED_BYTE, gImage);

	EDXGui::BeginDialog(1280, 800);
	EDXGui::Text("Fuck you! %f", 1.0f);
	EDXGui::Text("Fuck you! %f", 2.0f);
	EDXGui::Text("Fuck you! %f", 2.0f);
	EDXGui::Text("Fuck you! %f", 4.0f);
	EDXGui::Text("Fuck you! %f", 236.0f);
	EDXGui::Text("Fuck you! %f", 2345.0f);
	EDXGui::EndDialog();
}

void OnResize(Object* pSender, ResizeEventArgs args)
{
	glViewport(0, 0, args.Width, args.Height);

	// Set opengl params
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, args.Width, 0, args.Height, -1, 1);

	glMatrixMode(GL_MODELVIEW);

}

void OnMouseEvent(Object* pSender, MouseEventArgs args)
{
}

void OnRelease(Object* pSender, EventArgs args)
{
	GUIPainter::DeleteInstance();
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