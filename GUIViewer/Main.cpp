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
	c += "\\Background.jpg";
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

	EDXGui::BeginFrame();
	EDXGui::BeginDialog();

	EDXGui::Text("Right docked dialog");

	static string buf("");
	EDXGui::Text(buf.c_str());
	if (EDXGui::Button("Button 1"))
		buf = "Button 1 clicked";
	if (EDXGui::Button("Button 2"))
		buf = "Button 2 clicked";

	static float counter1 = 0;
	static int counter2 = 0;

	//EDXGui::Text("Value 1: %f", counter1);
	EDXGui::Slider<float>("Float slider", &counter1, 0.0f, 20.0f);
	//EDXGui::Text("Value 2: %i", counter2);
	EDXGui::Slider<int>("Int slider", &counter2, 0, 5);

	static bool show = false;
	if (EDXGui::CollapsingHeader("Collapsing Header", show))
	{
		static bool checked = false;
		EDXGui::CheckBox("Check Box", checked);

		static int radioVal;
		EDXGui::RadioButton("Radio Button 1", 1, radioVal);
		EDXGui::RadioButton("Radio Button 2", 2, radioVal);
		EDXGui::RadioButton("Radio Button 3", 3, radioVal);

		static bool show2 = true;
		if (EDXGui::CollapsingHeader("Collapsing Header 2", show2))
		{
			EDXGui::Button("Hidden Button");
			EDXGui::CloseHeaderSection();
		}

		EDXGui::CloseHeaderSection();
	}

	static int selected = 0;
	ComboBoxItem items[] = {
			{ 1, "Item 1" },
			{ 2, "Item 2" },
			{ 3, "Item 3" },
	};
	EDXGui::ComboBox("Combo box", items, 3, selected);


	static string textBuf1("Text field 1");
	static string textBuf2("Text field 2");
	EDXGui::InputText(textBuf1);
	EDXGui::InputText(textBuf2);

	static int digitInput;
	EDXGui::InputDigit(digitInput, "Digit input");

	static bool showSecondDialog = true;
	EDXGui::CheckBox("Multiple Dialog", showSecondDialog);

	static bool showConsole = true;
	EDXGui::CheckBox("Show Console", showConsole);

	EDXGui::EndDialog();

	if (showSecondDialog)
	{
		EDXGui::BeginDialog(LayoutStrategy::Floating);
		{
			EDXGui::Text("Multiple dialogs supported");
			static string multiLineText = "Multiple lines of texts and wrapped texts supported:\n";

			if (EDXGui::Button("Add Texts"))
				multiLineText += "Adding more texts! ";

			static float scroller = 0.0f;
			static int contentHeight = 0;
			EDXGui::BeginScrollableArea(320, contentHeight, scroller);

			EDXGui::MultilineText(multiLineText.c_str());

			EDXGui::EndScrollableArea(320, contentHeight, scroller);
		}
		EDXGui::EndDialog();
	}

	if (showConsole)
		EDXGui::Console("", 300);

	EDXGui::EndFrame();
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

	mainWindow->Create(L"EDXGui", 1280, 720);

	Application::Run(mainWindow);

	return 0;
}