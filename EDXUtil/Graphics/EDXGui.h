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
			int mTextListBase;

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
			void DrawBorderedRect(int iX0, int iY0, int iX1, int iY1, int iBorderSize);
			void DrawString(int x, int y, const char* strText);
		};

		typedef Event<Object*, NotifyEvent> ControlEvent;
		class EDXControl;

		class EDXDialog : public Object
		{
		private:
			vector<RefPtr<EDXControl>> mvControls;
			EDXControl* mpFocusControl;

			int mPosX, mPosY;
			int mParentWidth, mParentHeight;

			int mWidth, mHeight;
			const int mPaddingX, mVerticalDistance;
			const int mControlWidth, mControlHeight;
			int mPaddingY;

			NotifyEvent mCallbackEvent;

		public:
			EDXDialog()
				: mPaddingX(30)
				, mPaddingY(30)
				, mVerticalDistance(25)
				, mControlWidth(140)
				, mControlHeight(21)
			{
			}
			~EDXDialog()
			{
				Release();
			}

			void Init(int iParentWidth, int iParentHeight);
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

			bool AddButton(uint ID, char* pStr);
			bool AddSlider(uint ID, float min, float max, float val);
			bool AddCheckBox(uint ID, bool bChecked, char* pStr);
			bool AddText(uint ID, char* pStr);
		};

		class EDXControl : public Object
		{
		protected:
			int mX, mY;
			int mWidth, mHeight;
			RECT mrcBBox;
			bool mbHasFocus;

			uint mID;
			EDXDialog* mpDialog;

		public:
			EDXControl(uint iID, int iX, int iY, int iW, int iH, EDXDialog* pDiag)
				: mID(iID)
				, mX(iX)
				, mY(iY)
				, mWidth(iW)
				, mHeight(iH)
				, mbHasFocus(false)
				, mpDialog(pDiag)
			{}
			virtual ~EDXControl() {}

			uint GetID() const { return mID; }
			virtual void Render() const = 0;
			virtual float GetValue() const { return 0.0f; }
			void SetFocus() { mbHasFocus = true; mpDialog->SetFocusControl(this); }
			void ResetFocus() { mbHasFocus = false; mpDialog->ResetFocusControl(); }
			virtual void UpdateRect() { SetRect(&mrcBBox, mX, mY, mX + mWidth, mY + mHeight); }
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
			static const int Padding = 35;
			static const int Width = 140;
			static const int Height = 22;

		public:
			Button(uint iID, int iX, int iY, int iWidth, int iHeight, char* pStr, EDXDialog* pDiag);
			void Render() const;
			void Trigger() { mpDialog->SendEvent(this); }
			bool HandleMouse(const MouseEventArgs& mouseArgs);
		};

		class Slider : public EDXControl
		{
		private:
			float mMin;
			float mMax;
			float mVal;

			bool mbPressed;
			int mDragX;
			int mDragOffset;
			int mButtonX;


			int mButtonSize;
			RECT mrcButtonBBox;

		public:
			static const int Padding = 30;
			static const int Width = 140;
			static const int Height = 22;

		public:
			Slider(uint iID, int iX, int iY, int iWidth, int iHeight, float min, float max, float val, EDXDialog* pDiag);
			~Slider() {}

			void Render() const;
			float GetValue() const { return mVal; }
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

			int mBoxSize;

		public:
			static const int Padding = 22;
			static const int Width = 140;
			static const int Height = 21;

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
			static const int Padding = 22;
			static const int Width = 140;
			static const int Height = 21;

		public:
			Text(uint iID, int iX, int iY, int iWidth, int iHeight, char* pStr, EDXDialog* pDiag);
			void Render() const;

			void SetText(char* pStr);
		};
	}
}
