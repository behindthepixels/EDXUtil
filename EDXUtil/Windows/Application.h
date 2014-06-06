#pragma once

#include "../EDXPrerequisites.h"
#include "Base.h"

#include <Windows.h>

namespace EDX
{
	class Application : public Object
	{
	private:
		static HINSTANCE InstHandle;

	public:
		static HINSTANCE GetInstanceHandle() { return InstHandle; }
	};
}