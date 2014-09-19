#pragma once

#include "../EDXPrerequisites.h"
#include "../Windows/Event.h"
#include "../Memory/RefPtr.h"

namespace EDX
{
	namespace GUI
	{
		class GUIPainter
		{
		private:
			static GUIPainter* mpInstance;
			int miTextListBase;

		private:
			GUIPainter();

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

			void DrawRect(int iX0, int iY0, int iX1, int iY1);
			void DrawRect(int iX0, int iY0, int iX1, int iY1, int iBorderSize);
			void DrawString(int x, int y, const char* strText);
		};

		typedef Event<Object*, NotifyEvent> ControlEvent;
		class EDXControl;

		class EDXDialog : public Object
		{
		private:
			vector<RefPtr<EDXControl>> mvControls;
			EDXControl* mpFocusControl;

			int miPosX, miPosY;
			int miParentWidth, miParentHeight;

			NotifyEvent mCallbackEvent;

		public:
			EDXDialog() {}
			~EDXDialog()
			{
				Release();
			}

			void Init(int iX, int iY, int iParentWidth, int iParentHeight);
			void SetCallback(const NotifyEvent& event) { mCallbackEvent = event; }
			bool MsgProc(const MouseEventArgs& mouseArgs);
			void SendEvent(EDXControl* pControl);
			void Render() const;
			void Resize(int iWidth, int iHeight);
			void Release();

			void SetFocusControl(EDXControl* pControl) { mpFocusControl = pControl; }
			void ResetFocusControl() { mpFocusControl = NULL; }

			EDXControl* GetControlAtPoint(const POINT& pt) const;
			EDXControl* GetControlWithID(uint ID) const;

			bool AddButton(uint ID, int iX, int iY, int iWidth, int iHeight, char* pStr);
			bool AddSlider(uint ID, int iX, int iY, int iWidth, int iHeight, float fMin, float fMax, float fVal);
			bool AddCheckBox(uint ID, int iX, int iY, int iWidth, int iHeight, bool bChecked, char* pStr);
			bool AddText(uint ID, int iX, int iY, int iWidth, int iHeight, char* pStr);
		};

		class EDXControl : public Object
		{
		protected:
			int miX, miY;
			int miWidth, miHeight;
			RECT mrcBBox;
			bool mbHasFocus;

			uint mID;
			EDXDialog* mpDialog;

		public:
			EDXControl(uint iID, int iX, int iY, int iW, int iH, EDXDialog* pDiag)
				: mID(iID)
				, miX(iX)
				, miY(iY)
				, miWidth(iW)
				, miHeight(iH)
				, mbHasFocus(false)
				, mpDialog(pDiag)
			{}
			virtual ~EDXControl() {}

			uint GetID() const { return mID; }
			virtual void Render() const = 0;
			virtual float GetValue() const { return 0.0f; }
			void SetFocus() { mbHasFocus = true; mpDialog->SetFocusControl(this); }
			void ResetFocus() { mbHasFocus = false; mpDialog->ResetFocusControl(); }
			virtual void UpdateRect() { SetRect(&mrcBBox, miX, miY, miX + miWidth, miY + miHeight); }
			virtual bool ContainsPoint(const POINT& pt) const { return PtInRect(&mrcBBox, pt); }

			virtual bool HandleMouse(const MouseEventArgs& mouseArgs) { return false; }
		};

		class Button : public EDXControl
		{
		private:
			char mstrText[256];
			bool mbDown;
			bool mbPressed;
			bool mbHovered;

		public:
			Button(uint iID, int iX, int iY, int iWidth, int iHeight, char* pStr, EDXDialog* pDiag);
			void Render() const;
			void Trigger() { mpDialog->SendEvent(this); }
			bool HandleMouse(const MouseEventArgs& mouseArgs);
		};

		class Slider : public EDXControl
		{
		private:
			float mfMin;
			float mfMax;
			float mfVal;

			bool mbPressed;
			int miDragX;
			int miDragOffset;
			int miButtonX;


			int miButtonSize;
			RECT mrcButtonBBox;

		public:
			Slider(uint iID, int iX, int iY, int iWidth, int iHeight, float fMin, float fMax, float fVal, EDXDialog* pDiag);
			~Slider() {}

			void Render() const;
			float GetValue() const { return mfVal; }
			void UpdateRect();

			void SetValue(float fValue);
			void SetValueFromPos(int iPos);
			bool HandleMouse(const MouseEventArgs& mouseArgs);
		};

		class CheckBox : public EDXControl
		{
		private:
			char mstrText[256];
			RECT mrcBoxBBox;
			bool mbChecked;
			bool mbPressed;

			int miBoxSize;

		public:
			CheckBox(uint iID, int iX, int iY, int iWidth, int iHeight, bool bChecked, char* pStr, EDXDialog* pDiag);
			void Render() const;
			bool GetChecked() const { return mbChecked; }
			void UpdateRect();
			void Toggle() { mbChecked = !mbChecked; mpDialog->SendEvent(this); }
			bool HandleMouse(const MouseEventArgs& mouseArgs);
		};

		class Text : public EDXControl
		{
		private:
			char mstrText[256];

		public:
			Text(uint iID, int iX, int iY, int iWidth, int iHeight, char* pStr, EDXDialog* pDiag);
			void Render() const;

			void SetText(char* pStr);
		};
	}
}
