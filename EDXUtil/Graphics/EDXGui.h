#pragma once

#include "../EDXPrerequisites.h"
#include "../Windows/Event.h"
#include "Color.h"
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

		public:
			static const float DEPTH_FAR;
			static const float DEPTH_MID;
			static const float DEPTH_NEAR;

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

			void DrawRect(int iX0, int iY0, int iX1, int iY1, float depth);
			void DrawLineStrip(int iX0, int iY0, int iX1, int iY1);
			void DrawBorderedRect(int iX0, int iY0, int iX1, int iY1, float depth, int iBorderSize,
				const Color& interiorColor = Color::BLACK, const Color& borderColor = Color::WHITE);
			void DrawString(int x, int y, float depth, const char* strText);
		};

		class EDXControl;

		class EDXDialog : public Object
		{
		private:
			vector<RefPtr<EDXControl>> mvControls;
			EDXControl* mpFocusControl;
			EDXControl* mpHoveredControl;

			int mPosX, mPosY;
			int mWidth, mHeight;
			int mParentWidth, mParentHeight;

			const int mPaddingX;
			int mPaddingY;

			bool mVisible;
			NotifyEvent mCallbackEvent;

		public:
			EDXDialog()
				: mPaddingX(30)
				, mPaddingY(30)
				, mVisible(true)
				, mpFocusControl(nullptr)
				, mpHoveredControl(nullptr)
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
			void ResetFocusControl() { mpFocusControl = nullptr; }

			EDXControl* GetControlAtPoint(const POINT& pt) const;
			EDXControl* GetControlWithID(uint ID) const;

			bool AddButton(uint ID, char* pStr);
			bool AddSlider(uint ID, float min, float max, float val, float* pRefVal, const char* pText);
			bool AddCheckBox(uint ID, bool bChecked, bool* pRefVal, char* pStr);
			bool AddText(uint ID, const char* pStr);
			bool AddComboBox(uint iID, int initSelectedIdx, int* pRefVal, struct ComboBoxItem* pItems, int numItems);
			void AddPadding(int padding)
			{
				mPaddingY += padding;
			}

			void ToggleVisible() { mVisible = !mVisible; }
		};

		class EDXControl : public Object
		{
		protected:
			int mX, mY;
			int mWidth, mHeight;
			RECT mBBox;
			bool mHasFocus;
			bool mHovered;

			uint mID;
			EDXDialog* mpDialog;

		public:
			EDXControl(uint iID, int iX, int iY, int iW, int iH, EDXDialog* pDiag)
				: mID(iID)
				, mX(iX)
				, mY(iY)
				, mWidth(iW)
				, mHeight(iH)
				, mHasFocus(false)
				, mpDialog(pDiag)
			{}
			virtual ~EDXControl() {}

			uint GetID() const { return mID; }
			virtual void Render() const = 0;
			void SetFocus() { mHasFocus = true; mpDialog->SetFocusControl(this); }
			void ResetFocus() { OnResetFocus(); mHasFocus = false; mpDialog->ResetFocusControl(); }
			bool HasFocus() const { return mHasFocus; }
			virtual void UpdateRect() { SetRect(&mBBox, mX, mY, mX + mWidth, mY + mHeight); }
			virtual bool ContainsPoint(const POINT& pt) const { return PtInRect(&mBBox, pt); }

			virtual void OnResetFocus() {}
			virtual void OnMouseIn() { mHovered = true; }
			virtual void OnMouseOut() { mHovered = false; }

			virtual bool HandleMouse(const MouseEventArgs& mouseArgs) { return false; }
		};

		class Button : public EDXControl
		{
		private:
			char mstrText[256];
			bool mbDown;
			bool mPressed;

		public:
			static const int Padding = 40;
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

			int mSlideBase, mSlideEnd;

			bool mPressed;
			int mDragX;
			int mDragOffset;
			int mButtonX;

			int mButtonSize;
			RECT mrcButtonBBox;

			class Text* mpText;
			char mMainText[256];
			char mValuedText[256];

			float* mpRefVal;

		public:
			static const int Padding = 20;
			static const int Width = 140;
			static const int Height = 10;

		public:
			Slider(uint iID, int iX, int iY, int iWidth, int iHeight, float min, float max, float val, float* pRefVal, const char* pText, EDXDialog* pDiag);
			~Slider() {}

			void Render() const;
			float GetValue() const { return mVal; }
			void UpdateRect();
			void SetTextControl(Text* pText) { mpText = pText; }

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
			bool mPressed;

			int mBoxSize;

			bool* mpRefVal;

		public:
			static const int Padding = 25;
			static const int Width = 140;
			static const int Height = 21;

		public:
			CheckBox(uint iID, int iX, int iY, int iWidth, int iHeight, bool bChecked, bool* pRefVal, char* pStr, EDXDialog* pDiag);
			void Render() const;
			bool GetChecked() const { return mbChecked; }
			void UpdateRect();
			void Toggle() { mbChecked = !mbChecked; }
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
			Text(uint iID, int iX, int iY, int iWidth, int iHeight, const char* pStr, EDXDialog* pDiag);
			void Render() const;

			void SetText(char* pStr);
		};

		struct ComboBoxItem
		{
			int Value;
			char* Label;
		};

		class ComboBox : public EDXControl
		{
		public:
		private:
			ComboBoxItem* mpItems;
			int mNumItems;
			bool mOpened;
			int mHoveredIdx;
			int mSelectedIdx;

			int mButtonSize;
			RECT mBoxMain;
			RECT mBoxDropdown;

			int* mpRefVal;

		public:
			static const int Padding = 28;
			static const int Width = 140;
			static const int Height = 18;
			static const int ItemHeight = 20;

		public:
			ComboBox(uint iID, int iX, int iY, int iWidth, int iHeight, int initSelectedIdx, int* pRefVal, ComboBoxItem* pItems, int numItems, EDXDialog* pDiag);
			~ComboBox()
			{
				if (mpItems)
					delete[] mpItems;
			}

			void Render() const;
			void UpdateRect();

			void OnResetFocus()
			{
				mOpened = false;
				mBBox = mBoxMain;
			}

			bool HandleMouse(const MouseEventArgs& mouseArgs);
		};
	}
}
