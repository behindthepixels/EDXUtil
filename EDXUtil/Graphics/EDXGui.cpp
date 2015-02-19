
#include <stdarg.h>

#include "EDXGui.h"
#include "../Math/EDXMath.h"
#include "../Memory/Memory.h"

#include "../Windows/Application.h"
#include "../Windows/Window.h"

namespace EDX
{
	namespace GUI
	{
		//----------------------------------------------------------------------------------
		// GUI Painter implementation
		//----------------------------------------------------------------------------------
		GUIPainter* GUIPainter::mpInstance = NULL;

		const float GUIPainter::DEPTH_FAR = 0.8f;
		const float GUIPainter::DEPTH_MID = 0.6f;
		const float GUIPainter::DEPTH_NEAR = 0.4f;

		const char* GUIPainter::ScreenQuadVertShaderSource = R"(
		    varying vec2 texCoord;
			void main()
			{
				gl_Position = gl_Vertex;
				texCoord = gl_MultiTexCoord0.xy;
			})";
		const char* GUIPainter::GaussianBlurFragShaderSource = R"(
			uniform sampler2D texSampler;
			uniform float weights[13];
			uniform vec2 offsets[13];
			varying vec2 texCoord;
			void main()
			{
				vec4 sample = 0.0f;
				for(int i = 0; i < 13; i++)
				{
					sample += weights[i] * texture2DLod(texSampler, texCoord + offsets[i], 2);
				}
				gl_FragColor = vec4(sample.rgb, 1.0);
			})";

		GUIPainter::GUIPainter()
		{
			// Initialize font
			mTextListBase = glGenLists(128);
			mFont = CreateFont(16,
				0,
				0,
				0,
				FW_BOLD,
				FALSE,
				FALSE,
				FALSE,
				DEFAULT_CHARSET,
				OUT_TT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				ANTIALIASED_QUALITY,
				FF_DONTCARE | DEFAULT_PITCH,
				L"Helvetica");

			mHDC = ::GetDC(Application::GetMainWindow()->GetHandle());
			mOldfont = (HFONT)SelectObject(mHDC, mFont);
			wglUseFontBitmaps(mHDC, 0, 128, mTextListBase);

			// Load shaders
			mVertexShader.Load(ShaderType::VertexShader, ScreenQuadVertShaderSource);
			mBlurFragmentShader.Load(ShaderType::FragmentShader, GaussianBlurFragShaderSource);
			mProgram.AttachShader(&mVertexShader);
			mProgram.AttachShader(&mBlurFragmentShader);
			mProgram.Link();

			// Calculate circle coordinates
			const float phiItvl = float(Math::EDX_TWO_PI) / float(CIRCLE_VERTEX_COUNT - 1);

			float phi = 0.0f;
			for (int i = 0; i < CIRCLE_VERTEX_COUNT - 1; i++)
			{
				mCircleCoords[i].x = Math::Sin(phi);
				mCircleCoords[i].y = -Math::Cos(phi);
				phi += phiItvl;
			}
			mCircleCoords[CIRCLE_VERTEX_COUNT - 1] = mCircleCoords[0];
		}

		GUIPainter::~GUIPainter()
		{
			SelectObject(mHDC, mOldfont);
			DeleteObject(mFont);
		}

		void GUIPainter::Resize(int width, int height)
		{
			mFBWidth = width;
			mFBHeight = height;

			// Init background texture
			mColorRBO.SetStorage(width >> 2, height >> 2, ImageFormat::RGBA);
			mFBO.Attach(FrameBufferAttachment::Color0, &mColorRBO);

			CalcGaussianBlurWeightsAndOffsets();
		}

		void GUIPainter::BlurBackgroundTexture(int x0, int y0, int x1, int y1)
		{
			mBackgroundTex.ReadFromFrameBuffer(ImageFormat::RGBA, mFBWidth, mFBHeight);

			float u0 = (x0 / (float)mFBWidth);
			float v0 = (y0 / (float)mFBHeight);
			float u1 = (x1 / (float)mFBWidth);
			float v1 = (y1 / (float)mFBHeight);
			float _x0 = u0 * 2.0f - 1.0f;
			float _y0 = v0 * 2.0f - 1.0f;
			float _x1 = u1 * 2.0f - 1.0f;
			float _y1 = v1 * 2.0f - 1.0f;

			mFBO.SetTarget(FrameBufferTarget::Draw);
			mFBO.Bind();

			glViewport(0, 0, mFBWidth >> 2, mFBHeight >> 2);
			glClear(GL_COLOR_BUFFER_BIT);

			mProgram.Use();
			mProgram.SetUniform("texSampler", 0);
			mProgram.SetUniform("weights", mGaussianWeights, 13);
			mProgram.SetUniform("offsets", mGaussianOffsets, 13);

			mBackgroundTex.Bind();
			mBackgroundTex.SetFilter(TextureFilter::TriLinear);
			glBegin(GL_QUADS);

			glTexCoord2f(u0, v0);
			glVertex3f(_x0, _y0, DEPTH_FAR);

			glTexCoord2f(u1, v0);
			glVertex3f(_x1, _y0, DEPTH_FAR);

			glTexCoord2f(u1, v1);
			glVertex3f(_x1, _y1, DEPTH_FAR);

			glTexCoord2f(u0, v1);
			glVertex3f(_x0, _y1, DEPTH_FAR);

			glEnd();

			mProgram.Unuse();
			mBackgroundTex.UnBind();
			mFBO.UnBind();

			glViewport(0, 0, mFBWidth, mFBHeight);
		}

		void GUIPainter::DrawBackgroundTexture(int x0, int y0, int x1, int y1)
		{
			mFBO.SetTarget(FrameBufferTarget::Read);
			mFBO.Bind();

			glBlitFramebuffer((x0 + 3) >> 2, (y0 - 3) >> 2, (x1 - 3) >> 2, (y1 + 3) >> 2, x0, y0, x1, y1, GL_COLOR_BUFFER_BIT, GL_LINEAR);

			mFBO.UnBind();
		}

		void GUIPainter::DrawRect(int iX0, int iY0, int iX1, int iY1, float depth, const bool filled, const Color& color, const Color& blendedColor)
		{
			auto Draw = [](int iX0, int iY0, int iX1, int iY1, float depth)
			{
				glBegin(GL_QUADS);

				glVertex3f(iX0, iY0, depth);
				glVertex3f(iX1, iY0, depth);
				glVertex3f(iX1, iY1, depth);
				glVertex3f(iX0, iY1, depth);

				glEnd();
			};

			glBlendColor(blendedColor.r, blendedColor.g, blendedColor.b, blendedColor.a);
			glColor4fv((float*)&color);

			if (filled)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				Draw(iX0 - 1, iY0, iX1, iY1 + 1, depth);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				Draw(iX0, iY0, iX1, iY1, depth);
			}
		}

		void GUIPainter::DrawRoundedRect(int x0, int y0, int x1, int y1, float depth, float radius, const bool filled, const Color& color, const Color& blendedColor) const
		{
			glBlendColor(blendedColor.r, blendedColor.g, blendedColor.b, blendedColor.a);
			glColor4fv((float*)&color);
			if (filled)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glBegin(GL_TRIANGLE_FAN);

				const int quaterVertexCount = CIRCLE_VERTEX_COUNT / 4;
				// Center vertex
				glVertex3f((x0 + x1) * 0.5f, (y0 + y1) * 0.5f, depth);

				glVertex3f(x0 + radius, y0, depth);
				glVertex3f(x1 - radius, y0, depth);
				for (auto i = 0; i < quaterVertexCount; i++)
				{
					float _x = x1 - radius + mCircleCoords[i].x * radius;
					float _y = y0 + radius + mCircleCoords[i].y * radius;
					glVertex3f(_x, _y, depth);
				}
				glVertex3f(x1, y0 + radius, depth);
				glVertex3f(x1, y1 - radius, depth);
				for (auto i = quaterVertexCount; i < 2 * quaterVertexCount; i++)
				{
					float _x = x1 - radius + mCircleCoords[i].x * radius;
					float _y = y1 - radius + mCircleCoords[i].y * radius;
					glVertex3f(_x, _y, depth);
				}
				glVertex3f(x1 - radius, y1, depth);
				glVertex3f(x0 + radius, y1, depth);
				for (auto i = 2 * quaterVertexCount; i < 3 * quaterVertexCount; i++)
				{
					float _x = x0 + radius + mCircleCoords[i].x * radius;
					float _y = y1 - radius + mCircleCoords[i].y * radius;
					glVertex3f(_x, _y, depth);
				}
				glVertex3f(x0, y1 - radius, depth);
				glVertex3f(x0, y0 + radius, depth);
				for (auto i = 3 * quaterVertexCount; i < 4 * quaterVertexCount; i++)
				{
					float _x = x0 + radius + mCircleCoords[i].x * radius;
					float _y = y0 + radius + mCircleCoords[i].y * radius;
					glVertex3f(_x, _y, depth);
				}

				glEnd();
			}
			else
			{
				radius += 1;

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glBegin(GL_LINE_STRIP);

				const int quaterVertexCount = CIRCLE_VERTEX_COUNT / 4;

				glVertex3f(x0 + radius, y0, depth);
				glVertex3f(x1 - radius, y0, depth);
				for (auto i = 0; i < quaterVertexCount; i++)
				{
					float _x = x1 - radius + mCircleCoords[i].x * radius;
					float _y = y0 + radius + mCircleCoords[i].y * radius;
					glVertex3f(_x, _y, depth);
				}

				glVertex3f(x1, y1 - radius, depth);
				for (auto i = quaterVertexCount; i < 2 * quaterVertexCount; i++)
				{
					float _x = x1 - radius + mCircleCoords[i].x * radius;
					float _y = y1 - radius + mCircleCoords[i].y * radius;
					glVertex3f(_x, _y, depth);
				}

				glVertex3f(x0 + radius, y1, depth);
				for (auto i = 2 * quaterVertexCount; i < 3 * quaterVertexCount; i++)
				{
					float _x = x0 + radius + mCircleCoords[i].x * radius;
					float _y = y1 - radius + mCircleCoords[i].y * radius;
					glVertex3f(_x, _y, depth);
				}

				glVertex3f(x0, y0 + radius, depth);
				for (auto i = 3 * quaterVertexCount; i < 4 * quaterVertexCount; i++)
				{
					float _x = x0 + radius + mCircleCoords[i].x * radius;
					float _y = y0 + radius + mCircleCoords[i].y * radius;
					glVertex3f(_x, _y, depth);
				}

				glEnd();
			}
		}

		void GUIPainter::DrawCircle(int x, int y, float depth, int radius, bool filled, const Color& color) const
		{
			glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
			glColor4fv((float*)&color);

			if (filled)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glBegin(GL_TRIANGLE_FAN);

				// Center vertex
				glVertex3f(x, y, depth);

				for (auto i = 0; i < CIRCLE_VERTEX_COUNT; i++)
				{
					float _x = x + mCircleCoords[i].x * radius;
					float _y = y + mCircleCoords[i].y * radius;
					glVertex3f(_x, _y, depth);
				}

				glEnd();
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glBegin(GL_LINE_STRIP);

				for (auto i = 0; i < CIRCLE_VERTEX_COUNT; i++)
				{
					float _x = x + mCircleCoords[i].x * radius;
					float _y = y + mCircleCoords[i].y * radius;
					glVertex3f(_x, _y, depth);
				}

				glEnd();
			}
		}

		void GUIPainter::DrawLine(int iX0, int iY0, int iX1, int iY1, float depth)
		{
			glBegin(GL_LINES);

			glVertex3f(iX0, iY0, depth);
			glVertex3f(iX1, iY1, depth);

			glEnd();
		}

		void GUIPainter::DrawString(int x, int y, float depth, const char* strText, int length)
		{
			glListBase(mTextListBase);

			glRasterPos3f(x, y + 10, depth);
			if (length == -1)
				glCallLists((GLsizei)strlen(strText), GL_UNSIGNED_BYTE, strText);
			else
			{
				assert(length >= 0 && length <= strlen(strText));
				glCallLists(length, GL_UNSIGNED_BYTE, strText);
			}
		}

		void GUIPainter::CalcGaussianBlurWeightsAndOffsets()
		{
			auto GaussianDistribution = [](float x, float y, float rho) -> float
			{
				float g = 1.0f / Math::Sqrt(2.0f * float(Math::EDX_PI) * rho * rho);
				g *= Math::Exp(-(x * x + y * y) / (2 * rho * rho));

				return g;
			};

			float tu = 1.0f / (float)mFBWidth * 4;
			float tv = 1.0f / (float)mFBHeight * 4;

			float totalWeight = 0.0f;
			int index = 0;
			for (int x = -2; x <= 2; x++)
			{
				for (int y = -2; y <= 2; y++)
				{
					if (abs(x) + abs(y) > 2)
						continue;

					// Get the unscaled Gaussian intensity for this offset
					mGaussianOffsets[index] = Vector2(x * tu, y * tv);
					mGaussianWeights[index] = GaussianDistribution((float)x, (float)y, 1.0f);
					totalWeight += mGaussianWeights[index];

					index++;
				}
			}

			for (int i = 0; i < index; i++)
				mGaussianWeights[i] /= totalWeight;
		}

		//----------------------------------------------------------------------------------
		// Immediate mode GUI implementation
		//----------------------------------------------------------------------------------
		GuiStates* EDXGui::States;

		void EDXGui::Init()
		{
			States = new GuiStates;
			States->ActiveId = -1;
			States->KeyState.key = char(Key::None);
			States->EditingId = -1;
			States->ScrollerInitY = 0.0f;
		}

		void EDXGui::Release()
		{
			SafeDelete(States);
			GUIPainter::DeleteInstance();
		}

		void EDXGui::BeginFrame()
		{
			States->CurrentId = 0;
			States->HoveredId = -1;
		}

		void EDXGui::EndFrame()
		{
			States->GlobalMouseState.Action = MouseAction::None;
			States->KeyState.key = char(Key::None);
		}

		void EDXGui::BeginDialog(LayoutStrategy layoutStrategy,
			const int x,
			const int y,
			const int dialogWidth,
			const int dialogHeight)
		{
			States->CurrentLayoutStrategy = layoutStrategy;
			States->CurrentGrowthStrategy = GrowthStrategy::Vertical;

			switch (States->CurrentLayoutStrategy)
			{
			case LayoutStrategy::DockRight:
					States->DialogWidth = 200;
					States->DialogHeight = States->ScreenHeight;
					States->DialogPosX = States->ScreenWidth - States->DialogWidth;
					States->DialogPosY = 0;
					States->CurrentPosX = 25;
					States->CurrentPosY = 25;
					States->WidgetEndX = States->DialogWidth - 25;
				break;
			case LayoutStrategy::DockLeft:
					States->DialogWidth = 200;
					States->DialogHeight = States->ScreenHeight;
					States->DialogPosX = 0;
					States->DialogPosY = 0;
					States->CurrentPosX = 25;
					States->CurrentPosY = 25;
					States->WidgetEndX = States->DialogWidth - 25;
				break;
			case LayoutStrategy::Floating:
					States->DialogWidth = dialogWidth;
					States->DialogHeight = dialogHeight;
					States->DialogPosX = x;
					States->DialogPosY = y;
					States->CurrentPosX = 25;
					States->CurrentPosY = 25;
					States->WidgetEndX = States->DialogWidth - 25;
				break;
			}

			States->MouseState = States->GlobalMouseState;
			States->MouseState.x = States->GlobalMouseState.x - States->DialogPosX;
			States->MouseState.y = States->GlobalMouseState.y - States->DialogPosY;

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, States->ScreenWidth, 0, States->ScreenHeight, 1, -1);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			// Render the blurred background texture
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glEnable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			GUIPainter::Instance()->BlurBackgroundTexture(States->DialogPosX, States->ScreenHeight - States->DialogPosY,
				States->DialogPosX + States->DialogWidth, States->ScreenHeight - (States->DialogPosY + States->DialogHeight));
			GUIPainter::Instance()->DrawBackgroundTexture(States->DialogPosX, States->ScreenHeight - States->DialogPosY,
				States->DialogPosX + States->DialogWidth, States->ScreenHeight - (States->DialogPosY + States->DialogHeight));
			GUIPainter::Instance()->BlurBackgroundTexture(States->DialogPosX, States->ScreenHeight - States->DialogPosY,
				States->DialogPosX + States->DialogWidth, States->ScreenHeight - (States->DialogPosY + States->DialogHeight));
			GUIPainter::Instance()->DrawBackgroundTexture(States->DialogPosX, States->ScreenHeight - States->DialogPosY,
				States->DialogPosX + States->DialogWidth, States->ScreenHeight - (States->DialogPosY + States->DialogHeight));

			glTranslatef(States->DialogPosX, States->ScreenHeight - States->DialogPosY, 0.0f);
			glScalef(1.0f, -1.0f, 1.0f);

			glLineWidth(1.0f);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_CONSTANT_ALPHA);
			glBlendEquation(GL_FUNC_ADD);

			// Draw blurred background
			if (States->CurrentLayoutStrategy == LayoutStrategy::Floating)
			{
				GUIPainter::Instance()->DrawRoundedRect(0,
					0,
					States->DialogWidth,
					States->DialogHeight,
					GUIPainter::DEPTH_FAR,
					15.0f,
					true,
					Color(0.0f, 0.0f, 0.0f, 0.5f),
					Color(1.0f, 1.0f, 1.0f, 0.5f));
			}
			else
			{
				glBlendColor(1.0f, 1.0f, 1.0f, 0.5f);
				glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
				glBegin(GL_QUADS);

				glVertex3f(0, 0, GUIPainter::DEPTH_FAR);
				glVertex3f(States->DialogWidth, 0, GUIPainter::DEPTH_FAR);
				glVertex3f(States->DialogWidth, States->DialogHeight, GUIPainter::DEPTH_FAR);
				glVertex3f(0, States->DialogHeight, GUIPainter::DEPTH_FAR);

				glEnd();
			}
		}
		void EDXGui::EndDialog()
		{
			glPopAttrib();
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
		}

		void EDXGui::Resize(int screenWidth, int screenHeight)
		{
			States->ScreenWidth = screenWidth;
			States->ScreenHeight = screenHeight;

			GUIPainter::Instance()->Resize(screenWidth, screenHeight);
		}

		void EDXGui::HandleMouseEvent(const MouseEventArgs& mouseArgs)
		{
			States->GlobalMouseState = mouseArgs;
		}

		void EDXGui::HandleKeyboardEvent(const KeyboardEventArgs& keyArgs)
		{
			States->KeyState = keyArgs;
		}

		void EDXGui::Text(const char* str, ...)
		{
			const int Height = 10;

			States->CurrentId++;

			va_list args;
			va_start(args, str);

			char buff[1024];
			int size = vsnprintf(buff, sizeof(buff) - 1, str, args);

			va_end(args);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(States->CurrentPosX, States->CurrentPosY, GUIPainter::DEPTH_MID, buff);

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += Height + Padding;
			else
				States->CurrentPosX += 5;
		}

		void EDXGui::MultilineText(const char* str, ...)
		{
			const int LineHeight = 16;
			States->CurrentId++;

			// Print va string
			va_list args;
			va_start(args, str);

			char buff[4096];
			int size = vsnprintf(buff, sizeof(buff) - 1, str, args);

			va_end(args);

			// Calculate line count
			vector<int> lineIdx;
			string reformattedStr;
			lineIdx.clear();
			lineIdx.push_back(0);
			auto lineLength = 0;
			for (auto i = 0; i < size; i++)
			{
				if (buff[i] == '\n')
				{
					reformattedStr += '\0';
					lineLength = 0;
					lineIdx.push_back(reformattedStr.length());
				}
				else
				{
					reformattedStr += buff[i];

					SIZE textExtent;
					GetTextExtentPoint32A(GUIPainter::Instance()->GetDC(), &buff[i], 1, &textExtent);
					lineLength += textExtent.cx;

					if (lineLength >= States->WidgetEndX - States->CurrentPosX)
					{
						reformattedStr += '\0';
						lineLength = 0;
						lineIdx.push_back(reformattedStr.length());
					}
				}
			}

			// Render text
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			const auto lineCount = lineIdx.size();
			for (auto i = 0; i < lineCount; i++)
			{
				GUIPainter::Instance()->DrawString(States->CurrentPosX,
					States->CurrentPosY + i * LineHeight,
					GUIPainter::DEPTH_MID,
					reformattedStr.c_str() + lineIdx[i]);
			}

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += lineCount * LineHeight + Padding;
			else
				States->CurrentPosX += 5;
		}

		bool EDXGui::CollapsingHeader(const char* str, bool& collapsed)
		{
			const int Height = 30;
			const int TextHeight = 11;

			int Id = States->CurrentId++;

			RECT headerRect;
			if (collapsed) // Draw dots
				SetRect(&headerRect, States->CurrentPosX, States->CurrentPosY, States->WidgetEndX, States->CurrentPosY + Height);
			else
				SetRect(&headerRect, States->CurrentPosX, States->CurrentPosY, States->WidgetEndX, States->CurrentPosY + TextHeight);

			POINT mousePt;
			mousePt.x = States->MouseState.x;
			mousePt.y = States->MouseState.y;
			bool inRect = PtInRect(&headerRect, mousePt);

			if (inRect)
			{
				if (States->MouseState.Action == MouseAction::LButtonDown)
					States->ActiveId = Id;

				States->HoveredId = Id;
			}

			if (States->MouseState.Action == MouseAction::LButtonUp)
			{
				if (States->ActiveId == Id)
				{
					States->ActiveId = -1;
					if (inRect)
						collapsed = !collapsed;
				}
			}

			// Draw header text
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(States->CurrentPosX, States->CurrentPosY, GUIPainter::DEPTH_MID, str);

			// Draw Line
			Color color = States->HoveredId == Id && States->ActiveId == -1 || States->ActiveId == Id ?
				Color(1.0f, 1.0f, 1.0f, 1.0f) : Color(1.0f, 1.0f, 1.0f, 0.5f);
			glColor4fv((float*)&color);
			GUIPainter::Instance()->DrawLine(States->CurrentPosX, States->CurrentPosY + TextHeight + 1, States->WidgetEndX, States->CurrentPosY + TextHeight + 1, GUIPainter::DEPTH_MID);

			if (collapsed) // Draw dots
				GUIPainter::Instance()->DrawString(States->WidgetEndX - 15, States->CurrentPosY + TextHeight + 6, GUIPainter::DEPTH_MID, "...");
			else
				States->CurrentPosX += 16;

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += (collapsed ? Height : TextHeight) + Padding;
			else
				States->CurrentPosX += 5;

			return !collapsed;
		}

		bool EDXGui::Button(const char* str, const int width, const int height)
		{
			bool trigger = false;
			int Id = States->CurrentId++;

			RECT btnRect;
			SetRect(&btnRect, States->CurrentPosX, States->CurrentPosY, Math::Min(States->CurrentPosX + width, States->WidgetEndX), States->CurrentPosY + height);

			POINT mousePt;
			mousePt.x = States->MouseState.x;
			mousePt.y = States->MouseState.y;

			bool inRect = PtInRect(&btnRect, mousePt);

			if (inRect)
			{
				if (States->MouseState.Action == MouseAction::LButtonDown)
					States->ActiveId = Id;

				States->HoveredId = Id;
			}

			if (States->MouseState.Action == MouseAction::LButtonUp)
			{
				if (States->ActiveId == Id)
				{
					States->ActiveId = -1;
					if (inRect)
						trigger = true;
				}
			}

			float btnRadius = 5.0f;
			if (States->HoveredId == Id && States->ActiveId == Id)
			{
				GUIPainter::Instance()->DrawRoundedRect(btnRect.left + 1,
					btnRect.top + 1,
					btnRect.right - 1,
					btnRect.bottom - 1,
					GUIPainter::DEPTH_MID,
					btnRadius,
					true,
					Color(1.0f, 1.0f, 1.0f, 0.65f));
				//GUIPainter::Instance()->DrawRect(btnRect.left + 1,
				//	btnRect.top + 1,
				//	btnRect.right - 1,
				//	btnRect.bottom - 1,
				//	GUIPainter::DEPTH_MID,
				//	true,
				//	Color(1.0f, 1.0f, 1.0f, 0.65f));

				glColor4f(0.15f, 0.15f, 0.15f, 0.15f);
			}
			else if (States->HoveredId == Id && States->ActiveId == -1 || States->ActiveId == Id)
			{
				GUIPainter::Instance()->DrawRoundedRect(btnRect.left,
					btnRect.top,
					btnRect.right,
					btnRect.bottom,
					GUIPainter::DEPTH_MID,
					btnRadius,
					true,
					Color(1.0f, 1.0f, 1.0f, 0.5f));
				//GUIPainter::Instance()->DrawRect(btnRect.left,
				//	btnRect.top,
				//	btnRect.right,
				//	btnRect.bottom,
				//	GUIPainter::DEPTH_MID,
				//	true,
				//	Color(1.0f, 1.0f, 1.0f, 0.5f));

				glColor4f(0.15f, 0.15f, 0.15f, 0.15f);
			}
			else
			{
				GUIPainter::Instance()->DrawRoundedRect(btnRect.left,
					btnRect.top,
					btnRect.right,
					btnRect.bottom,
					GUIPainter::DEPTH_MID,
					btnRadius,
					false,
					Color(1.0f, 1.0f, 1.0f, 0.5f));
				//GUIPainter::Instance()->DrawRect(btnRect.left,
				//	btnRect.top,
				//	btnRect.right,
				//	btnRect.bottom,
				//	GUIPainter::DEPTH_MID,
				//	false,
				//	Color(1.0f, 1.0f, 1.0f, 0.5f));

				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			}

			SIZE textExtent;
			GetTextExtentPoint32A(GUIPainter::Instance()->GetDC(), str, strlen(str), &textExtent);
			glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
			GUIPainter::Instance()->DrawString((btnRect.right + btnRect.left - textExtent.cx) / 2, (btnRect.top + btnRect.bottom - textExtent.cy) / 2 + 3, GUIPainter::DEPTH_MID, str);

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += height + Padding;
			else
				States->CurrentPosX += 5;

			return trigger;
		}

		void EDXGui::CheckBox(const char* str, bool& checked)
		{
			const int Width = 140;
			const int BoxSize = 12;

			int Id = States->CurrentId++;

			RECT boxRect;
			SetRect(&boxRect, States->CurrentPosX, States->CurrentPosY, States->CurrentPosX + BoxSize, States->CurrentPosY + BoxSize);

			POINT mousePt;
			mousePt.x = States->MouseState.x;
			mousePt.y = States->MouseState.y;

			if (PtInRect(&boxRect, mousePt))
			{
				if (States->MouseState.Action == MouseAction::LButtonDown)
					States->ActiveId = Id;
				if (States->MouseState.Action == MouseAction::LButtonUp)
				{
					if (States->ActiveId == Id)
					{
						States->ActiveId = -1;
						checked = !checked;
					}
				}

				States->HoveredId = Id;
			}
			else
			{
				if (States->MouseState.Action == MouseAction::Move)
					if (States->ActiveId == Id)
						States->ActiveId = -1;
			}

			Color color1 = States->HoveredId == Id && States->ActiveId == -1 ? Color(1.0f, 1.0f, 1.0f, 0.65f) : Color(1.0f, 1.0f, 1.0f, 0.5f);
			GUIPainter::Instance()->DrawRect(boxRect.left,
				boxRect.top,
				boxRect.right,
				boxRect.bottom,
				GUIPainter::DEPTH_MID,
				false,
				color1);

			Color color2 = checked ? color1 : States->HoveredId == Id && States->ActiveId == -1 ? Color(1.0f, 1.0f, 1.0f, 0.15f) : Color::BLACK;
			GUIPainter::Instance()->DrawRect(boxRect.left + 2,
				boxRect.top + 2,
				boxRect.right - 2,
				boxRect.bottom - 2,
				GUIPainter::DEPTH_MID,
				true,
				color2);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(States->CurrentPosX + BoxSize + 7, States->CurrentPosY + 2, GUIPainter::DEPTH_MID, str);

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += BoxSize + Padding;
			else
				States->CurrentPosX += 5;
		}

		void EDXGui::RadioButton(const char* str, int activeVal, int& currentVal)
		{
			const int CircleDiameter = 12;
			const int CircleRadius = CircleDiameter / 2;

			int Id = States->CurrentId++;

			RECT boxRect;
			SetRect(&boxRect, States->CurrentPosX, States->CurrentPosY, States->CurrentPosX + CircleDiameter, States->CurrentPosY + CircleDiameter);

			POINT mousePt;
			mousePt.x = States->MouseState.x;
			mousePt.y = States->MouseState.y;

			if (PtInRect(&boxRect, mousePt))
			{
				if (States->MouseState.Action == MouseAction::LButtonDown)
					States->ActiveId = Id;
				if (States->MouseState.Action == MouseAction::LButtonUp)
				{
					if (States->ActiveId == Id)
					{
						States->ActiveId = -1;
						currentVal = activeVal;
					}
				}

				States->HoveredId = Id;
			}
			else
			{
				if (States->MouseState.Action == MouseAction::Move)
					if (States->ActiveId == Id)
						States->ActiveId = -1;
			}

			Color color1 = States->HoveredId == Id && States->ActiveId == -1 ? Color(1.0f, 1.0f, 1.0f, 0.65f) : Color(1.0f, 1.0f, 1.0f, 0.5f);
			GUIPainter::Instance()->DrawCircle((boxRect.left + boxRect.right) / 2,
				(boxRect.bottom + boxRect.top) * 0.5f,
				GUIPainter::DEPTH_MID,
				CircleRadius,
				false,
				color1);

			Color color2 = currentVal == activeVal ? color1 : States->HoveredId == Id && States->ActiveId == -1 ? Color(1.0f, 1.0f, 1.0f, 0.15f) : Color::BLACK;
			GUIPainter::Instance()->DrawCircle((boxRect.left + boxRect.right) / 2,
				(boxRect.bottom + boxRect.top) * 0.5f,
				GUIPainter::DEPTH_MID,
				CircleRadius - 2,
				true,
				color2);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(States->CurrentPosX + CircleDiameter + 7, States->CurrentPosY + 2, GUIPainter::DEPTH_MID, str);

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += CircleDiameter + Padding;
			else
				States->CurrentPosX += 5;
		}

		void EDXGui::ComboBox(const ComboBoxItem* pItems, int numItems, int& selected)
		{
			const int Width = 140;
			const int Height = 18;
			const int ItemHeight = 20;

			int Id = States->CurrentId++;

			POINT mousePt;
			mousePt.x = States->MouseState.x;
			mousePt.y = States->MouseState.y;

			RECT mainRect;
			SetRect(&mainRect, States->CurrentPosX, States->CurrentPosY, States->WidgetEndX, States->CurrentPosY + Height);
			if (PtInRect(&mainRect, mousePt))
			{
				if (States->MouseState.Action == MouseAction::LButtonDown)
				{
					if (States->ActiveId != Id)
						States->ActiveId = Id;
					else if (States->ActiveId == Id)
						States->ActiveId = -1;
				}

				States->HoveredId = Id;
			}

			Color btnColor = States->ActiveId == Id || States->HoveredId == Id && States->ActiveId == -1 ? Color(1.0f, 1.0f, 1.0f, 0.65f) : Color(1.0f, 1.0f, 1.0f, 0.5f);

			GUIPainter::Instance()->DrawRect(mainRect.left, mainRect.top, mainRect.right, mainRect.bottom, GUIPainter::DEPTH_MID, false, btnColor);
			GUIPainter::Instance()->DrawRect(mainRect.right - Height, mainRect.top + 1, mainRect.right - 1, mainRect.bottom - 1, GUIPainter::DEPTH_MID, true, btnColor);

			glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(mainRect.left + 3, mainRect.top + 5, GUIPainter::DEPTH_MID, pItems[selected].Label);

			if (States->ActiveId == Id)
			{
				RECT dropDownRect;
				SetRect(&dropDownRect, States->CurrentPosX, States->CurrentPosY + Height, States->WidgetEndX - Height, States->CurrentPosY + Height + 1 + numItems * ItemHeight);

				if (PtInRect(&dropDownRect, mousePt) && States->MouseState.Action == MouseAction::LButtonDown)
				{
					selected = (mousePt.y - dropDownRect.top) / ItemHeight;
					States->ActiveId = -1;
					States->HoveredId = Id;
					States->MouseState.Action = MouseAction::None;
				}

				GUIPainter::Instance()->DrawRect(dropDownRect.left, dropDownRect.top + 1, dropDownRect.right, dropDownRect.bottom, GUIPainter::DEPTH_NEAR, true, Color(0.25f, 0.25f, 0.25f, 1.0f));

				int hoveredIdx = Math::Clamp((mousePt.y - dropDownRect.top) / ItemHeight, 0, numItems - 1);
				for (auto i = 0; i < numItems; i++)
				{
					if (i == hoveredIdx)
					{
						GUIPainter::Instance()->DrawRect(dropDownRect.left, dropDownRect.top + 2 + hoveredIdx * ItemHeight, dropDownRect.right - 1, dropDownRect.top + 1 + (hoveredIdx + 1) * ItemHeight, GUIPainter::DEPTH_NEAR, true, Color(0.4f, 0.4f, 0.4f, 0.5f));

						glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
						glColor4f(0.15f, 0.15f, 0.15f, 1.0f);
					}
					else
					{
						glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
						glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
					}

					GUIPainter::Instance()->DrawString(dropDownRect.left + 3, dropDownRect.top + 6 + i * ItemHeight, GUIPainter::DEPTH_NEAR, pItems[i].Label);
				}
			}

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += Height + Padding;
			else
				States->CurrentPosX += 5;
		}

		bool EDXGui::InputText(string& buf, const int width, const bool autoSelectAll, const bool autoClearOnEnter)
		{
			const int Height = 18;
			const int Indent = 4;

			auto CalcCharWidthPrefixSum = [&]()
			{
				// Calculate string length prefix sum
				States->StrWidthPrefixSum.clear();
				States->StrWidthPrefixSum.push_back(0);
				for (auto i = 0; i < States->BufferedString.length(); i++)
				{
					SIZE textExtent;
					GetTextExtentPoint32A(GUIPainter::Instance()->GetDC(), &States->BufferedString[i], 1, &textExtent);
					States->StrWidthPrefixSum.push_back(i == 0 ? textExtent.cx : textExtent.cx + States->StrWidthPrefixSum[i]);
				}
			};

			bool trigger = false;
			int Id = States->CurrentId++;

			RECT rect;
			SetRect(&rect, States->CurrentPosX, States->CurrentPosY, Math::Min(States->CurrentPosX + width, States->WidgetEndX), States->CurrentPosY + Height);

			POINT mousePt;
			mousePt.x = States->MouseState.x;
			mousePt.y = States->MouseState.y;

			if (PtInRect(&rect, mousePt))
			{
				if (States->MouseState.Action == MouseAction::LButtonDbClick || (States->MouseState.Action == MouseAction::LButtonDown && autoSelectAll))
				{
					States->ActiveId = Id;
					if (States->EditingId != Id)
					{
						States->BufferedString = buf;
						States->EditingId = Id;
					}

					CalcCharWidthPrefixSum();

					// Place cursor
					States->CursorPos = Indent + (States->BufferedString.length() > 0 ? *States->StrWidthPrefixSum.rbegin() : 0);
					States->CursorIdx = States->BufferedString.length();

					States->SelectIdx = 0;
				}
				else if (States->MouseState.Action == MouseAction::LButtonDown)
				{
					// Set activated
					States->ActiveId = Id;
					if (States->EditingId != Id)
					{
						States->BufferedString = buf;
						States->EditingId = Id;
					}

					CalcCharWidthPrefixSum();

					// Place cursor
					auto distX = mousePt.x - (States->CurrentPosX + 3);
					auto charIt = std::lower_bound(States->StrWidthPrefixSum.begin(), States->StrWidthPrefixSum.end(), distX);

					States->CursorIdx = ((charIt == States->StrWidthPrefixSum.begin()) ? 0 : charIt - States->StrWidthPrefixSum.begin() - 1);
					States->CursorPos = Indent + ((charIt == States->StrWidthPrefixSum.begin()) ? 0 : *(charIt - 1));

					States->SelectIdx = States->CursorIdx;
				}

				States->HoveredId = Id;
			}
			else if (States->MouseState.Action == MouseAction::LButtonDown || States->MouseState.Action == MouseAction::LButtonDbClick)
			{
				if (States->EditingId == Id)
				{
					States->EditingId = -1;
					buf = States->BufferedString;
				}
				if (States->ActiveId == Id)
					States->ActiveId = -1;
			}

			if (States->MouseState.Action == MouseAction::Move && States->MouseState.lDown && States->ActiveId == Id)
			{
				auto distX = mousePt.x - (States->CurrentPosX + 3);
				auto charIt = std::lower_bound(States->StrWidthPrefixSum.begin(), States->StrWidthPrefixSum.end(), distX);
				States->CursorIdx = ((charIt == States->StrWidthPrefixSum.begin()) ? 0 : charIt - States->StrWidthPrefixSum.begin() - 1);
				States->CursorPos = Indent + ((charIt == States->StrWidthPrefixSum.begin()) ? 0 : *(charIt - 1));
			}

			if (States->ActiveId == Id && States->KeyState.key != char(Key::None))
			{
				switch (States->KeyState.key)
				{
				case char(Key::LeftArrow):
				{
					auto orgIdx = States->CursorIdx--;
					States->CursorIdx = Math::Max(States->CursorIdx, 0);
					States->CursorPos -= States->StrWidthPrefixSum[orgIdx] - States->StrWidthPrefixSum[States->CursorIdx];
					break;
				}
				case char(Key::RightArrow):
				{
					auto orgIdx = States->CursorIdx++;
					States->CursorIdx = Math::Min(States->CursorIdx, States->BufferedString.length());
					States->CursorPos += States->StrWidthPrefixSum[States->CursorIdx] - States->StrWidthPrefixSum[orgIdx];
					break;
				}
				case char(Key::Enter):
				{
					if (States->EditingId == Id && States->ActiveId == Id)
					{
						buf = States->BufferedString;
						if (autoClearOnEnter)
						{
							States->BufferedString = "";

							States->CursorIdx = 0;
							States->CursorPos = Indent;
							CalcCharWidthPrefixSum();
						}
						else
						{
							States->EditingId = -1;
							States->ActiveId = -1;
						}
					}
					break;
				}
				case char(Key::BackSpace) :
					if (States->CursorIdx != States->SelectIdx) // When in selection mode, erase all charactors selected
					{
						auto minIdx = Math::Min(States->CursorIdx, States->SelectIdx);
						auto maxIdx = Math::Max(States->CursorIdx, States->SelectIdx);
						States->BufferedString.erase(minIdx, maxIdx - minIdx);
						States->CursorIdx = minIdx;
						States->CursorPos = Indent + States->StrWidthPrefixSum[minIdx];
						CalcCharWidthPrefixSum();
					}
					else if (States->CursorIdx > 0)
					{
						int shift = States->StrWidthPrefixSum[States->CursorIdx] - States->StrWidthPrefixSum[States->CursorIdx - 1];
						States->BufferedString.erase(States->CursorIdx - 1, 1);

						CalcCharWidthPrefixSum();

						States->CursorPos -= shift;
						States->CursorIdx--;
					}
					break;
				//case char(Key::Delete) :
				//	if (States->CursorIdx < States->BufferedString.length())
				//	{
				//		int indent = States->StrWidthPrefixSum[States->CursorIdx + 1] - States->StrWidthPrefixSum[States->CursorIdx];
				//		States->BufferedString.erase(States->CursorIdx, 1);

				//		CalcCharWidthPrefixSum();
				//	}
				//	break;
				case char(Key::Home):
					States->CursorPos = Indent;
					States->CursorIdx = 0;
					break;
				case char(Key::End) :
					States->CursorPos = *States->StrWidthPrefixSum.rbegin() + Indent;
					States->CursorIdx = States->StrWidthPrefixSum.size() - 1;
					break;

				default:
					if (States->KeyState.key < ' ' || States->KeyState.key > '~' || States->KeyState.ctrlDown)
						break; // Displayable charactors only

					if (States->CursorIdx != States->SelectIdx) // When in selection mode, erase all charactors selected
					{
						auto minIdx = Math::Min(States->CursorIdx, States->SelectIdx);
						auto maxIdx = Math::Max(States->CursorIdx, States->SelectIdx);
						States->BufferedString.erase(minIdx, maxIdx - minIdx);
						States->CursorIdx = minIdx;
						States->CursorPos = Indent + States->StrWidthPrefixSum[minIdx];
						CalcCharWidthPrefixSum();
					}

					SIZE textExtent;
					GetTextExtentPoint32A(GUIPainter::Instance()->GetDC(), &States->KeyState.key, 1, &textExtent);

					if (*States->StrWidthPrefixSum.rbegin() + textExtent.cx >= width - Indent)
						break; // Limit string length to input frame width

					// Insert charactors
					States->BufferedString.insert(States->CursorIdx, 1, States->KeyState.key);

					CalcCharWidthPrefixSum();

					States->CursorPos += textExtent.cx;
					States->CursorIdx++;
				}

				// Terminate selection when any key is pressed
				States->SelectIdx = States->CursorIdx;
			}

			Color color = States->HoveredId == Id && States->ActiveId == -1 || States->ActiveId == Id ? Color(1.0f, 1.0f, 1.0f, 0.65f) : Color(1.0f, 1.0f, 1.0f, 0.5f);
			GUIPainter::Instance()->DrawRect(rect.left,
				rect.top,
				rect.right,
				rect.bottom,
				GUIPainter::DEPTH_MID,
				false, color);

			if (States->HoveredId == Id && States->ActiveId == -1 || States->ActiveId == Id) // Draw high lighted background if active or hovered
			{
				glPushAttrib(GL_COLOR_BUFFER_BIT);
				glBlendFunc(GL_DST_COLOR, GL_CONSTANT_ALPHA);
				GUIPainter::Instance()->DrawRect(rect.left,
					rect.top,
					rect.right,
					rect.bottom,
					GUIPainter::DEPTH_MID,
					true, Color(0.6f));
				glPopAttrib();
			}

			string& renderedStr = States->ActiveId != Id ? buf : States->BufferedString;

			// Draw string
			if (States->SelectIdx == States->CursorIdx || States->ActiveId != Id || States->CursorIdx == States->SelectIdx)
			{
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				GUIPainter::Instance()->DrawString(States->CurrentPosX + 3, States->CurrentPosY + 5, GUIPainter::DEPTH_MID, renderedStr.c_str());
			}
			else
			{
				auto minIdx = Math::Min(States->CursorIdx, States->SelectIdx);
				auto maxIdx = Math::Max(States->CursorIdx, States->SelectIdx);

				GUIPainter::Instance()->DrawRect(States->CurrentPosX + Indent + States->StrWidthPrefixSum[minIdx],
					States->CurrentPosY + 3,
					States->CurrentPosX + Indent + States->StrWidthPrefixSum[maxIdx],
					States->CurrentPosY + 16,
					GUIPainter::DEPTH_MID,
					true,
					color);

				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				GUIPainter::Instance()->DrawString(States->CurrentPosX + 3, States->CurrentPosY + 5, GUIPainter::DEPTH_MID, renderedStr.c_str(), minIdx);
				GUIPainter::Instance()->DrawString(States->CurrentPosX + 3 + States->StrWidthPrefixSum[maxIdx], States->CurrentPosY + 5, GUIPainter::DEPTH_MID, renderedStr.c_str() + maxIdx);

				glColor4f(0.15f, 0.15f, 0.15f, 0.15f);
				glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
				GUIPainter::Instance()->DrawString(States->CurrentPosX + 3 + States->StrWidthPrefixSum[minIdx], States->CurrentPosY + 5, GUIPainter::DEPTH_MID, renderedStr.c_str() + minIdx, maxIdx - minIdx);
			}

			if (States->ActiveId == Id) // Draw cursor
				GUIPainter::Instance()->DrawLine(States->CurrentPosX + States->CursorPos, States->CurrentPosY + 3, States->CurrentPosX + States->CursorPos, States->CurrentPosY + 16, GUIPainter::DEPTH_MID);

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += Height + Padding;
			else
				States->CurrentPosX += 5;

			return false;
		}

		bool EDXGui::InputDigit(int& digit, const char* notation)
		{
			// Print slider text
			Text(notation, "%s: %.2f", notation, digit);
			States->CurrentPosY -= 5;

			// +/- buttons
			auto oldX = States->CurrentPosX;
			auto oldY = States->CurrentPosY;

			States->CurrentPosX += 62;
			if (EDXGui::Button("-", 22, 18))
				digit--;

			States->CurrentPosX += 24;
			States->CurrentPosY = oldY;
			if (EDXGui::Button("+", 22, 18))
				digit++;

			States->CurrentPosX = oldX;
			States->CurrentPosY = oldY;
			char buf[32];
			_itoa(digit, buf, 10);

			string str(buf);
			InputText(str, 60, true);

			digit = atoi(str.c_str());

			return true;
		}

		void EDXGui::Scroller(int limitLen, int actualLen, float& lin)
		{
			int Id = States->CurrentId++;

			const int BarBase = States->CurrentPosY;
			const int Diff = 2;
			const int InnerBase = BarBase + Diff;

			float ratio = limitLen / float(actualLen);
			int scrollBarLen = limitLen * ratio;
			int scrollBarStart = Math::Lerp(InnerBase, BarBase + limitLen - Diff - scrollBarLen, lin);

			const int ScrollBase = InnerBase + scrollBarLen / 2;
			const int ScrollEnd = limitLen - scrollBarLen / 2;

			int scrollPos = (int)Math::Lerp(ScrollBase, ScrollEnd, lin);

			RECT barRect;
			SetRect(&barRect,
				States->DialogWidth - 15,
				BarBase,
				States->DialogWidth - 7,
				BarBase + limitLen);

			POINT mousePt;
			mousePt.x = States->MouseState.x;
			mousePt.y = States->MouseState.y;
			if (PtInRect(&barRect, mousePt))
			{
				if (States->MouseState.Action == MouseAction::LButtonDown)
					States->ActiveId = Id;

				if (States->MouseState.Action == MouseAction::LButtonUp)
				{
					scrollPos = Math::Clamp(States->MouseState.y, ScrollBase, ScrollEnd);
					lin = Math::LinStep(scrollPos, ScrollBase, ScrollEnd);

					if (States->ActiveId == Id)
						States->ActiveId = -1;
				}

				States->HoveredId = Id;
			}

			if (States->MouseState.Action == MouseAction::Move && States->MouseState.lDown)
			{
				if (States->ActiveId == Id)
				{
					scrollPos = Math::Clamp(States->MouseState.y, ScrollBase, ScrollEnd);
					lin = Math::LinStep(scrollPos, ScrollBase, ScrollEnd);
				}
			}


			if (States->MouseState.Action == MouseAction::LButtonUp)
				if (States->ActiveId == Id)
					States->ActiveId = -1;

			Color color = States->HoveredId == Id && States->ActiveId == -1 || States->ActiveId == Id ?
				Color(1.0f, 1.0f, 1.0f, 0.65f) : Color(1.0f, 1.0f, 1.0f, 0.5f);

			GUIPainter::Instance()->DrawRoundedRect(States->DialogWidth - 15,
				BarBase,
				States->DialogWidth - 7,
				BarBase + limitLen,
				GUIPainter::DEPTH_MID,
				4,
				false,
				color);

			GUIPainter::Instance()->DrawRoundedRect(States->DialogWidth - 13,
				scrollBarStart,
				States->DialogWidth - 9,
				scrollBarStart + scrollBarLen,
				GUIPainter::DEPTH_MID,
				2,
				true,
				color);
		}

		void EDXGui::BeginScrollableArea(int areaHeight, int& contentHeight, float& scroller)
		{
			glEnable(GL_SCISSOR_TEST);
			glScissor(States->DialogPosX,
				States->ScreenHeight - (States->DialogPosY + States->CurrentPosY + areaHeight),
				States->DialogWidth,
				areaHeight);

			if (contentHeight > areaHeight)
				Scroller(areaHeight, contentHeight, scroller);

			States->ScrollerInitY = Math::Lerp(States->CurrentPosY, States->CurrentPosY - (contentHeight - areaHeight), scroller);
			States->OrigY = States->CurrentPosY;
			States->CurrentPosY = States->ScrollerInitY;
		}

		void EDXGui::EndScrollableArea(int areaHeight, int& contentHeight, float& scroller)
		{
			contentHeight = States->CurrentPosY - States->ScrollerInitY;
			States->CurrentPosY = States->OrigY + areaHeight + Padding;

			glDisable(GL_SCISSOR_TEST);
		}

		void EDXGui::Console(const char* str,
			const int x,
			const int y,
			const int width,
			const int height)
		{
			const int TotalPadding = 80;

			BeginDialog(LayoutStrategy::Floating, x, y, width, height);
			{
				static float scroller = 0.0f;
				static int contentHeight = 0;
				BeginScrollableArea(height - TotalPadding, contentHeight, scroller);

				MultilineText(States->ConsoleTextBuffer.c_str());

				EndScrollableArea(height - TotalPadding, contentHeight, scroller);

				glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
				GUIPainter::Instance()->DrawLine(States->CurrentPosX - 10,
					States->CurrentPosY,
					States->DialogWidth - 20,
					States->CurrentPosY,
					GUIPainter::DEPTH_MID);

				States->CurrentPosY += Padding;
				auto inputY = States->CurrentPosY;

				int textId = States->CurrentId;
				bool active = States->CurrentId == States->ActiveId;
				static string inputTextBuffer;
				InputText(inputTextBuffer, width - 120, false, true);

				States->CurrentPosX += width - 110;
				States->CurrentPosY = inputY;
				if (Button("Enter", 70, 19) || States->KeyState.key == char(Key::Enter) && active)
				{
					ConsoleCommand(inputTextBuffer.c_str());
					inputTextBuffer = "";
				}
			}
			EndDialog();
		}

		void EDXGui::ConsoleCommand(const char* command)
		{
			States->ConsoleTextBuffer += command;
			States->ConsoleTextBuffer += '\n';
		}
	}
}