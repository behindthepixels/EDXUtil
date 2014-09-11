#include "Texture.h"
#include "Color.h"
#include "../Windows/Bitmap.h"

namespace EDX
{
	template<class T>
	ImageTexture<T>::ImageTexture(const char* strFile)
		: mpTexels(NULL)
		, mTexWidth(0)
		, mTexHeight(0)
	{
		int iChannel;
		_byte* pRawTex = Bitmap::ReadFromFileByte(strFile, &mTexWidth, &mTexHeight, &iChannel);
		if (!pRawTex)
		{
			throw "Texture file load failed.";
		}

		mpTexels = new T[mTexWidth * mTexHeight];

		for (int i = 0; i < mTexWidth * mTexHeight; i++)
		{
			mpTexels[i].r = pRawTex[4 * i + 0] * 0.00390625f; // Divided by 256
			mpTexels[i].g = pRawTex[4 * i + 1] * 0.00390625f;
			mpTexels[i].b = pRawTex[4 * i + 2] * 0.00390625f;
			mpTexels[i].a = 1.0f;
		}

		SafeDeleteArray(pRawTex);
	}

	template<class T>
	T ImageTexture<T>::Sample(const Vector2& texCoord) const
	{
		int iU = Math::FloorToInt(texCoord.u * mTexWidth);
		int iV = Math::FloorToInt((1.0f - texCoord.v) * mTexHeight);

		iU = iU % mTexWidth;
		if (iU < 0)
			iU += mTexWidth;
		iV = iV % mTexHeight;
		if (iV < 0)
			iV += mTexHeight;

		return mpTexels[iV * mTexWidth + iU];
	}

	template class ImageTexture<Color>;
}