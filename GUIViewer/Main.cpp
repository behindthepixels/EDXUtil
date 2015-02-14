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

	EDXGui::Init();

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

	static string buf("fuck");
	static string buf2("you");
	static int counter = 0;
	EDXGui::BeginDialog();

	EDXGui::Text("Active Id: %i", EDXGui::States->ActiveId);
	EDXGui::Text("Hovered Id: %i", EDXGui::States->HoveredId);
	EDXGui::Text("Editing Id: %i", EDXGui::States->EditingId);
	EDXGui::Text("L Down: %i", EDXGui::States->MouseState.lDown ? 1 : 0);
	EDXGui::Text("Value: %i", counter);
	if (EDXGui::Bottun("Button 1"))
		counter += 1;
	EDXGui::Slider<int>("Slider 1", &counter, 0.0f, 20.0f);
	static bool checked = false;
	EDXGui::CheckBox("Check Box", checked);
	if (EDXGui::Bottun("Button 2"))
		buf = "";
	static int selected = 0;
	ComboBoxItem items[] = {
		{ 1, "Item 1" },
		{ 2, "Item 2" },
		{ 3, "Item 3" },
	};
	EDXGui::ComboBox(items, 3, selected);
	if (EDXGui::Bottun("Button 3"))
		counter -= 3;

	EDXGui::Text("Cursor Idx: %i", EDXGui::States->CursorIdx);
	EDXGui::InputText(buf);
	EDXGui::InputText(buf2);
	EDXGui::InputDigit(counter, "Number:");

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

	EDXGui::Resize(args.Width, args.Height);
}

void OnMouseEvent(Object* pSender, MouseEventArgs args)
{
	EDXGui::HandleMouseEvent(args);
}

void OnKeyboardEvent(Object* pSender, KeyboardEventArgs args)
{
	EDXGui::HandleKeyboardEvent(args);
}

void OnRelease(Object* pSender, EventArgs args)
{
	EDXGui::Release();
	SafeDeleteArray(gImage);
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdArgs, int cmdShow)
{
	Application::Init(hInst);

	Window* mainWindow = new GLWindow;
	mainWindow->SetMainLoop(NotifyEvent(OnRender));
	mainWindow->SetInit(NotifyEvent(OnInit));
	mainWindow->SetResize(ResizeEvent(OnResize));
	mainWindow->SetMouseHandler(MouseEvent(OnMouseEvent));
	mainWindow->SetkeyboardHandler(KeyboardEvent(OnKeyboardEvent));
	mainWindow->SetRelease(NotifyEvent(OnRelease));

	mainWindow->Create(L"EDXGui", 1280, 800);

	Application::Run(mainWindow);

	return 0;
}