
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
					sample += weights[i] * texture2DLod(texSampler, texCoord + offsets[i], 3);
				}
				gl_FragColor = vec4(sample.rgb, 1.0);
			})";

		GUIPainter::GUIPainter()
		{
			// Initialize font
			HFONT	font;
			HFONT	oldfont;

			mTextListBase = glGenLists(128);
			font = CreateFont(16,
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

			HDC hDC = GetDC(Application::GetMainWindow()->GetHandle());
			oldfont = (HFONT)SelectObject(hDC, font);
			wglUseFontBitmaps(hDC, 0, 128, mTextListBase);

			SelectObject(hDC, oldfont);
			DeleteObject(font);

			// Load shaders
			mVertexShader.Load(ShaderType::VertexShader, ScreenQuadVertShaderSource);
			mBlurFragmentShader.Load(ShaderType::FragmentShader, GaussianBlurFragShaderSource);
			mProgram.AttachShader(&mVertexShader);
			mProgram.AttachShader(&mBlurFragmentShader);
			mProgram.Link();
		}

		void GUIPainter::Resize(int width, int height)
		{
			mFBWidth = width;
			mFBHeight = height;

			// Init background texture
			mColorRBO.SetStorage(width >> 3, height >> 3, ImageFormat::RGBA);
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

			glViewport(0, 0, mFBWidth >> 3, mFBHeight >> 3);
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

			glBlitFramebuffer(x0 >> 3, y0 >> 3, x1 >> 3, y1 >> 3, x0, y0, x1, y1, GL_COLOR_BUFFER_BIT, GL_LINEAR);

			mFBO.UnBind();
		}

		void GUIPainter::DrawBorderedRect(int iX0, int iY0, int iX1, int iY1, float depth, int iBorderSize, const Color& interiorColor, const Color& borderColor)
		{
			if (iBorderSize > 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				glColor4f(borderColor.r, borderColor.g, borderColor.b, 0.5f);
				DrawRect(iX0, iY0, iX1, iY1, depth);

				iX0 += iBorderSize;
				iX1 -= iBorderSize;
				iY0 += iBorderSize;
				iY1 -= iBorderSize;

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				glColor4f(interiorColor.r, interiorColor.g, interiorColor.b, 0.5f);
				DrawRect(iX0, iY0, iX1, iY1, depth);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				glColor4f(interiorColor.r, interiorColor.g, interiorColor.b, 0.5f);
				DrawRect(iX0, iY0, iX1, iY1, depth);
			}
		}

		void GUIPainter::DrawRect(int iX0, int iY0, int iX1, int iY1, float depth)
		{
			glBegin(GL_QUADS);

			glVertex3f(iX0, iY0, depth);
			glVertex3f(iX1, iY0, depth);
			glVertex3f(iX1, iY1, depth);
			glVertex3f(iX0, iY1, depth);

			glEnd();
		}

		void GUIPainter::DrawLineStrip(int iX0, int iY0, int iX1, int iY1)
		{
			glBegin(GL_LINE_STRIP);

			glVertex2i(iX0, iY0);
			glVertex2i(iX1, iY0);
			glVertex2i(iX1, iY1);
			glVertex2i(iX0, iY1);

			glEnd();
		}

		void GUIPainter::DrawString(int x, int y, float depth, const char* strText)
		{
			glListBase(mTextListBase);

			glRasterPos3f(x, y + 10, depth);
			glCallLists((GLsizei)strlen(strText), GL_UNSIGNED_BYTE, strText);
		}

		void GUIPainter::CalcGaussianBlurWeightsAndOffsets()
		{
			auto GaussianDistribution = [](float x, float y, float rho) -> float
			{
				float g = 1.0f / Math::Sqrt(2.0f * float(Math::EDX_PI) * rho * rho);
				g *= Math::Exp(-(x * x + y * y) / (2 * rho * rho));

				return g;
			};

			float tu = 1.0f / (float)mFBWidth * 8;
			float tv = 1.0f / (float)mFBHeight * 8;

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
		// Dialog implementation
		//----------------------------------------------------------------------------------
		void EDXDialog::Init(int iParentWidth, int iParentHeight)
		{
			mParentWidth = iParentWidth;
			mParentHeight = iParentHeight;

			mWidth = 200;
			mHeight = iParentHeight;
			mPosX = mParentWidth - mWidth;
			mPosY = 0;
		}

		void EDXDialog::Render() const
		{
			if (!mVisible)
				return;

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, mParentWidth, 0, mParentHeight, 1, -1);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			// Render the blurred background texture
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glEnable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			GUIPainter::Instance()->BlurBackgroundTexture(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight);
			GUIPainter::Instance()->DrawBackgroundTexture(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight);

			glTranslatef(mPosX, mParentHeight - mPosY, 0.0f);
			glScalef(1.0f, -1.0f, 1.0f);

			glLineWidth(1.0f);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_CONSTANT_ALPHA);
			glBlendColor(1.0f, 1.0f, 1.0f, 0.5f);

			glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
			GUIPainter::Instance()->DrawRect(0, 0, mWidth, mHeight, GUIPainter::DEPTH_FAR);

			for (int i = 0; i < mvControls.size(); i++)
			{
				mvControls[i]->Render();
			}

			glPopAttrib();
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
		}

		void EDXDialog::Resize(int width, int height)
		{
			mParentWidth = width;
			mParentHeight = height;

			mPosX = mParentWidth - mWidth;
			mPosY = 0;
			mHeight = height;

			GUIPainter::Instance()->Resize(width, height);
		}

		bool EDXDialog::HandleKeyboard(const KeyboardEventArgs& args)
		{
			if (args.key == 'U')
			{
				ToggleVisible();
				return true;
			}

			return false;
		}

		void EDXDialog::Release()
		{
			mvControls.clear();
			GUIPainter::DeleteInstance();
		}

		bool EDXDialog::AddButton(uint ID, char* pStr)
		{
			int posY = mPaddingY + (Button::Padding - Button::Height) / 2;
			Button* pButton = new Button(ID, mPaddingX, posY, Button::Width, Button::Height, pStr, this);
			if (!pButton)
			{
				return false;
			}

			pButton->UpdateRect();
			mvControls.push_back(pButton);
			mPaddingY += Button::Padding;

			return true;
		}

		bool EDXDialog::AddSlider(uint ID, float min, float max, float val, float* pRefVal, const char* pText)
		{
			char str[256];
			sprintf_s(str, "%s%.2f", pText, val);
			AddText(999, str);

			int posY = mPaddingY + (Slider::Padding - Slider::Height) / 2;
			Slider* pSlider = new Slider(ID, mPaddingX, posY, Slider::Width, Slider::Height, min, max, val, pRefVal, pText, this);
			if (!pSlider)
			{
				return false;
			}

			pSlider->UpdateRect();
			pSlider->SetTextControl((Text*)mvControls.back().Ptr());

			mvControls.push_back(pSlider);
			mPaddingY += Slider::Padding;

			return true;
		}

		bool EDXDialog::AddCheckBox(uint ID, bool bChecked, bool* pRefVal, char* pStr)
		{
			int posY = mPaddingY + (CheckBox::Padding - CheckBox::Height) / 2;
			CheckBox* pCheckedBox = new CheckBox(ID, mPaddingX, posY, CheckBox::Width, CheckBox::Height, bChecked, pRefVal, pStr, this);
			if (!pCheckedBox)
			{
				return false;
			}

			pCheckedBox->UpdateRect();
			mvControls.push_back(pCheckedBox);
			mPaddingY += CheckBox::Padding;

			return true;
		}

		bool EDXDialog::AddComboBox(uint iID, int initSelectedIdx, int* pRefVal, ComboBoxItem* pItems, int numItems)
		{
			int posY = mPaddingY + (ComboBox::Padding - ComboBox::Height) / 2;
			ComboBox* pComboBox = new ComboBox(iID, mPaddingX, posY, ComboBox::Width, ComboBox::Height, initSelectedIdx, pRefVal, pItems, numItems, this);

			pComboBox->UpdateRect();
			mvControls.push_back(pComboBox);
			mPaddingY += ComboBox::Padding;

			return true;
		}

		bool EDXDialog::AddText(uint ID, const char* pStr)
		{
			int posY = mPaddingY + (Text::Padding - Text::Height) / 2;
			Text* pText = new Text(ID, mPaddingX, posY, Text::Width, Text::Height, pStr, this);
			if (!pText)
			{
				return false;
			}

			pText->UpdateRect();
			mvControls.push_back(pText);
			mPaddingY += Text::Padding;

			return true;
		}

		EDXControl* EDXDialog::GetControlAtPoint(const POINT& pt) const
		{
			for (int i = 0; i < mvControls.size(); i++)
			{
				EDXControl* pControl = mvControls[i].Ptr();

				if (pControl->ContainsPoint(pt))
				{
					return pControl;
				}
			}
			return NULL;
		}

		EDXControl* EDXDialog::GetControlWithID(uint ID) const
		{
			for (int i = 0; i < mvControls.size(); i++)
			{
				EDXControl* pControl = mvControls[i].Ptr();

				if (pControl->GetID() == ID)
					return pControl;
			}
			return NULL;
		}

		void EDXDialog::SendEvent(EDXControl* pControl)
		{
			if (!mCallbackEvent.Attached())
				return;

			mCallbackEvent.Invoke(pControl, EventArgs());
		}


		bool EDXDialog::MsgProc(const MouseEventArgs& mouseArgs)
		{
			if (!mVisible)
				return false;

			MouseEventArgs offsettedArgs = mouseArgs;
			offsettedArgs.x -= mPosX;
			offsettedArgs.y -= mPosY;

			if (mpFocusControl)
			{
				if (mpFocusControl->HandleMouse(offsettedArgs))
					return true;
				else if (offsettedArgs.Action == MouseAction::LButtonDown)
					mpFocusControl->ResetFocus();
			}

			POINT mousePt;
			mousePt.x = offsettedArgs.x;
			mousePt.y = offsettedArgs.y;
			EDXControl* pControl = GetControlAtPoint(mousePt);
			if (pControl)
			{
				if (mpHoveredControl != pControl)
				{
					if (mpHoveredControl)
						mpHoveredControl->OnMouseOut();
					mpHoveredControl = pControl;
					mpHoveredControl->OnMouseIn();
				}

				if (offsettedArgs.Action == MouseAction::LButtonDown)
					pControl->SetFocus();

				if (pControl->HandleMouse(offsettedArgs))
					return true;
			}
			else
			{
				if (mpHoveredControl)
				{
					mpHoveredControl->OnMouseOut();
					mpHoveredControl = nullptr;
				}
			}

			return false;
		}

		//----------------------------------------------------------------------------------
		// Button implementation
		//----------------------------------------------------------------------------------
		Button::Button(uint iID, int iX, int iY, int iWidth, int iHeight, char* pStr, EDXDialog* pDiag)
			: EDXControl(iID, iX, iY, iWidth, iHeight, pDiag)
			, mbDown(false)
			, mPressed(false)
		{
			strcpy_s(mstrText, 256, pStr);
		}

		void Button::Render() const
		{
			if (mbDown)
			{
				GUIPainter::Instance()->DrawBorderedRect(mBBox.left + 1, mBBox.top + 1, mBBox.right - 1, mBBox.bottom - 1, GUIPainter::DEPTH_MID, 0, Color::WHITE);
			}
			else if (mHovered)
			{
				GUIPainter::Instance()->DrawBorderedRect(mBBox.left - 1, mBBox.top - 1, mBBox.right + 1, mBBox.bottom + 1, GUIPainter::DEPTH_MID, 0, Color::WHITE);
			}
			else
			{
				GUIPainter::Instance()->DrawBorderedRect(mBBox.left, mBBox.top, mBBox.right, mBBox.bottom, GUIPainter::DEPTH_MID, 2);
			}

			int midX = mX + mWidth / 2 - strlen(mstrText) * 7 / 2;
			int midY = mY + mHeight / 2;


			glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
			if (mbDown || mHovered)
				glColor4f(0.15f, 0.15f, 0.15f, 1.0f);
			else
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			GUIPainter::Instance()->DrawString(midX, midY + 1, GUIPainter::DEPTH_MID, mstrText);
		}

		bool Button::HandleMouse(const MouseEventArgs& mouseArgs)
		{
			POINT mousePt;
			mousePt.x = mouseArgs.x;
			mousePt.y = mouseArgs.y;

			switch (mouseArgs.Action)
			{
			case MouseAction::LButtonDown:
				if (PtInRect(&mBBox, mousePt))
				{
					mbDown = true;
					mPressed = true;
					return true;
				}
				break;

			case MouseAction::LButtonUp:
				if (PtInRect(&mBBox, mousePt) && mbDown)
				{
					Trigger();
				}
				mbDown = false;
				mPressed = false;

				return true;

				break;

			case MouseAction::Move:
				if (PtInRect(&mBBox, mousePt))
				{
					if (mPressed)
					{
						mbDown = true;
						return true;
					}
				}
				else if (mPressed)
				{
					mbDown = false;
					return true;
				}

				break;
			}

			return false;
		}

		//----------------------------------------------------------------------------------
		// Slider implementation
		//----------------------------------------------------------------------------------
		Slider::Slider(uint iID, int iX, int iY, int iWidth, int iHeight, float min, float max, float val, float* pRefVal, const char* pText, EDXDialog* pDiag)
			: EDXControl(iID, iX, iY, iWidth, iHeight, pDiag)
			, mMin(min)
			, mMax(max)
			, mVal(Math::Clamp(val, min, max))
			, mPressed(false)
			, mButtonSize(6)
			, mpRefVal(pRefVal)
		{
			mSlideBase = mX + mButtonSize;
			mSlideEnd = mX + mWidth - mButtonSize;
			strcpy_s(mMainText, 256, pText);
		}

		void Slider::Render() const
		{
			int iY = mY + mHeight / 2;

			float fLerp = Math::LinStep(mVal, mMin, mMax);
			int iButPos = (int)Math::Lerp(mSlideBase, mSlideEnd, fLerp);

			GUIPainter::Instance()->DrawBorderedRect(mX, iY - 1, iButPos - mButtonSize, iY + 2, GUIPainter::DEPTH_MID, 0, Color::WHITE);
			glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
			glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
			glBegin(GL_LINE_STRIP);

			glVertex2i(iButPos + mButtonSize, iY - 1);
			glVertex2i(mX + mWidth, iY - 1);
			glVertex2i(mX + mWidth, iY + 1);
			glVertex2i(iButPos + mButtonSize, iY + 1);

			glEnd();

			GUIPainter::Instance()->DrawBorderedRect(iButPos - mButtonSize, iY - mButtonSize, iButPos + mButtonSize, iY + mButtonSize, GUIPainter::DEPTH_MID, 0, Color::WHITE);
		}

		void Slider::UpdateRect()
		{
			EDXControl::UpdateRect();

			float fLerp = Math::LinStep(mVal, mMin, mMax);
			mButtonX = (int)Math::Lerp(mSlideBase, mSlideEnd, fLerp);

			int mid = mY + mHeight / 2;
			SetRect(&mrcButtonBBox, mButtonX - mButtonSize, mid - mButtonSize, mButtonX + mButtonSize, mid + mButtonSize);
		}

		void Slider::SetValue(float fValue)
		{
			float clampedVal = Math::Clamp(fValue, mMin, mMax);

			if (clampedVal == mVal)
			{
				return;
			}

			mVal = clampedVal;
			sprintf_s(mValuedText, 256, "%s%.2f", mMainText, mVal);
			mpText->SetText(mValuedText);

			UpdateRect();

			mpDialog->SendEvent(this);
		}

		void Slider::SetValueFromPos(int iPos)
		{
			float fLerp = Math::LinStep(float(iPos), mSlideBase, mSlideEnd);
			fLerp = Math::Clamp(fLerp, 0.0f, 1.0f);
			float fVal = Math::Lerp(mMin, mMax, fLerp);

			SetValue(fVal);
		}

		bool Slider::HandleMouse(const MouseEventArgs& mouseArgs)
		{
			POINT mousePt;
			mousePt.x = mouseArgs.x;
			mousePt.y = mouseArgs.y;

			switch (mouseArgs.Action)
			{
			case MouseAction::LButtonDown:
			case MouseAction::LButtonDbClick:
				if (PtInRect(&mrcButtonBBox, mousePt))
				{
					mPressed = true;

					mDragX = mousePt.x;
					mDragOffset = mButtonX - mDragX;

					return true;
				}
				else if (PtInRect(&mBBox, mousePt))
				{
					SetValueFromPos(mousePt.x);
					*mpRefVal = GetValue();
					return true;
				}
				break;

			case MouseAction::LButtonUp:
				mPressed = false;
				mDragOffset = 0;
				mpDialog->SendEvent(this);

				return true;

			case MouseAction::Move:
				if (mPressed)
				{
					SetValueFromPos(mousePt.x + mDragOffset);
					*mpRefVal = GetValue();
					return true;
				}
				break;
			}

			return false;
		}

		//----------------------------------------------------------------------------------
		// CheckBox implementation
		//----------------------------------------------------------------------------------
		CheckBox::CheckBox(uint iID, int iX, int iY, int iWidth, int iHeight, bool bChecked, bool* pRefVal, char* pStr, EDXDialog* pDiag)
			: EDXControl(iID, iX, iY, iWidth, iHeight, pDiag)
			, mbChecked(bChecked)
			, mPressed(false)
			, mBoxSize(6)
			, mpRefVal(pRefVal)
		{
			strcpy_s(mstrText, 256, pStr);
		}

		void CheckBox::Render() const
		{
			int midX = mX + 6;
			int midY = mY + mHeight / 2;
			GUIPainter::Instance()->DrawBorderedRect(midX - mBoxSize, midY - mBoxSize, midX + mBoxSize, midY + mBoxSize, GUIPainter::DEPTH_MID, 2);
			if (mbChecked)
				GUIPainter::Instance()->DrawBorderedRect(midX - mBoxSize + 1, midY - mBoxSize + 2, midX + mBoxSize - 2, midY + mBoxSize - 1, GUIPainter::DEPTH_MID, 0, Color::WHITE);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(midX + mBoxSize + 2, midY + 1, GUIPainter::DEPTH_MID, mstrText);
		}

		void CheckBox::UpdateRect()
		{
			EDXControl::UpdateRect();

			int midX = mX + 6;
			int midY = mY + mHeight / 2;
			SetRect(&mrcBoxBBox, midX - mBoxSize, midY - mBoxSize, midX + mBoxSize, midY + mBoxSize);
		}

		bool CheckBox::HandleMouse(const MouseEventArgs& mouseArgs)
		{
			POINT mousePt;
			mousePt.x = mouseArgs.x;
			mousePt.y = mouseArgs.y;

			switch (mouseArgs.Action)
			{
			case MouseAction::LButtonDown:
			case MouseAction::LButtonDbClick:
				if (PtInRect(&mrcBoxBBox, mousePt))
				{
					mPressed = true;
					return true;
				}
				break;

			case MouseAction::LButtonUp:
				if (PtInRect(&mrcBoxBBox, mousePt) && mPressed)
				{
					Toggle();
					mPressed = false;
					*mpRefVal = GetChecked();
					mpDialog->SendEvent(this);
					return true;
				}
				break;
			}

			return false;
		}

		//----------------------------------------------------------------------------------
		// Text implementation
		//----------------------------------------------------------------------------------
		Text::Text(uint iID, int iX, int iY, int iWidth, int iHeight, const char* pStr, EDXDialog* pDiag)
			: EDXControl(iID, iX, iY, iWidth, iHeight, pDiag)
		{
			strcpy_s(mstrText, 256, pStr);
		}

		void Text::Render() const
		{
			int imdY = mY + mHeight / 2;

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(mX, imdY, GUIPainter::DEPTH_MID, mstrText);
		}

		void Text::SetText(char* pStr)
		{
			if (pStr == NULL)
			{
				mstrText[0] = 0;
				return;
			}

			strcpy_s(mstrText, 256, pStr);
		}

		//----------------------------------------------------------------------------------
		// ComboBox implementation
		//----------------------------------------------------------------------------------
		ComboBox::ComboBox(uint iID, int iX, int iY, int iWidth, int iHeight, int initSelectedIdx, int* pRefVal, ComboBoxItem* pItems, int numItems, EDXDialog* pDiag)
			: EDXControl(iID, iX, iY, iWidth, iHeight, pDiag)
			, mButtonSize(8)
			, mpRefVal(pRefVal)
		{
			mpItems = new ComboBoxItem[numItems];
			memcpy(mpItems, pItems, numItems * sizeof(ComboBoxItem));

			mNumItems = numItems;
			mSelectedIdx = initSelectedIdx;
		}

		void ComboBox::UpdateRect()
		{
			EDXControl::UpdateRect();
			mBoxMain = mBBox;

			SetRect(&mBoxDropdown, mX, mY + mHeight, mX + mWidth - mHeight, mY + mHeight + 1 + mNumItems * ItemHeight);
		}

		void ComboBox::Render() const
		{
			GUIPainter::Instance()->DrawBorderedRect(mBoxMain.left, mBoxMain.top, mBoxMain.right, mBoxMain.bottom, GUIPainter::DEPTH_MID, 2);
			GUIPainter::Instance()->DrawBorderedRect(mBoxMain.right - mHeight, mBoxMain.top + 1, mBoxMain.right - 1, mBoxMain.bottom, GUIPainter::DEPTH_MID, 0, Color::WHITE);

			glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(mX + 6, mBoxMain.top + ItemHeight / 2, GUIPainter::DEPTH_MID, mpItems[mSelectedIdx].Label);

			if (mOpened && HasFocus())
			{
				GUIPainter::Instance()->DrawBorderedRect(mBoxDropdown.left, mBoxDropdown.top + 1, mBoxDropdown.right, mBoxDropdown.bottom, GUIPainter::DEPTH_NEAR, 0, 0.5f * Color::WHITE);

				int midX = mX + 6;
				int midY = mBoxDropdown.top + 1 + ItemHeight / 2;
				for (auto i = 0; i < mNumItems; i++)
				{
					if (i == mHoveredIdx)
					{
						GUIPainter::Instance()->DrawBorderedRect(mBoxDropdown.left, mBoxDropdown.top + 2 + mHoveredIdx * ItemHeight, mBoxDropdown.right - 1, mBoxDropdown.top + 1 + (mHoveredIdx + 1) * ItemHeight, GUIPainter::DEPTH_NEAR, 0, 0.85f * Color::WHITE);

						glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
						glColor4f(0.15f, 0.15f, 0.15f, 1.0f);
					}
					else
					{
						glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
						glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
					}

					GUIPainter::Instance()->DrawString(midX, midY + i * ItemHeight, GUIPainter::DEPTH_NEAR, mpItems[i].Label);
				}
			}
		}

		bool ComboBox::HandleMouse(const MouseEventArgs& mouseArgs)
		{
			POINT mousePt;
			mousePt.x = mouseArgs.x;
			mousePt.y = mouseArgs.y;

			switch (mouseArgs.Action)
			{
			case MouseAction::LButtonDown:
			case MouseAction::LButtonDbClick:
				if (PtInRect(&mBBox, mousePt))
				{
					if (mOpened)
					{
						mSelectedIdx = (mousePt.y - mBBox.top) / ItemHeight;
						*mpRefVal = mpItems[mSelectedIdx].Value;
						mpDialog->SendEvent(this);
					}

					mOpened = !mOpened;
					if (mOpened)
					{
						mBBox = mBoxDropdown;
						mHoveredIdx = mSelectedIdx;
					}
					else
					{
						mBBox = mBoxMain;
					}
					return true;
				}
				if (PtInRect(&mBoxMain, mousePt))
				{
					mOpened = !mOpened;
					mBBox = mBoxMain;
					return true;
				}
				break;

			case MouseAction::Move:
				if (PtInRect(&mBBox, mousePt))
				{
					if (mOpened)
						mHoveredIdx = (mousePt.y - mBBox.top) / ItemHeight;
				}
				break;
			}

			return false;
		}

		//----------------------------------------------------------------------------------
		// Immediate mode GUI implementation
		//----------------------------------------------------------------------------------
		GuiStates* EDXGui::States;

		void EDXGui::Init()
		{
			States = new GuiStates;
			States->ActiveId = -1;
		}

		void EDXGui::Release()
		{
			SafeDelete(States);
			GUIPainter::DeleteInstance();
		}

		void EDXGui::BeginDialog(LayoutStrategy layoutStrategy)
		{
			States->CurrentLayoutStrategy = layoutStrategy;
			States->CurrentGrowthStrategy = GrowthStrategy::Vertical;
			States->CurrentId = -1;
			States->HoveredId = -1;

			if (States->CurrentLayoutStrategy == LayoutStrategy::DockRight)
			{
				States->DialogWidth = 200;
				States->DialogHeight = States->ScreenHeight;
				States->DialogPosX = States->ScreenWidth - States->DialogWidth;
				States->DialogPosY = 0;
				States->CurrentPosX = 30;
				States->CurrentPosY = 30;
			}

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
			GUIPainter::Instance()->BlurBackgroundTexture(States->DialogPosX, States->DialogPosY, States->DialogPosX + States->DialogWidth, States->DialogPosY + States->DialogHeight);
			GUIPainter::Instance()->DrawBackgroundTexture(States->DialogPosX, States->DialogPosY, States->DialogPosX + States->DialogWidth, States->DialogPosY + States->DialogHeight);

			glTranslatef(States->DialogPosX, States->ScreenHeight - States->DialogPosY, 0.0f);
			glScalef(1.0f, -1.0f, 1.0f);

			glLineWidth(1.0f);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_CONSTANT_ALPHA);
			glBlendColor(1.0f, 1.0f, 1.0f, 0.5f);

			// Draw blurred background
			glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
			GUIPainter::Instance()->DrawRect(0, 0, States->ScreenWidth, States->ScreenHeight, GUIPainter::DEPTH_FAR);
		}
		void EDXGui::EndDialog()
		{
			glPopAttrib();
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			if (States->CurrentLayoutStrategy == LayoutStrategy::DockRight)
			{
				States->DialogWidth = 200;
				States->DialogHeight = States->ScreenHeight;
				States->DialogPosX = States->ScreenWidth - States->DialogWidth;
				States->DialogPosY = 0;
				States->CurrentPosX = 30;
				States->CurrentPosY = 30;
			}
		}

		void EDXGui::Resize(int screenWidth, int screenHeight)
		{
			States->ScreenWidth = screenWidth;
			States->ScreenHeight = screenHeight;

			GUIPainter::Instance()->Resize(screenWidth, screenHeight);
		}

		void EDXGui::HandleMouseEvent(const MouseEventArgs& mouseArgs)
		{
			States->MouseState = mouseArgs;
			States->MouseState.x = mouseArgs.x - States->DialogPosX;
			States->MouseState.y = mouseArgs.y - States->DialogPosY;

			if (States->MouseState.Action == MouseAction::LButtonUp)
				States->ActiveId = -1;
		}

		void EDXGui::Text(const char* str, ...)
		{
			const int Height = 10;
			const int Width = 140;

			++States->CurrentId;

			va_list args;
			va_start(args, str);

			char buff[1024];
			int size = vsnprintf(buff, sizeof(buff) - 1, str, args);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(States->CurrentPosX, States->CurrentPosY, 0.5f, buff);

			va_end(args);

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += Height + 10;
			else
				States->CurrentPosX += 5;
		}

		bool EDXGui::Bottun(const char* str)
		{
			const int Width = 140;
			const int Height = 22;

			bool trigger = false;
			int Id = ++States->CurrentId;

			RECT btnRect;
			SetRect(&btnRect, States->CurrentPosX, States->CurrentPosY, States->CurrentPosX + Width, States->CurrentPosY + Height);

			POINT mousePt;
			mousePt.x = States->MouseState.x;
			mousePt.y = States->MouseState.y;

			bool inRect = PtInRect(&btnRect, mousePt);

			if (inRect)
			{
				if (States->MouseState.Action == MouseAction::LButtonDown)
					States->ActiveId = Id;
				if (States->MouseState.Action == MouseAction::LButtonUp)
				{
					if (States->ActiveId == Id)
					{
						States->ActiveId = -1;
						trigger = true;
					}
				}
			}

			if (inRect && States->ActiveId == Id)
			{
				GUIPainter::Instance()->DrawBorderedRect(btnRect.left + 1,
					btnRect.top + 1,
					btnRect.right - 1,
					btnRect.bottom - 1,
					GUIPainter::DEPTH_MID,
					0,
					Color::WHITE);

				glColor4f(0.15f, 0.15f, 0.15f, 0.15f);
			}
			else if (inRect && States->ActiveId == -1)
			{
				GUIPainter::Instance()->DrawBorderedRect(btnRect.left - 1,
					btnRect.top - 1,
					btnRect.right + 1,
					btnRect.bottom + 1,
					GUIPainter::DEPTH_MID,
					0,
					Color::WHITE);

				glColor4f(0.15f, 0.15f, 0.15f, 0.15f);
			}
			else
			{
				GUIPainter::Instance()->DrawBorderedRect(btnRect.left,
					btnRect.top,
					btnRect.right,
					btnRect.bottom,
					GUIPainter::DEPTH_MID,
					2);

				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			}


			glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
			int midX = States->CurrentPosX + Width / 2 - strlen(str) * 7 / 2;
			GUIPainter::Instance()->DrawString(midX, States->CurrentPosY + 6, GUIPainter::DEPTH_MID, str);

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += Height + 10;
			else
				States->CurrentPosX += 5;

			return trigger;
		}

		void EDXGui::CheckBox(const char* str, bool& checked)
		{
			const int Width = 140;
			const int BoxSize = 12;

			int Id = ++States->CurrentId;

			RECT boxRect;
			SetRect(&boxRect, States->CurrentPosX, States->CurrentPosY, States->CurrentPosX + BoxSize, States->CurrentPosY + BoxSize);

			POINT mousePt;
			mousePt.x = States->MouseState.x;
			mousePt.y = States->MouseState.y;
			bool inRect = PtInRect(&boxRect, mousePt);

			if (inRect)
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
			}
			else
			{
				if (States->MouseState.Action == MouseAction::Move)
					if (States->ActiveId == Id)
						States->ActiveId = -1;
			}

			GUIPainter::Instance()->DrawBorderedRect(boxRect.left,
				boxRect.top,
				boxRect.right,
				boxRect.bottom,
				GUIPainter::DEPTH_MID,
				2);
			if (checked)
				GUIPainter::Instance()->DrawBorderedRect(boxRect.left + 1,
				boxRect.top + 2,
				boxRect.right - 2,
				boxRect.bottom - 1,
				GUIPainter::DEPTH_MID,
				0,
				Color::WHITE);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(States->CurrentPosX + BoxSize + 7, States->CurrentPosY + 2, GUIPainter::DEPTH_MID, str);

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += BoxSize + 10;
			else
				States->CurrentPosX += 5;
		}

		void EDXGui::ComboBox(const ComboBoxItem* pItems, int& selected)
		{
			static const int Padding = 28;
			static const int Width = 140;
			static const int Height = 18;
			static const int ItemHeight = 20;

			int Id = ++States->CurrentId;
		}
	}
}