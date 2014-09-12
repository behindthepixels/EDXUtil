#pragma once

#include "../EDXPrerequisites.h"
#include "../Graphics/Color.h"
#include "../Math/Vector.h"
#include "../Memory/Memory.h"

namespace EDX
{
	template<class T>
	class Texture
	{
	public:
		virtual ~Texture() {}
		virtual T Sample(const Vector2& texCoord) const = 0;
	};

	template<class T>
	class ConstantTexture : public Texture<T>
	{
	private:
		T mVal;

	public:
		ConstantTexture(const T& val)
			: mVal(val) {}

		T Sample(const Vector2& texCoord) const
		{
			return mVal;
		}
	};

	template<typename TRet, typename TMem>
	class ImageTexture : public Texture<TRet>
	{
	private:
		int mTexWidth;
		int mTexHeight;

		TMem* mpTexels;

	public:
		ImageTexture(const char* strFile);
		~ImageTexture()
		{
			FreeAligned(mpTexels);
		}

		TRet Sample(const Vector2& texCoord) const;

		//static T GammaCorrect(T tIn, float fGamma = 2.2f)
		//{
		//	return Math::Pow(tIn, fGamma);
		//}
		//static Color ConvertOut(const Color4b& in)
		//{
		//	return in * 0.00390625f;
		//}
		//static Color4b ConvertIn(const Color4b& in)
		//{
		//	return in;
		//}
	};
}