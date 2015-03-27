#pragma once

#include "../EDXPrerequisites.h"
#include "../Windows/Event.h"
#include "Color.h"
#include "../Memory/Array.h"
#include "../Memory/RefPtr.h"

#include "OpenGL.h"

namespace EDX
{
	using namespace OpenGL;

	namespace GUI
	{
		class GUIPainter
		{
		private:
			// Singleton handle
			static GUIPainter* mpInstance;

			int mTextListBase;
			int mFBWidth, mFBHeight;
			HFONT mFont;
			HFONT mOldfont;

			OpenGL::Texture2D mBackgroundTex;

			HDC mHDC;
			FrameBuffer mFBO;
			RenderBuffer mColorRBO;

			Shader mVertexShader;
			Shader mBlurFragmentShader;
			Program mProgram;

			// Gaussian blur weights and offsets
			static const int BLUR_SAMPLE = 13;
			float mGaussianWeights[BLUR_SAMPLE];
			Vector2 mGaussianOffsets[BLUR_SAMPLE];

			// Precomputed coordinates for circle
			static const int CIRCLE_VERTEX_COUNT = 12;
			Vector2 mCircleCoords[CIRCLE_VERTEX_COUNT];

			static const char* GUIPainter::ScreenQuadVertShaderSource;
			static const char* GUIPainter::GaussianBlurFragShaderSource;

		public:
			static const float DEPTH_FAR;
			static const float DEPTH_MID;
			static const float DEPTH_NEAR;

		private:
			GUIPainter();
			~GUIPainter();

		public:
			static inline GUIPainter* Instance()
			{
				if (!mpInstance)
				{
					mpInstance = new GUIPainter;
				}

				return mpInstance;
			}

			static void DeleteInstance()
			{
				if (mpInstance)
				{
					delete mpInstance;
					mpInstance = nullptr;
				}
			}

			HDC GetDC() const { return mHDC; }

			void Resize(int width, int height);
			void BlurBackgroundTexture(int x0, int y0, int x1, int y1);
			void DrawBackgroundTexture(int x0, int y0, int x1, int y1);

			void DrawLine(int iX0, int iY0, int iX1, int iY1, float depth) const;
			void DrawRect(int iX0, int iY0, int iX1, int iY1, float depth,
				const bool filled = false, const Color& color = Color(1.0f, 1.0f, 1.0f, 0.5f), const Color& blendedColor = Color::BLACK) const;
			void DrawRoundedRect(int iX0, int iY0, int iX1, int iY1, float depth, float radius,
				const bool filled = false, const Color& color = Color(1.0f, 1.0f, 1.0f, 0.5f), const Color& blendedColor = Color::BLACK) const;
			void DrawCircle(int x, int y, float depth, int radius, bool filled, const Color& color) const;

			void DrawString(int x, int y, float depth, const char* strText, int length = -1) const;

		private:
			void CalcGaussianBlurWeightsAndOffsets();
		};

		//----------------------------------------------------------------------------------
		// Immediate mode GUI
		//----------------------------------------------------------------------------------
		enum class LayoutStrategy
		{
			DockLeft,
			DockRight,
			Floating
		};

		enum class GrowthStrategy
		{
			Vertical,
			Horizontal
		};

		struct ComboBoxItem
		{
			int Value;
			char* Label;
		};

		struct GuiStates
		{
			int ScreenWidth;
			int ScreenHeight;
			int DialogPosX;
			int DialogPosY;
			int DialogWidth;
			int DialogHeight;
			int CurrentPosX;
			int CurrentPosY;
			int WidgetEndX;
			LayoutStrategy CurrentLayoutStrategy;
			GrowthStrategy CurrentGrowthStrategy;

			int CurrentId;
			int HoveredId;
			int ActiveId;
			int CurrentDialogId;
			int ActiveDialogId;
			MouseEventArgs MouseState;
			MouseEventArgs GlobalMouseState;
			KeyboardEventArgs KeyState;

			// Text edit states
			int EditingId;
			int CursorPos;
			int CursorIdx;
			int SelectIdx;
			string BufferedString;
			vector<int> StrWidthPrefixSum;

			// Scroller states
			int ScrollerInitY;
			int OrigY;

			// Console states
			string ConsoleTextBuffer;
			float ConsoleScroller;
		};

		class EDXGui
		{
		private:
			static GuiStates* States;
			static const int Padding = 10;

		public:
			static void Init();
			static void Release();
			static void BeginFrame();
			static void EndFrame();
			static void BeginDialog(LayoutStrategy layoutStrategy = LayoutStrategy::DockRight,
				const int x = 25,
				const int y = 25,
				const int dialogWidth = 250,
				const int dialogHeight = 420);
			static void EndDialog();
			static void Resize(int screenWidth, int screenHeight);
			static bool HandleMouseEvent(const MouseEventArgs& mouseArgs);
			static bool HandleKeyboardEvent(const KeyboardEventArgs& keyArgs);

			static void Text(const char* str, ...);
			static void MultilineText(const char* str, ...);
			static bool CollapsingHeader(const char* str, bool& collapsed);
			static void CloseHeaderSection() { States->CurrentPosX -= 16; }
			static bool Button(const char* str, const int width = 99999, const int height = 22);
			static void CheckBox(const char* str, bool& checked);
			static void RadioButton(const char* str, int activeVal, int& currentVal);
			static void ComboBox(const char* lable, const ComboBoxItem* pItems, int numItems, int& selected);
			static bool InputText(string& str, const int width = 100, const bool autoSelectAll = false, const bool autoClearOnEnter = false);
			static bool InputDigit(int& digit, const char* notation);
			static void Scroller(int limitLen, int actualLen, float& lin);
			static void BeginScrollableArea(int areaHeight, int& contentHeight, float& scroller);
			static void EndScrollableArea(int areaHeight, int& contentHeight, float& scroller);

			static void Console(const char* str,
				const int x = 25,
				const int y = 25,
				const int width = 640,
				const int height = 480);
			static void ConsoleCommand(const char* command);

			static void ColorSlider(Color* pColor)
			{
				const int Dist = 5;
				auto initX = States->CurrentPosX;
				auto initY = States->CurrentPosY;
				auto initEndX = States->WidgetEndX;
				auto stride = (initEndX - initX - Dist - Dist) / 3.0f;

				States->WidgetEndX = initX + stride;
				Slider<float>("R", &pColor->r, 0.0f, 1.0f);

				States->CurrentPosX += stride + Dist;
				States->CurrentPosY = initY;
				States->WidgetEndX = initX + 2 * stride + Dist;
				Slider<float>("G", &pColor->g, 0.0f, 1.0f);

				States->CurrentPosX += stride + Dist;
				States->CurrentPosY = initY;
				States->WidgetEndX = initX + 3 * stride + 2 * Dist;
				Slider<float>("B", &pColor->b, 0.0f, 1.0f);

				States->CurrentPosX = initX;
				States->WidgetEndX = initEndX;
			}

			template<typename T>
			static void Slider(const char* str, T* pVal, T min, T max)
			{
				// Print slider text
				Text("%s: %.2f", str, (float)*pVal);
				States->CurrentPosY -= 9;

				const int ButtonSize = 12;
				const int ButtonSize_2 = 7;
				const int SlideBase = States->CurrentPosX + ButtonSize_2;
				const int SlideEnd = States->WidgetEndX - ButtonSize_2;

				int Id = States->CurrentId++;

				float lin = Math::LinStep(*pVal, min, max);
				int buttonX = (int)Math::Lerp(SlideBase, SlideEnd, lin);

				RECT barRect;
				SetRect(&barRect, States->CurrentPosX, States->CurrentPosY, States->WidgetEndX, States->CurrentPosY + ButtonSize);

				POINT mousePt;
				mousePt.x = States->MouseState.x;
				mousePt.y = States->MouseState.y;
				if (PtInRect(&barRect, mousePt))
				{
					if (States->MouseState.Action == MouseAction::LButtonDown)
						States->ActiveId = Id;
					
					if (States->MouseState.Action == MouseAction::LButtonUp)
					{
						if (States->ActiveId == Id)
						{
							buttonX = Math::Clamp(States->MouseState.x, SlideBase, SlideEnd);
							float btnLin = Math::LinStep(buttonX, SlideBase, SlideEnd);
							*pVal = (T)Math::Lerp(min, max, btnLin);
							float lin = Math::LinStep(*pVal, min, max);
							buttonX = (int)Math::Lerp(SlideBase, SlideEnd, lin);

							States->ActiveId = -1;
						}
					}

					States->HoveredId = Id;
				}

				if (States->MouseState.Action == MouseAction::Move && States->MouseState.lDown)
				{
					if (States->ActiveId == Id)
					{
						buttonX = Math::Clamp(States->MouseState.x, SlideBase, SlideEnd);
						float btnLin = Math::LinStep(buttonX, SlideBase, SlideEnd);
						*pVal = (T)Math::Lerp(min, max, btnLin);
						float lin = Math::LinStep(*pVal, min, max);
						buttonX = (int)Math::Lerp(SlideBase, SlideEnd, lin);
					}
				}

				
				if (States->MouseState.Action == MouseAction::LButtonUp)
					if (States->ActiveId == Id)
						States->ActiveId = -1;

				Color color = States->HoveredId == Id && States->ActiveId == -1 || States->ActiveId == Id ?
					Color(1.0f, 1.0f, 1.0f, 0.65f) : Color(1.0f, 1.0f, 1.0f, 0.5f);

				// Rendering
				// First half bar
				GUIPainter::Instance()->DrawRect(States->CurrentPosX,
					States->CurrentPosY + ButtonSize_2 - 1,
					buttonX - ButtonSize_2,
					States->CurrentPosY + ButtonSize_2 + 1,
					GUIPainter::DEPTH_MID,
					true, color);
				// Second half bar
				glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				glColor4fv((float*)&color);
				glBegin(GL_LINE_STRIP);

				glVertex3f(buttonX + ButtonSize_2, States->CurrentPosY + ButtonSize_2 - 1, GUIPainter::DEPTH_MID);
				glVertex3f(States->WidgetEndX, States->CurrentPosY + ButtonSize_2 - 1, GUIPainter::DEPTH_MID);
				glVertex3f(States->WidgetEndX, States->CurrentPosY + ButtonSize_2 + 1, GUIPainter::DEPTH_MID);
				glVertex3f(buttonX + ButtonSize_2, States->CurrentPosY + ButtonSize_2 + 1, GUIPainter::DEPTH_MID);

				glEnd();

				// Button
				GUIPainter::Instance()->DrawCircle(buttonX, States->CurrentPosY + ButtonSize_2, GUIPainter::DEPTH_MID, ButtonSize_2, true, color);
				//GUIPainter::Instance()->DrawRect(buttonX - ButtonSize_2 + 1,
				//	States->CurrentPosY,
				//	buttonX + ButtonSize_2,
				//	States->CurrentPosY + ButtonSize,
				//	GUIPainter::DEPTH_MID,
				//	true, color);

				if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
					States->CurrentPosY += ButtonSize + Padding;
				else
					States->CurrentPosX += 5;
			}
		};
	}
}
