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

			mTextListBase = glGenLists(96);
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

		void GUIPainter::DrawBorderedRect(int iX0, int iY0, int iX1, int iY1, int iBorderSize)
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			DrawRect(iX0, iY0, iX1, iY1);

			iX0 += iBorderSize;
			iX1 -= iBorderSize;
			iY0 += iBorderSize;
			iY1 -= iBorderSize;

			if (iX0 > iX1 || iY0 > iY1 || iBorderSize == -1)
				return;

			glColor4f(0.5f, 0.6f, 1.0f, 1.0f);
			DrawRect(iX0, iY0, iX1, iY1);
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

		void GUIPainter::DrawString(int x, int y, const char* strText)
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
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
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
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
			EDXControl* pButton = new Button(ID, mPaddingX, mPaddingY + mvControls.size() * mVerticalDistance, mControlWidth, mControlHeight, pStr, this);
			if(!pButton)
			{
				return false;
			}

			pButton->UpdateRect();
			mvControls.push_back(pButton);

			return true;
		}

		bool EDXDialog::AddSlider(uint ID, float min, float max, float val)
		{
			EDXControl* pSlider = new Slider(ID, mPaddingX, mPaddingY + mvControls.size() * mVerticalDistance, mControlWidth, mControlHeight, min, max, val, this);
			if(!pSlider)
			{
				return false;
			}

			pSlider->UpdateRect();
			mvControls.push_back(pSlider);

			return true;
		}

		bool EDXDialog::AddCheckBox(uint ID, bool bChecked, char* pStr)
		{
			EDXControl* pCheckedBox = new CheckBox(ID, mPaddingX, mPaddingY + mvControls.size() * mVerticalDistance, mControlWidth, mControlHeight, bChecked, pStr, this);
			if(!pCheckedBox)
			{
				return false;
			}

			pCheckedBox->UpdateRect();
			mvControls.push_back(pCheckedBox);

			return true;
		}

		bool EDXDialog::AddText(uint ID, char* pStr)
		{
			EDXControl* pText = new Text(ID, mPaddingX, mPaddingY + mvControls.size() * mVerticalDistance, mControlWidth, mControlHeight, pStr, this);
			if(!pText)
			{
				return false;
			}

			pText->UpdateRect();
			mvControls.push_back(pText);

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
				{
					return pControl;
				}
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
			, mbPressed(false)
			, mbHovered(false)
		{
			strcpy_s(mstrText, 256, pStr);
		}

		void Button::Render() const
		{
			if(mbDown)
			{
				GUIPainter::Instance()->DrawBorderedRect(mrcBBox.left + 1, mrcBBox.top + 1, mrcBBox.right - 1, mrcBBox.bottom - 1, 2);
			}
			else if(mbHovered)
			{
				GUIPainter::Instance()->DrawBorderedRect(mrcBBox.left - 1, mrcBBox.top - 1, mrcBBox.right + 1, mrcBBox.bottom + 1, 2);
			}
			else
			{
				GUIPainter::Instance()->DrawBorderedRect(mrcBBox.left, mrcBBox.top, mrcBBox.right, mrcBBox.bottom, 2);
			}

			int midX = mX + mWidth / 2 - strlen(mstrText) * 7 / 2;
			int midY = mY + mHeight / 2;
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
					mbPressed = true;
					return true;
				}
				break;

			case MouseAction::LButtonUp:
				if (PtInRect(&mrcBBox, mousePt) && mbDown)
				{
					mbDown = false;
					Trigger();
				}
				mbPressed = false;

				return true;

				break;

			case MouseAction::Move:
				if (PtInRect(&mrcBBox, mousePt))
				{
					if(!mbPressed)
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
					if(!mbPressed)
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
		Slider::Slider(uint iID, int iX, int iY, int iWidth, int iHeight, float min, float max, float val, EDXDialog* pDiag)
			: EDXControl(iID, iX, iY, iWidth, iHeight, pDiag)
			, mMin(min)
			, mMax(max)
			, mVal(Math::Clamp(val, min, max))
			, mbPressed(false)
			, mButtonSize(6)
		{

		}

		void Slider::Render() const
		{
			int iY = mY + mHeight / 2;

			float fLerp = Math::LinStep(mVal, mMin, mMax);
			int iButPos = (int)Math::Lerp(mX, mX + mWidth, fLerp);

			GUIPainter::Instance()->DrawBorderedRect(mX, iY - 1, mX + mWidth, iY + 1, 1);
			GUIPainter::Instance()->DrawBorderedRect(iButPos - mButtonSize, iY - mButtonSize, iButPos + mButtonSize, iY + mButtonSize, 1);
		}

		void Slider::UpdateRect()
		{
			EDXControl::UpdateRect();

			float fLerp = Math::LinStep(mVal, mMin, mMax);
			mButtonX = (int)Math::Lerp(mX, mX + mWidth, fLerp);

			int mid = mY + mHeight / 2;
			SetRect(&mrcButtonBBox, mButtonX - mButtonSize, mid - mButtonSize, mButtonX + mButtonSize, mid + mButtonSize);
		}

		void Slider::SetValue(float fValue)
		{
			float fClampedVal = Math::Clamp(fValue, mMin, mMax);

			if(fClampedVal == mVal)
			{
				return;
			}

			mVal = fClampedVal;
			UpdateRect();

			mpDialog->SendEvent(this);
		}

		void Slider::SetValueFromPos(int iPos)
		{
			float fLerp = Math::LinStep(float(iPos), mX, mX + mWidth);
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
					mbPressed = true;
					
					mDragX = mousePt.x;
					mDragOffset = mButtonX - mDragX;

					return true;
				}
				else if (PtInRect(&mrcBBox, mousePt))
				{
					SetValueFromPos(mousePt.x);
					return true;
				}
				break;

			case MouseAction::LButtonUp:
				mbPressed = false;
				mDragOffset = 0;
				mpDialog->SendEvent(this);

				return true;
				break;

			case MouseAction::Move:
				if(mbPressed)
				{
					SetValueFromPos(mousePt.x + mDragOffset);
					return true;
				}
				break;	
			}

			return false;
		}

		//----------------------------------------------------------------------------------
		// CheckedBox implementation
		//----------------------------------------------------------------------------------
		CheckBox::CheckBox(uint iID, int iX, int iY, int iWidth, int iHeight, bool bChecked, char* pStr, EDXDialog* pDiag)
			: EDXControl(iID, iX, iY, iWidth, iHeight, pDiag)
			, mbChecked(bChecked)
			, mbPressed(false)
			, mBoxSize(6)
		{
			strcpy_s(mstrText, 256, pStr);
		}

		void CheckBox::Render() const
		{
			int imdX = mX + 6;
			int imdY = mY + mHeight / 2;
			if(mbChecked)
			{
				GUIPainter::Instance()->DrawBorderedRect(imdX - mBoxSize, imdY - mBoxSize, imdX + mBoxSize, imdY + mBoxSize, 3);
			}
			else
			{
				GUIPainter::Instance()->DrawBorderedRect(imdX - mBoxSize, imdY - mBoxSize, imdX + mBoxSize, imdY + mBoxSize, -1);
			}

			GUIPainter::Instance()->DrawString(imdX + mBoxSize + 2, imdY + 1, mstrText);
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
					mbPressed = true;
					return true;
				}
				break;

			case MouseAction::LButtonUp:
				if (PtInRect(&mrcBoxBBox, mousePt) && mbPressed)
				{
					Toggle();
					mbPressed = false;
					return true;
				}
				break;
			}

			return false;
		}

		//----------------------------------------------------------------------------------
		// Text implementation
		//----------------------------------------------------------------------------------
		Text::Text(uint iID, int iX, int iY, int iWidth, int iHeight, char* pStr, EDXDialog* pDiag)
			: EDXControl(iID, iX, iY, iWidth, iHeight, pDiag)
		{
			strcpy_s(mstrText, 256, pStr);
		}

		void Text::Render() const
		{
			int imdY = mY + mHeight / 2;
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