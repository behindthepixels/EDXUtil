
#include "Base.h"
#include "../Graphics/Color.h"
#include <WinGDI.h>

#include "Bitmap.h"
#include "../Memory/Memory.h"

#define STBI_HEADER_FILE_ONLY
#define _CRT_SECURE_NO_WARNINGS 1
#include "stb_image.c"

namespace EDX
{
	// write data to bmp file
	void Bitmap::SaveBitmapFile(const char* strFilename, const float* pData, int iWidth, int iHeight, EDXImageFormat format)
	{
		int iPixelBytes = int(format);
		int iSize = iWidth * iHeight * iPixelBytes; // the byte of pixel, data size

		const float fInvGamma = 1.f / 2.2f; // Gamma

		unsigned char* pPixels = new unsigned char[iSize];
		for (int i = 0; i < iHeight; i++)
		{
			for (int j = 0; j < iWidth; j++)
			{
				int iIndex = int(format) * (i * iWidth + j);
				pPixels[iIndex + 0] = Math::Clamp(Math::RoundToInt(255 * pData[iIndex + 2]), 0, 255);
				pPixels[iIndex + 1] = Math::Clamp(Math::RoundToInt(255 * pData[iIndex + 1]), 0, 255);
				pPixels[iIndex + 2] = Math::Clamp(Math::RoundToInt(255 * pData[iIndex + 0]), 0, 255);

				// Set alpha
				if (format == EDX_RGBA_32)
					pPixels[iIndex + 3] = 255;
			}
		}

		// Bmp first part, file information
		BITMAPFILEHEADER bmpHeader;
		bmpHeader.bfType = 0x4d42; //Bmp
		bmpHeader.bfSize = iSize // data size
			+ sizeof(BITMAPFILEHEADER) // first section size
			+ sizeof(BITMAPINFOHEADER); // second section size

		bmpHeader.bfReserved1 = 0; // reserved 
		bmpHeader.bfReserved2 = 0; // reserved
		bmpHeader.bfOffBits = bmpHeader.bfSize - iSize;

		// Bmp second part, data information
		BITMAPINFOHEADER bmpInfo;
		bmpInfo.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.biWidth = iWidth;
		bmpInfo.biHeight = iHeight;
		bmpInfo.biPlanes = 1;
		bmpInfo.biBitCount = 8 * iPixelBytes;
		bmpInfo.biCompression = 0;
		bmpInfo.biSizeImage = iSize;
		bmpInfo.biXPelsPerMeter = 0;
		bmpInfo.biYPelsPerMeter = 0;
		bmpInfo.biClrUsed = 0;
		bmpInfo.biClrImportant = 0;

		FILE* pFile = NULL;
		fopen_s(&pFile, strFilename, "wb");
		assert(pFile);


		fwrite(&bmpHeader, 1, sizeof(BITMAPFILEHEADER), pFile);
		fwrite(&bmpInfo, 1, sizeof(BITMAPINFOHEADER), pFile);

		fwrite(pPixels, 1, iSize, pFile);
		fclose(pFile);

		SafeDeleteArray(pPixels);
	}

	void Bitmap::SaveBitmapFile(const char* strFilename, const _byte* pData, int iWidth, int iHeight)
	{
		int iPixelBytes = 4;
		int iSize = iWidth * iHeight * iPixelBytes; // the byte of pixel, data size

		const float fInvGamma = 1.f / 2.2f; // Gamma

		unsigned char* pPixels = new unsigned char[iSize];
		for (int i = 0; i < iHeight; i++)
		{
			for (int j = 0; j < iWidth; j++)
			{
				int iIndex = iPixelBytes * (i * iWidth + j);
				pPixels[iIndex + 0] = pData[iIndex + 2];
				pPixels[iIndex + 1] = pData[iIndex + 1];
				pPixels[iIndex + 2] = pData[iIndex + 0];
				pPixels[iIndex + 3] = pData[iIndex + 3];
			}
		}

		// Bmp first part, file information
		BITMAPFILEHEADER bmpHeader;
		bmpHeader.bfType = 0x4d42; //Bmp
		bmpHeader.bfSize = iSize // data size
			+ sizeof(BITMAPFILEHEADER) // first section size
			+ sizeof(BITMAPINFOHEADER); // second section size

		bmpHeader.bfReserved1 = 0; // reserved 
		bmpHeader.bfReserved2 = 0; // reserved
		bmpHeader.bfOffBits = bmpHeader.bfSize - iSize;

		// Bmp second part, data information
		BITMAPINFOHEADER bmpInfo;
		bmpInfo.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.biWidth = iWidth;
		bmpInfo.biHeight = iHeight;
		bmpInfo.biPlanes = 1;
		bmpInfo.biBitCount = 8 * iPixelBytes;
		bmpInfo.biCompression = 0;
		bmpInfo.biSizeImage = iSize;
		bmpInfo.biXPelsPerMeter = 0;
		bmpInfo.biYPelsPerMeter = 0;
		bmpInfo.biClrUsed = 0;
		bmpInfo.biClrImportant = 0;

		FILE* pFile = NULL;
		fopen_s(&pFile, strFilename, "wb");
		assert(pFile);

		fwrite(&bmpHeader, 1, sizeof(BITMAPFILEHEADER), pFile);
		fwrite(&bmpInfo, 1, sizeof(BITMAPINFOHEADER), pFile);

		fwrite(pPixels, 1, iSize, pFile);
		fclose(pFile);

		SafeDeleteArray(pPixels);
	}

	Color* Bitmap::ReadFromFileFloat(const char* strFile, int* pWidth, int* pHeight, int* pChannel)
	{
		Color* pRet;
		pRet = (Color*)stbi_loadf(strFile, pWidth, pHeight, pChannel, 4);

		return pRet;
	}
	byte* Bitmap::ReadFromFileByte(const char* strFile, int* pWidth, int* pHeight, int* pChannel)
	{
		byte* pRet;
		pRet = (byte*)stbi_load(strFile, pWidth, pHeight, pChannel, 4);

		return pRet;
	}
}

