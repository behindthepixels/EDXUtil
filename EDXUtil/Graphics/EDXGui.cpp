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

			miTextListBase = glGenLists(96);
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
			wglUseFontBitmaps(hDC, 0, 128, miTextListBase);

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
			glListBase(miTextListBase);

			glRasterPos2i(x + 1, y + 4);
			glCallLists((GLsizei)strlen(strText), GL_UNSIGNED_BYTE, strText);
		}

		//----------------------------------------------------------------------------------
		// Dialog implementation
		//----------------------------------------------------------------------------------
		void EDXDialog::Init(int iX, int iY, int iParentWidth, int iParentHeight)
		{
			miPosX = iX;
			miPosY = iY;
			miParentWidth = iParentWidth;
			miParentHeight = iParentHeight;
		}

		void EDXDialog::Render() const
		{
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(miPosX, miParentHeight - miPosY, 0.0f);
			glScalef(1.0f, -1.0f, 1.0f);

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, miParentWidth, 0, miParentHeight, -1, 1);

			glPushAttrib(GL_ENABLE_BIT);
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glColor4f(0.1f, 0.1f, 0.1f, 0.3f);
			GUIPainter::Instance()->DrawRect(0, 0, miParentWidth - miPosX, miParentHeight - miPosY);

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
			miPosX += iWidth - miParentWidth;
			miParentWidth = iWidth;
			miParentHeight = iHeight;
		}

		void EDXDialog::Release()
		{
			mvControls.clear();
			GUIPainter::DeleteInstance();
		}

		bool EDXDialog::AddButton(uint ID, int iX, int iY, int iWidth, int iHeight, char* pStr)
		{
			EDXControl* pButton = new Button(ID, iX, iY, iWidth, iHeight, pStr, this);
			if(!pButton)
			{
				return false;
			}

			pButton->UpdateRect();

			mvControls.push_back(pButton);

			return true;
		}

		bool EDXDialog::AddSlider(uint ID, int iX, int iY, int iWidth, int iHeight, float fMin, float fMax, float fVal)
		{
			EDXControl* pSlider = new Slider(ID, iX, iY, iWidth, iHeight, fMin, fMax, fVal, this);
			if(!pSlider)
			{
				return false;
			}

			pSlider->UpdateRect();

			mvControls.push_back(pSlider);

			return true;
		}

		bool EDXDialog::AddCheckBox(uint ID, int iX, int iY, int iWidth, int iHeight, bool bChecked, char* pStr)
		{
			EDXControl* pCheckedBox = new CheckBox(ID, iX, iY, iWidth, iHeight, bChecked, pStr, this);
			if(!pCheckedBox)
			{
				return false;
			}

			pCheckedBox->UpdateRect();

			mvControls.push_back(pCheckedBox);

			return true;
		}

		bool EDXDialog::AddText(uint ID, int iX, int iY, int iWidth, int iHeight, char* pStr)
		{
			EDXControl* pText = new Text(ID, iX, iY, iWidth, iHeight, pStr, this);
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
			offsettedArgs.x -= miPosX;
			offsettedArgs.y -= miPosY;

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

			int iMidX = miX + miWidth / 2 - strlen(mstrText) * 7 / 2;
			int iMidY = miY + miHeight / 2;
			GUIPainter::Instance()->DrawString(iMidX, iMidY + 1, mstrText);
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
		Slider::Slider(uint iID, int iX, int iY, int iWidth, int iHeight, float fMin, float fMax, float fVal, EDXDialog* pDiag)
			: EDXControl(iID, iX, iY, iWidth, iHeight, pDiag)
			, mfMin(fMin)
			, mfMax(fMax)
			, mfVal(Math::Clamp(fVal, fMin, fMax))
			, mbPressed(false)
			, miButtonSize(6)
		{

		}

		void Slider::Render() const
		{
			int iY = miY + miHeight / 2;

			float fLerp = Math::LinStep(mfVal, mfMin, mfMax);
			int iButPos = (int)Math::Lerp(miX, miX + miWidth, fLerp);

			GUIPainter::Instance()->DrawBorderedRect(miX, iY - 1, miX + miWidth, iY + 1, 1);
			GUIPainter::Instance()->DrawBorderedRect(iButPos - miButtonSize, iY - miButtonSize, iButPos + miButtonSize, iY + miButtonSize, 1);
		}

		void Slider::UpdateRect()
		{
			EDXControl::UpdateRect();

			float fLerp = Math::LinStep(mfVal, mfMin, mfMax);
			miButtonX = (int)Math::Lerp(miX, miX + miWidth, fLerp);

			int iMidY = miY + miHeight / 2;
			SetRect(&mrcButtonBBox, miButtonX - miButtonSize, iMidY - miButtonSize, miButtonX + miButtonSize, iMidY + miButtonSize);
		}

		void Slider::SetValue(float fValue)
		{
			float fClampedVal = Math::Clamp(fValue, mfMin, mfMax);

			if(fClampedVal == mfVal)
			{
				return;
			}

			mfVal = fClampedVal;
			UpdateRect();

			mpDialog->SendEvent(this);
		}

		void Slider::SetValueFromPos(int iPos)
		{
			float fLerp = Math::LinStep(float(iPos), miX, miX + miWidth);
			float fVal = Math::Lerp(mfMin, mfMax, fLerp);

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
					
					miDragX = mousePt.x;
					miDragOffset = miButtonX - miDragX;

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
				miDragOffset = 0;
				mpDialog->SendEvent(this);

				return true;
				break;

			case MouseAction::Move:
				if(mbPressed)
				{
					SetValueFromPos(mousePt.x + miDragOffset);
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
			, miBoxSize(6)
		{
			strcpy_s(mstrText, 256, pStr);
		}

		void CheckBox::Render() const
		{
			int iMidX = miX + 6;
			int iMidY = miY + miHeight / 2;
			if(mbChecked)
			{
				GUIPainter::Instance()->DrawBorderedRect(iMidX - miBoxSize, iMidY - miBoxSize, iMidX + miBoxSize, iMidY + miBoxSize, 3);
			}
			else
			{
				GUIPainter::Instance()->DrawBorderedRect(iMidX - miBoxSize, iMidY - miBoxSize, iMidX + miBoxSize, iMidY + miBoxSize, -1);
			}

			GUIPainter::Instance()->DrawString(iMidX + miBoxSize + 2, iMidY + 1, mstrText);
		}

		void CheckBox::UpdateRect()
		{
			EDXControl::UpdateRect();

			int iMidX = miX + 6;
			int iMidY = miY + miHeight / 2;
			SetRect(&mrcBoxBBox, iMidX - miBoxSize, iMidY - miBoxSize, iMidX + miBoxSize, iMidY + miBoxSize);
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
			int iMidY = miY + miHeight / 2;
			GUIPainter::Instance()->DrawString(miX, iMidY, mstrText);
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