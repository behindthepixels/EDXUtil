#pragma once

#include "../EDXPrerequisites.h"

namespace EDX
{
	enum ImageFormat
	{
		EDX_RGB_24 = 3,
		EDX_RGBA_32 = 4
	};

	class Bitmap
	{
	public:
		static void SaveBitmapFile(const char* strFilename, const float* pData, int iWidth, int iHeight, ImageFormat format = EDX_RGBA_32);
		static void SaveBitmapFile(const char* strFilename, const _byte* pData, int iWidth, int iHeight);
		static class Color* ReadFromFileFloat(const char* strFile, int* pWidth, int* pHeight, int* pChannel);
		static _byte* ReadFromFileByte(const char* strFile, int* pWidth, int* pHeight, int* pChannel);
	};
}
