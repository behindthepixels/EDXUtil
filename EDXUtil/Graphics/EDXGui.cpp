
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
				glColor4f(borderColor.r, borderColor.g, borderColor.b, borderColor.a);
				DrawRect(iX0, iY0, iX1, iY1, depth);

				//iX0 += iBorderSize;
				//iX1 -= iBorderSize;
				//iY0 += iBorderSize;
				//iY1 -= iBorderSize;

				//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				//glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				//glColor4f(interiorColor.r, interiorColor.g, interiorColor.b, interiorColor.a);
				//DrawRect(iX0, iY0, iX1, iY1, depth);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				glColor4f(interiorColor.r, interiorColor.g, interiorColor.b, interiorColor.a);
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

		void GUIPainter::DrawChar(int x, int y, float depth, const char ch)
		{
			glListBase(mTextListBase);

			glRasterPos3f(x, y + 10, depth);
			glCallLists(1, GL_UNSIGNED_BYTE, &ch);
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
			States->KeyState.key = char(Key::None);
			States->Selecting = false;
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
			States->CurrentId = 0;
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

			States->MouseState.Action = MouseAction::None;
			States->KeyState.key = char(Key::None);
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

			//if (States->MouseState.Action == MouseAction::LButtonUp)
			//{
			//	if (States->HoveredId != States->ActiveId && !States->Selecting)
			//		States->ActiveId = -1;

			//	//States->Selecting = false;
			//}
		}

		void EDXGui::HandleKeyboardEvent(const KeyboardEventArgs& keyArgs)
		{
			States->KeyState = keyArgs;
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
				States->CurrentPosY += Height + Padding;
			else
				States->CurrentPosX += 5;
		}

		bool EDXGui::Bottun(const char* str)
		{
			const int Width = 140;
			const int Height = 22;

			bool trigger = false;
			int Id = States->CurrentId++;

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

				States->HoveredId = Id;
			}

			if (States->HoveredId == Id && States->ActiveId == Id)
			{
				GUIPainter::Instance()->DrawBorderedRect(btnRect.left + 1,
					btnRect.top + 1,
					btnRect.right - 1,
					btnRect.bottom - 1,
					GUIPainter::DEPTH_MID,
					0,
					Color(1.0f, 1.0f, 1.0f, 0.65f));

				glColor4f(0.15f, 0.15f, 0.15f, 0.15f);
			}
			else if (States->HoveredId == Id && States->ActiveId == -1 || States->ActiveId == Id)
			{
				GUIPainter::Instance()->DrawBorderedRect(btnRect.left - 1,
					btnRect.top - 1,
					btnRect.right + 1,
					btnRect.bottom + 1,
					GUIPainter::DEPTH_MID,
					0,
					Color(1.0f, 1.0f, 1.0f, 0.5f));

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
				States->CurrentPosY += Height + Padding;
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

			GUIPainter::Instance()->DrawBorderedRect(boxRect.left,
				boxRect.top,
				boxRect.right,
				boxRect.bottom,
				GUIPainter::DEPTH_MID,
				2);

			Color color = checked ? Color(1.0f, 1.0f, 1.0f, 0.5f) : States->HoveredId == Id && States->ActiveId == -1 ? Color(1.0f, 1.0f, 1.0f, 0.15f) : Color::BLACK;
			GUIPainter::Instance()->DrawBorderedRect(boxRect.left + 1,
				boxRect.top + 2,
				boxRect.right - 2,
				boxRect.bottom - 1,
				GUIPainter::DEPTH_MID,
				0,
				color);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(States->CurrentPosX + BoxSize + 7, States->CurrentPosY + 2, GUIPainter::DEPTH_MID, str);

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += BoxSize + 10;
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
			SetRect(&mainRect, States->CurrentPosX, States->CurrentPosY, States->CurrentPosX + Width, States->CurrentPosY + Height);
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

			GUIPainter::Instance()->DrawBorderedRect(mainRect.left, mainRect.top, mainRect.right, mainRect.bottom, GUIPainter::DEPTH_MID, 2);

			Color btnColor = States->ActiveId == Id || States->HoveredId == Id && States->ActiveId == -1 ? Color(1.0f, 1.0f, 1.0f, 0.65f) : Color(1.0f, 1.0f, 1.0f, 0.5f);
			GUIPainter::Instance()->DrawBorderedRect(mainRect.right - Height, mainRect.top + 1, mainRect.right - 1, mainRect.bottom, GUIPainter::DEPTH_MID, 0, btnColor);

			glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(mainRect.left + 3, mainRect.top + 5, GUIPainter::DEPTH_MID, pItems[selected].Label);

			if (States->ActiveId == Id)
			{
				RECT dropDownRect;
				SetRect(&dropDownRect, States->CurrentPosX, States->CurrentPosY + Height, States->CurrentPosX + Width - Height, States->CurrentPosY + Height + 1 + numItems * ItemHeight);

				if (PtInRect(&dropDownRect, mousePt) && States->MouseState.Action == MouseAction::LButtonDown)
				{
					selected = (mousePt.y - dropDownRect.top) / ItemHeight;
					States->ActiveId = -1;
					States->HoveredId = Id;
					States->MouseState.Action = MouseAction::None;
				}

				GUIPainter::Instance()->DrawBorderedRect(dropDownRect.left, dropDownRect.top + 1, dropDownRect.right, dropDownRect.bottom, GUIPainter::DEPTH_NEAR, 0, 0.5f * Color::WHITE);

				int hoveredIdx = Math::Clamp((mousePt.y - dropDownRect.top) / ItemHeight, 0, numItems - 1);
				for (auto i = 0; i < numItems; i++)
				{
					if (i == hoveredIdx)
					{
						GUIPainter::Instance()->DrawBorderedRect(dropDownRect.left, dropDownRect.top + 2 + hoveredIdx * ItemHeight, dropDownRect.right - 1, dropDownRect.top + 1 + (hoveredIdx + 1) * ItemHeight, GUIPainter::DEPTH_NEAR, 0, Color(0.85f, 0.85f, 0.85f, 0.5f));

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

		bool EDXGui::InputText(string& buf)
		{
			const int Width = 100;
			const int Height = 18;
			const int Indent = 4;

			auto CalcCharWidthPrefixSum = [&]()
			{
				// Calculate string length prefix sum
				States->StrWidthPrefixSum.clear();
				States->StrWidthPrefixSum.push_back(0);
				for (auto i = 0; i < buf.length(); i++)
				{
					SIZE textExtent;
					GetTextExtentPoint32A(GUIPainter::Instance()->GetDC(), &buf[i], 1, &textExtent);
					States->StrWidthPrefixSum.push_back(i == 0 ? textExtent.cx : textExtent.cx + States->StrWidthPrefixSum[i]);
				}
			};

			bool trigger = false;
			int Id = States->CurrentId++;

			RECT rect;
			SetRect(&rect, States->CurrentPosX, States->CurrentPosY, States->CurrentPosX + Width, States->CurrentPosY + Height);

			POINT mousePt;
			mousePt.x = States->MouseState.x;
			mousePt.y = States->MouseState.y;

			if (PtInRect(&rect, mousePt))
			{
				if (States->MouseState.Action == MouseAction::LButtonDown)
				{
					// Set activated
					States->ActiveId = Id;

					CalcCharWidthPrefixSum();

					// Place cursor
					auto distX = mousePt.x - (States->CurrentPosX + 3);
					auto charIt = std::lower_bound(States->StrWidthPrefixSum.begin(), States->StrWidthPrefixSum.end(), distX);

					States->CursorIdx = ((charIt == States->StrWidthPrefixSum.begin()) ? 0 : charIt - States->StrWidthPrefixSum.begin() - 1);
					States->CursorPos = Indent + ((charIt == States->StrWidthPrefixSum.begin()) ? 0 : *(charIt - 1));

					States->Selecting = true;
					States->SelectIdx = States->CursorIdx;
				}
				if (States->MouseState.Action == MouseAction::LButtonDbClick)
				{
					States->ActiveId = Id;

					// Place cursor
					States->CursorPos = Indent + (buf.length() > 0 ? *States->StrWidthPrefixSum.rbegin() : 0);
					States->CursorIdx = buf.length();

					States->Selecting = true;
					States->SelectIdx = 0;
				}

				States->HoveredId = Id;
			}

			if (States->MouseState.Action == MouseAction::Move && States->MouseState.lDown && States->Selecting)
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
				case char(Key::RightArrow) :
				{
					auto orgIdx = States->CursorIdx++;
					States->CursorIdx = Math::Min(States->CursorIdx, buf.length());
					States->CursorPos += States->StrWidthPrefixSum[States->CursorIdx] - States->StrWidthPrefixSum[orgIdx];
					break;
				}
				case char(Key::BackSpace) :
					if (States->CursorIdx > 0)
					{
						int shift = States->StrWidthPrefixSum[States->CursorIdx] - States->StrWidthPrefixSum[States->CursorIdx - 1];
						buf.erase(States->CursorIdx - 1, 1);

						CalcCharWidthPrefixSum();

						States->CursorPos -= shift;
						States->CursorIdx--;
					}
					break;
				case char(Key::Delete) :
					if (States->CursorIdx < buf.length())
					{
						int indent = States->StrWidthPrefixSum[States->CursorIdx + 1] - States->StrWidthPrefixSum[States->CursorIdx];
						buf.erase(States->CursorIdx, 1);

						CalcCharWidthPrefixSum();
					}
					break;
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

					if (States->Selecting) // When in selection mode, erase all charactors selected
					{
						auto minIdx = Math::Min(States->CursorIdx, States->SelectIdx);
						auto maxIdx = Math::Max(States->CursorIdx, States->SelectIdx);
						buf.erase(minIdx, maxIdx - minIdx);
						States->CursorIdx = minIdx;
						States->CursorPos = Indent + States->StrWidthPrefixSum[minIdx];
						CalcCharWidthPrefixSum();
					}

					SIZE textExtent;
					GetTextExtentPoint32A(GUIPainter::Instance()->GetDC(), &States->KeyState.key, 1, &textExtent);

					if (*States->StrWidthPrefixSum.rbegin() + textExtent.cx >= Width - Indent)
						break; // Limit string length to input frame width

					// Insert charactors
					buf.insert(States->CursorIdx, 1, States->KeyState.key);

					CalcCharWidthPrefixSum();

					States->CursorPos += textExtent.cx;
					States->CursorIdx++;
				}

				// Terminate selection when any key is pressed
				States->Selecting = false;
			}

			Color color = States->HoveredId == Id || States->ActiveId == Id ? Color(1.0f, 1.0f, 1.0f, 0.65f) : Color(1.0f, 1.0f, 1.0f, 0.5f);
			GUIPainter::Instance()->DrawBorderedRect(rect.left,
				rect.top,
				rect.right,
				rect.bottom,
				GUIPainter::DEPTH_MID,
				1, Color(0.0f), color);

			// Draw string
			if (!States->Selecting || States->ActiveId != Id || States->CursorIdx == States->SelectIdx)
			{
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				GUIPainter::Instance()->DrawString(States->CurrentPosX + 3, States->CurrentPosY + 5, GUIPainter::DEPTH_MID, buf.c_str());
			}
			else
			{
				auto minIdx = Math::Min(States->CursorIdx, States->SelectIdx);
				auto maxIdx = Math::Max(States->CursorIdx, States->SelectIdx);

				GUIPainter::Instance()->DrawBorderedRect(States->CurrentPosX + Indent + States->StrWidthPrefixSum[minIdx],
					States->CurrentPosY + 3,
					States->CurrentPosX + Indent + States->StrWidthPrefixSum[maxIdx],
					States->CurrentPosY + 16,
					GUIPainter::DEPTH_MID,
					0,
					color);

				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				GUIPainter::Instance()->DrawString(States->CurrentPosX + 3, States->CurrentPosY + 5, GUIPainter::DEPTH_MID, buf.c_str(), minIdx);
				GUIPainter::Instance()->DrawString(States->CurrentPosX + 3 + States->StrWidthPrefixSum[maxIdx], States->CurrentPosY + 5, GUIPainter::DEPTH_MID, buf.c_str() + maxIdx);

				glColor4f(0.15f, 0.15f, 0.15f, 0.15f);
				glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
				GUIPainter::Instance()->DrawString(States->CurrentPosX + 3 + States->StrWidthPrefixSum[minIdx], States->CurrentPosY + 5, GUIPainter::DEPTH_MID, buf.c_str() + minIdx, maxIdx - minIdx);
			}

			if (States->ActiveId == Id) // Draw cursor
				GUIPainter::Instance()->DrawLineStrip(States->CurrentPosX + States->CursorPos, States->CurrentPosY + 3, States->CurrentPosX + States->CursorPos, States->CurrentPosY + 16);

			if (States->CurrentGrowthStrategy == GrowthStrategy::Vertical)
				States->CurrentPosY += Height + Padding;
			else
				States->CurrentPosX += 5;

			return false;
		}
	}
}