#pragma once

#include "../Windows/Debug.h"
#include <Assert.h>

namespace EDX
{

#ifndef _DEBUG

	#define Assert(expr)
	#define Assertf(expr, format, ...)
	#define AssertNoEntry()

#else

	#define Assert(expr) \
	{\
		if(!(expr))\
		{\
			EDX::Debug::AssertFailedMessage(#expr, __FILE__, __LINE__);\
			EDX::Debug::DebugBreak();\
		}\
	}
	
	#define Assertf(expr, format, ...) \
	{\
		if(!(expr))\
		{\
			EDX::Debug::AssertFailedMessage(#expr, __FILE__, __LINE__, format, ##__VA_ARGS__);\
			EDX::Debug::DebugBreak();\
		}\
	}
	
	#define AssertNoEntry() \
	{\
		EDX::Debug::AssertFailedMessage( "AssertNoEntry() called.", __FILE__, __LINE__ );\
		EDX::Debug::DebugBreak();\
	}

#endif

}