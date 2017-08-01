#include "Stream.h"
#include "../Windows/Base.h"
#include "../Containers/String.h"

namespace EDX
{
	void Stream::ByteSwap(void* V, int32 Length)
	{
		uint8* Ptr = (uint8*)V;
		int32 Top = Length - 1;
		int32 Bottom = 0;
		while (Bottom < Top)
		{
			Swap(Ptr[Top--], Ptr[Bottom++]);
		}
	}
}