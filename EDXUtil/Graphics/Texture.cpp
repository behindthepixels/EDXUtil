#include "Texture.h"
#include "Color.h"
#include "../Windows/Bitmap.h"

namespace EDX
{
	template<typename TRet, typename TMem>
	ImageTexture<TRet, TMem>::ImageTexture(const char* strFile)
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

		mpTexels = AllocAligned<TMem>(mTexWidth * mTexHeight);

		for (int i = 0; i < mTexWidth * mTexHeight; i++)
		{
			mpTexels[i] = TMem(Color4b(pRawTex[4 * i + 0],
				pRawTex[4 * i + 1],
				pRawTex[4 * i + 2],
				255));
		}

		SafeDeleteArray(pRawTex);
	}

	template<typename TRet, typename TMem>
	TRet ImageTexture<TRet, TMem>::Sample(const Vector2& texCoord) const
	{
		int iU = Math::FloorToInt(texCoord.u * mTexWidth);
		int iV = Math::FloorToInt((1.0f - texCoord.v) * mTexHeight);

		iU = iU % mTexWidth;
		if (iU < 0)
			iU += mTexWidth;
		iV = iV % mTexHeight;
		if (iV < 0)
			iV += mTexHeight;

		return TRet(mpTexels[iV * mTexWidth + iU]);
	}

	template class ImageTexture<Color, Color4b>;
}