#pragma once

#include "../Core/Types.h"

namespace EDX
{
	enum EDXImageFormat
	{
		EDX_RGB_24 = 3,
		EDX_RGBA_32 = 4
	};

	class Bitmap
	{
	public:
		static void SaveBitmapFile(const char* strFilename, const float* pData, int iWidth, int iHeight, EDXImageFormat format = EDX_RGBA_32);
		static void SaveBitmapFile(const char* strFilename, const _byte* pData, int iWidth, int iHeight);

		template<typename T>
		static T* ReadFromFile(const char* strFile, int* pWidth, int* pHeight, int* pChannel);
		template<typename T>
		static T* ReadFromFile(const char* strFile, int* pWidth, int* pHeight, int* pChannel, int requiredChannel);
	};
}
