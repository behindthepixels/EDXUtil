#include "Windows/Window.h"
#include "Windows/Application.h"
#include "Graphics/EDXGui.h"

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

double TimeWarp(double time)
{
	const double dPhaseShift = mPhase * mTotalPeriod / (2 * PI);
	time += dPhaseShift;

	double halfSinePeriod = 0.5 * mPeriod;
	double invHalfPeriod = 1.0 / (0.5 * (mTotalPeriod));
	time -= halfSinePeriod * 0.5;

	double overPeriod = time * invHalfPeriod;
	int flr = floor(overPeriod);
	double frac = overPeriod - flr;

	if (frac < mStraightLength * invHalfPeriod)
		return flr * halfSinePeriod + halfSinePeriod * 0.5 - dPhaseShift;
	else
		return flr * halfSinePeriod + (frac - mStraightLength * invHalfPeriod) / invHalfPeriod + halfSinePeriod * 0.5 - dPhaseShift;
}

double Lerp(double min, double max, double lin)
{
	return min * (1.0 - lin) + max * lin;
}

double TimeWarp2(double time)
{
	int nCycles = static_cast<int>(time / mTotalPeriod);
	double frac = (time - nCycles * mTotalPeriod) / mTotalPeriod;

	if (frac < mHalfRatio)
		frac = Lerp(0.0, 0.5, frac / mHalfRatio);
	else
		frac = Lerp(0.5, 1.0, (frac - mHalfRatio) / (1.0 - mHalfRatio));

	return (nCycles + frac) * mTotalPeriod;
}

double TimeWarp3(double time)
{
	int nCycles = static_cast<int>(time / (mTotalPeriod * 0.5));
	double frac = (time - nCycles * mTotalPeriod * 0.5) / (mTotalPeriod * 0.5);

	double ratio = 2.0 * mStraightLength / mTotalPeriod;


	if (frac < ratio)
		frac = 0.0;
	else
		frac = Lerp(0.0, 1.0, (frac - ratio) / (1.0 - ratio));

	return (nCycles + frac) * mTotalPeriod * 0.5;
}


double rectifyTimeToFractionOfPeriod(double time)
{
	while (time < 0)
		time += mTotalPeriod;
	if (time >= mTotalPeriod)
	{
		int nCycles = static_cast<int>(time / mTotalPeriod);
		time -= nCycles * mTotalPeriod;
	}
	return time / mTotalPeriod;
}
bool SecondHalfCycle(double time)
{
	int nCycles = static_cast<int>(time / double(mPeriod));

	return nCycles % 2;
}

double EvaluateSine(double fractionOfPeriod)
{
	double time = mPeriod * fractionOfPeriod;
	double omega = 2 * 3.141692653f / mPeriod;

	return mAmp * sin(omega * time) + mConst;
}

double EvaluateSineMod(double fractionOfPeriod)
{
	double time = mPeriod * fractionOfPeriod;
	double omega = 2 * PI / mPeriod;

	return mAmp * 0.5 * sin(omega * time) + mConst;
}

double evaluateValueEdward(double time)
{
	if (mMod)
	{
		if (SecondHalfCycle(2.0 * time + 2.0 * (mPhase + 1.5708) * mTotalPeriod / (2 * PI)))
		{
			time = TimeWarp(time);
			time += mPhase * mTotalPeriod / (2 * PI);
			double t = rectifyTimeToFractionOfPeriod(time);
			return EvaluateSine(t);
		}

		time *= 2.0;
		time += mTotalPeriod * 0.75;
		time += 2.0 * mPhase * mTotalPeriod / (2 * PI);
		bool bSec = SecondHalfCycle(time - mTotalPeriod * 0.75);
		double t = rectifyTimeToFractionOfPeriod(time);


		if (bSec)
			return -EvaluateSineMod(t) - 0.5 * mAmp + 2.0 * mConst;
		return EvaluateSineMod(t) + 0.5 * mAmp;
	}
	else
	{
		time += mStraightLength / 2;
		time += 6.28 * mTotalPeriod / (2 * PI);
		time += mPhase * mTotalPeriod / (2 * PI);
		time = TimeWarp3(TimeWarp2(time - 4.71 * mTotalPeriod / (2 * PI)));
		time += 4.71 * mTotalPeriod / (2 * PI);
		double t = rectifyTimeToFractionOfPeriod(time);

		return EvaluateSine(t);
	}
}

float gSlide;
bool gChecked;
int gCombo;

void OnInit(Object* pSender, EventArgs args)
{
	GL::LoadGLExtensions();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	gDialog.Init(1280, 800);

	gDialog.AddText(0, "GUI Test");
	gDialog.AddCheckBox(1, mMod, &mMod, "Mod");
	gDialog.AddButton(2, "Button");
	gDialog.AddSlider(3, 0.0f, 6.28f, mPhase, &mPhase, "Phase: ");
	gDialog.AddSlider(3, 0.0f, 1.0f, mHalfRatio, &mHalfRatio, "Half Ratio: ");
	gDialog.AddSlider(4, 0.0f, mTotalPeriod, mStraightLength, &mStraightLength, "Motionless: ");

	ComboBoxItem items[] = {
			{ 0, "Item 1" },
			{ 1, "Item 2" }
	};

	gDialog.AddComboBox(4, 0, &gCombo, items, 2);
	gDialog.AddComboBox(5, 0, &gCombo, items, 2);
}

void OnRender(Object* pSender, EventArgs args)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	int elevation = 500;
	glLineWidth(3.0f);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glBegin(GL_LINE_STRIP);

	for (auto i = 0; i < 1500; i++)
	{
		glVertex2f(i, evaluateValueEdward(i));
	}
	glEnd();

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