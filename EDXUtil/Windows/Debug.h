#pragma once

#include "Base.h"
#include "../Core/Types.h"

namespace EDX
{
	class Debug
	{
	public:

		static bool IsDebuggerPresent()
		{
			return ::IsDebuggerPresent() != 0;
		}

		__forceinline static void DebugBreak()
		{
			if (IsDebuggerPresent())
			{
				__debugbreak();
			}
		}

		static void AssertFailedMessage(const char* Msg, const char* File, int Line, const TCHAR* Format = EDX_TEXT(""), ...);

		template<typename CharType>
		static void WriteLine(const CharType* pTexts)
		{
		}

		template<>
		static void WriteLine(const ANSICHAR* pTexts)
		{
			if (IsDebuggerPresent())
			{
				OutputDebugStringA(pTexts);
				OutputDebugStringA("\n");
			}
		}

		template<>
		static void WriteLine(const WIDECHAR* pTexts)
		{
			if (IsDebuggerPresent())
			{
				OutputDebugStringW(pTexts);
				OutputDebugStringW(L"\n");
			}
		}

		template<typename CharType>
		static void DebugMessageBox(const CharType* pTexts, const CharType* pCaption)
		{
		}

		template<>
		static void DebugMessageBox(const ANSICHAR* pTexts, const ANSICHAR* pCaption)
		{
			::MessageBoxA(nullptr, pTexts, pCaption, MB_OK);
		}

		template<>
		static void DebugMessageBox(const WIDECHAR* pTexts, const WIDECHAR* pCaption)
		{
			::MessageBoxW(nullptr, pTexts, pCaption, MB_OK);
		}
	};
}