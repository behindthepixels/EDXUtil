#pragma once

#include "../Core/Types.h"

#define NOMINMAX
#include <Windows.h>

namespace EDX
{
	class Object
	{
	public:
		virtual ~Object()
		{
		}
	};

	class WindowsProcess
	{
	public:
		static void Sleep(float Seconds)
		{
			::Sleep(static_cast<uint32>(Seconds * 1000.0f));
		}
	};
}
