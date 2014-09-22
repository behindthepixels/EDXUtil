#include "EDXGui.h"
#include "../Math/EDXMath.h"
#include "../Memory/Memory.h"

#include "../Windows/Application.h"
#include "../Windows/Window.h"

#include "OpenGL.h"

namespace EDX
{
	namespace GUI
	{
		//----------------------------------------------------------------------------------
		// GUI Painter implementation
		//----------------------------------------------------------------------------------
		GUIPainter* GUIPainter::mpInstance = NULL;

		GUIPainter::GUIPainter()
		{
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
		}

		void GUIPainter::DrawBorderedRect(int iX0, int iY0, int iX1, int iY1, int iBorderSize, const Color& interiorColor, const Color& borderColor)
		{
			if (iBorderSize > 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				GL::glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				glColor4f(borderColor.r, borderColor.g, borderColor.b, 0.5f);
				DrawRect(iX0, iY0, iX1, iY1);

				iX0 += iBorderSize;
				iX1 -= iBorderSize;
				iY0 += iBorderSize;
				iY1 -= iBorderSize;

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				GL::glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				glColor4f(interiorColor.r, interiorColor.g, interiorColor.b, 0.5f);
				DrawRect(iX0, iY0, iX1, iY1);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				GL::glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
				glColor4f(interiorColor.r, interiorColor.g, interiorColor.b, 0.5f);
				DrawRect(iX0, iY0, iX1, iY1);
			}
		}

		void GUIPainter::DrawRect(int iX0, int iY0, int iX1, int iY1)
		{
			glBegin(GL_QUADS);

			glVertex2i(iX0, iY0);
			glVertex2i(iX1, iY0);
			glVertex2i(iX1, iY1);
			glVertex2i(iX0, iY1);

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

		void GUIPainter::DrawString(int x, int y, const char* strText)
		{
			glListBase(mTextListBase);

			glRasterPos2i(x + 1, y + 4);
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
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(mPosX, mParentHeight - mPosY, 0.0f);
			glScalef(1.0f, -1.0f, 1.0f);

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, mParentWidth, 0, mParentHeight, -1, 1);

			glPushAttrib(GL_ENABLE_BIT);
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_CONSTANT_ALPHA);
			GL::glBlendColor(1.0f, 1.0f, 1.0f, 0.5f);

			glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
			GUIPainter::Instance()->DrawRect(0, 0, mWidth, mHeight);

			for(int i = 0; i < mvControls.size(); i++)
			{
				mvControls[i]->Render();
			}

			glPopAttrib();
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}

		void EDXDialog::Resize(int iWidth, int iHeight)
		{
			mParentWidth = iWidth;
			mParentHeight = iHeight;

			mPosX = mParentWidth - mWidth;
			mPosY = 0;
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

		bool EDXDialog::AddSlider(uint ID, float min, float max, float val, const char* pText)
		{
			char str[256];
			sprintf_s(str, "%s%.2f", pText, val);
			AddText(999, str);

			int posY = mPaddingY + (Slider::Padding - Slider::Height) / 2;
			Slider* pSlider = new Slider(ID, mPaddingX, posY, Slider::Width, Slider::Height, min, max, val, pText, this);
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

		bool EDXDialog::AddCheckBox(uint ID, bool bChecked, char* pStr)
		{
			int posY = mPaddingY + (CheckBox::Padding - CheckBox::Height) / 2;
			CheckBox* pCheckedBox = new CheckBox(ID, mPaddingX, posY, CheckBox::Width, CheckBox::Height, bChecked, pStr, this);
			if(!pCheckedBox)
			{
				return false;
			}

			pCheckedBox->UpdateRect();
			mvControls.push_back(pCheckedBox);
			mPaddingY += CheckBox::Padding;

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
			MouseEventArgs offsettedArgs = mouseArgs;
			offsettedArgs.x -= mPosX;
			offsettedArgs.y -= mPosY;

			if (mpFocusControl)
			{
				if (mpFocusControl->HandleMouse(offsettedArgs))
					return true;
			}

			POINT mousePt;
			mousePt.x = offsettedArgs.x;
			mousePt.y = offsettedArgs.y;
			EDXControl* pControl = GetControlAtPoint(mousePt);
			if (pControl)
			{
				pControl->SetFocus();
				mpFocusControl = pControl;
				if (pControl->HandleMouse(offsettedArgs))
				{
					return true;
				}
			}
			else if (mpFocusControl && offsettedArgs.lDown)
			{
				mpFocusControl->ResetFocus();
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
			, mbHovered(false)
		{
			strcpy_s(mstrText, 256, pStr);
		}

		void Button::Render() const
		{
			if(mbDown)
			{
				GUIPainter::Instance()->DrawBorderedRect(mrcBBox.left + 1, mrcBBox.top + 1, mrcBBox.right - 1, mrcBBox.bottom - 1, 0, Color::WHITE);
			}
			else if(mbHovered)
			{
				GUIPainter::Instance()->DrawBorderedRect(mrcBBox.left - 1, mrcBBox.top - 1, mrcBBox.right + 1, mrcBBox.bottom + 1, 0, Color::WHITE);
			}
			else
			{
				GUIPainter::Instance()->DrawBorderedRect(mrcBBox.left, mrcBBox.top, mrcBBox.right, mrcBBox.bottom, 2);
			}

			int midX = mX + mWidth / 2 - strlen(mstrText) * 7 / 2;
			int midY = mY + mHeight / 2;


			GL::glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
			if (mbDown || mbHovered)
				glColor4f(0.15f, 0.15f, 0.15f, 1.0f);
			else
				glColor4f(0.85f, 0.85f, 0.85f, 1.0f);

			GUIPainter::Instance()->DrawString(midX, midY + 1, mstrText);
		}

		bool Button::HandleMouse(const MouseEventArgs& mouseArgs)
		{
			POINT mousePt;
			mousePt.x = mouseArgs.x;
			mousePt.y = mouseArgs.y;

			switch (mouseArgs.Action)
			{
			case MouseAction::LButtonDown:
				if (PtInRect(&mrcBBox, mousePt))
				{
					mbDown = true;
					mPressed = true;
					return true;
				}
				break;

			case MouseAction::LButtonUp:
				if (PtInRect(&mrcBBox, mousePt) && mbDown)
				{
					mbDown = false;
					Trigger();
				}
				mPressed = false;

				return true;

				break;

			case MouseAction::Move:
				if (PtInRect(&mrcBBox, mousePt))
				{
					if(!mPressed)
					{
						mbHovered = true;
					}
					else
					{
						mbDown = true;
					}
				}
				else
				{
					mbDown = false;
					mbHovered = false;
					if(!mPressed)
						ResetFocus();
				}
				return true;

				break;	
			}

			return false;
		}

		//----------------------------------------------------------------------------------
		// Slider implementation
		//----------------------------------------------------------------------------------
		Slider::Slider(uint iID, int iX, int iY, int iWidth, int iHeight, float min, float max, float val, const char* pText, EDXDialog* pDiag)
			: EDXControl(iID, iX, iY, iWidth, iHeight, pDiag)
			, mMin(min)
			, mMax(max)
			, mVal(Math::Clamp(val, min, max))
			, mPressed(false)
			, mButtonSize(6)
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

			GUIPainter::Instance()->DrawBorderedRect(mX, iY - 1, iButPos - mButtonSize, iY + 2, 0, Color::WHITE);
			GL::glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
			glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
			glBegin(GL_LINE_STRIP);

			glVertex2i(iButPos + mButtonSize, iY - 1);
			glVertex2i(mX + mWidth, iY - 1);
			glVertex2i(mX + mWidth, iY + 1);
			glVertex2i(iButPos + mButtonSize, iY + 1);

			glEnd();

			GUIPainter::Instance()->DrawBorderedRect(iButPos - mButtonSize, iY - mButtonSize, iButPos + mButtonSize, iY + mButtonSize, 0, Color::WHITE);
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
				else if (PtInRect(&mrcBBox, mousePt))
				{
					SetValueFromPos(mousePt.x);
					return true;
				}

			case MouseAction::LButtonUp:
				mPressed = false;
				mDragOffset = 0;
				mpDialog->SendEvent(this);

				return true;

			case MouseAction::Move:
				if(mPressed)
				{
					SetValueFromPos(mousePt.x + mDragOffset);
					return true;
				}
			}

			return false;
		}

		//----------------------------------------------------------------------------------
		// CheckedBox implementation
		//----------------------------------------------------------------------------------
		CheckBox::CheckBox(uint iID, int iX, int iY, int iWidth, int iHeight, bool bChecked, char* pStr, EDXDialog* pDiag)
			: EDXControl(iID, iX, iY, iWidth, iHeight, pDiag)
			, mbChecked(bChecked)
			, mPressed(false)
			, mBoxSize(6)
		{
			strcpy_s(mstrText, 256, pStr);
		}

		void CheckBox::Render() const
		{
			int midX = mX + 6;
			int midY = mY + mHeight / 2;
			if(mbChecked)
			{
				GUIPainter::Instance()->DrawBorderedRect(midX - mBoxSize, midY - mBoxSize, midX + mBoxSize, midY + mBoxSize, 0, Color::WHITE);
			}
			else
			{
				GUIPainter::Instance()->DrawBorderedRect(midX - mBoxSize, midY - mBoxSize, midX + mBoxSize, midY + mBoxSize, 2);
			}

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			GUIPainter::Instance()->DrawString(midX + mBoxSize + 2, midY + 1, mstrText);
		}

		void CheckBox::UpdateRect()
		{
			EDXControl::UpdateRect();

			int imdX = mX + 6;
			int imdY = mY + mHeight / 2;
			SetRect(&mrcBoxBBox, imdX - mBoxSize, imdY - mBoxSize, imdX + mBoxSize, imdY + mBoxSize);
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
			GUIPainter::Instance()->DrawString(mX, imdY, mstrText);
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
	}
}