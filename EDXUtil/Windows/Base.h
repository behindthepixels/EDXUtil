#pragma once

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

	inline int DetectCPUCount()
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return static_cast<int>(info.dwNumberOfProcessors);
	}
}
