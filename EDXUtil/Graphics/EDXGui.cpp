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

		const char* VertexShaderSource =
		   "varying vec2 texCoord;\
			void main()\
			{\
				gl_Position = gl_Vertex;\
				texCoord = gl_MultiTexCoord0.xy;\
			}";
		const char* FragmentShaderSource =
		   "uniform sampler2D texSampler;\
			varying vec2 texCoord;\
			void main()\
			{\
				vec4 tex = texture2D(texSampler, texCoord);\
				gl_FragColor = vec4(tex.rgb, 1.0);\
			}";

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
			mVertexShader.Load(ShaderType::VertexShader, VertexShaderSource);
			mBlurFragmentShader.Load(ShaderType::FragmentShader, FragmentShaderSource);
			mProgram.AttachShader(&mVertexShader);
			mProgram.AttachShader(&mBlurFragmentShader);
			mProgram.Link();
		}

		void GUIPainter::Resize(int width, int height)
		{
			mFBWidth = width;
			mFBHeight = height;
			// Init background texture
			mBackgroundTexStorage.Init(width, height);

			mColorRBO.SetStorage(width, height, ImageFormat::RGBA);
			mFBO.Attach(FrameBufferAttachment::Color0, &mColorRBO);
		}

		void GUIPainter::BlurBackgroundTexture()
		{
			glReadPixels(0, 0, mFBWidth, mFBHeight, (int)ImageFormat::RGBA, (int)ImageDataType::Byte, (void*)mBackgroundTexStorage.ModifiableData());
			mBackgroundTex.Load(ImageFormat::RGBA, ImageFormat::RGBA, ImageDataType::Byte, (void*)mBackgroundTexStorage.Data(), mFBWidth, mFBHeight);

			mFBO.SetTarget(FrameBufferTarget::Draw);
			mFBO.Bind();

			glClear(GL_COLOR_BUFFER_BIT);

			mProgram.Use();
			mProgram.SetUniform("texSampler", 0);

			mBackgroundTex.Bind();
			mBackgroundTex.SetFilter(TextureFilter::Nearest);
			glBegin(GL_QUADS);

			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-1.0f, -1.0f, DEPTH_FAR);

			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(1.0f, -1.0f, DEPTH_FAR);

			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(1.0f, 1.0f, DEPTH_FAR);

			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-1.0f, 1.0f, DEPTH_FAR);

			glEnd();

			mProgram.Unuse();
			mBackgroundTex.UnBind();
			mFBO.UnBind();
		}

		void GUIPainter::DrawBackgroundTexture(int x0, int y0, int x1, int y1)
		{
			mFBO.SetTarget(FrameBufferTarget::Read);
			mFBO.Bind();

			GL::glBlitFramebuffer(x0, y0, x1, y1, x0, y0, x1, y1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		void GUIPainter::DrawBorderedRect(int iX0, int iY0, int iX1, int iY1, float depth, int iBorderSize, const Color& interiorColor, const Color& borderColor)
		{
			if (iBorderSize > 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				GL::glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				glColor4f(borderColor.r, borderColor.g, borderColor.b, 0.5f);
				DrawRect(iX0, iY0, iX1, iY1, depth);

				iX0 += iBorderSize;
				iX1 -= iBorderSize;
				iY0 += iBorderSize;
				iY1 -= iBorderSize;

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				GL::glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				glColor4f(interiorColor.r, interiorColor.g, interiorColor.b, 0.5f);
				DrawRect(iX0, iY0, iX1, iY1, depth);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				GL::glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
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

			glRasterPos3f(x + 1, y + 4, depth);
			glCallLists((GLsizei)strlen(strText), GL_UNSIGNED_BYTE, strText);
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

			glPushAttrib(GL_ALL_ATTRIB_BITS);

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, mParentWidth, 0, mParentHeight, 1, -1);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			// Render the blurred background texture
			glEnable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			GUIPainter::Instance()->BlurBackgroundTexture();
			GUIPainter::Instance()->DrawBackgroundTexture(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight);

			glTranslatef(mPosX, mParentHeight - mPosY, 0.0f);
			glScalef(1.0f, -1.0f, 1.0f);

			glLineWidth(1.0f);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_CONSTANT_ALPHA);
			GL::glBlendColor(1.0f, 1.0f, 1.0f, 0.5f);

			glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
			GUIPainter::Instance()->DrawRect(0, 0, mWidth, mHeight, GUIPainter::DEPTH_FAR);

			for(int i = 0; i < mvControls.size(); i++)
			{
				mvControls[i]->Render();
			}

			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			glPopAttrib();
		}

		void EDXDialog::Resize(int width, int height)
		{
			mParentWidth = width;
			mParentHeight = height;

			mPosX = mParentWidth - mWidth;
			mPosY = 0;

			GUIPainter::Instance()->Resize(width, height);
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
			if(!pButton)
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
			if(!pSlider)
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
			if(!pCheckedBox)
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
			if(!pText)
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
			for(int i = 0; i < mvControls.size(); i++)
			{
				EDXControl* pControl = mvControls[i].Ptr();

				if(pControl->ContainsPoint(pt))
				{
					return pControl;
				}
			}
			return NULL;
		}

		EDXControl* EDXDialog::GetControlWithID(uint ID) const
		{
			for(int i = 0; i < mvControls.size(); i++)
			{
				EDXControl* pControl = mvControls[i].Ptr();

				if(pControl->GetID() == ID)
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
			if(mbDown)
			{
				GUIPainter::Instance()->DrawBorderedRect(mBBox.left + 1, mBBox.top + 1, mBBox.right - 1, mBBox.bottom - 1, GUIPainter::DEPTH_MID, 0, Color::WHITE);
			}
			else if(mHovered)
			{
				GUIPainter::Instance()->DrawBorderedRect(mBBox.left - 1, mBBox.top - 1, mBBox.right + 1, mBBox.bottom + 1, GUIPainter::DEPTH_MID, 0, Color::WHITE);
			}
			else
			{
				GUIPainter::Instance()->DrawBorderedRect(mBBox.left, mBBox.top, mBBox.right, mBBox.bottom, GUIPainter::DEPTH_MID, 2);
			}

			int midX = mX + mWidth / 2 - strlen(mstrText) * 7 / 2;
			int midY = mY + mHeight / 2;


			GL::glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
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
			GL::glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
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
				if(mPressed)
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
			if(mbChecked)
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
			if(pStr == NULL)
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

			GL::glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
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

						GL::glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
						glColor4f(0.15f, 0.15f, 0.15f, 1.0f);
					}
					else
					{
						GL::glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
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
	}
}