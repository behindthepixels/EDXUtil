#pragma once

#include "Base.h"

namespace EDX
{
	class Debug
	{
	public:
		static void Write(const char* pTexts)
		{
			if (IsDebuggerPresent() != 0)
			{
				OutputDebugStringA(pTexts);
			}
		}
		static void WriteLine(const char* pTexts)
		{
			if (IsDebuggerPresent() != 0)
			{
				OutputDebugStringA(pTexts);
				OutputDebugStringA("\n");
			}
		}
	};
}