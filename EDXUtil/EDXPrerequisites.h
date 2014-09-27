#pragma once

#pragma warning(disable: 4244)
#pragma warning(disable: 4018)
#pragma warning(disable: 4267)
#pragma warning(disable: 4800)

// C++ support
#include <cstdio>
#include <malloc.h>
#include <cassert>
#include <cmath>
#include <cfloat>
#include <fstream>

// STL include files
#include <cstring>
using std::string;
using std::wstring;
#include <vector>
using std::vector;
#include <algorithm>
using std::min;
using std::max;
using std::swap;
using std::sort;
#include <exception>

namespace EDX
{
	typedef unsigned int	 uint;
	typedef unsigned char	 _byte;
	typedef unsigned long	 dword;
	typedef			 __int8	 int8;
	typedef unsigned __int8	 uint8;
	typedef			 __int16 int16;
	typedef unsigned __int16 uint16;
	typedef			 __int32 int32;
	typedef unsigned __int32 uint32;
	typedef			 __int64 int64;
	typedef unsigned __int64 uint64;
}