#pragma once

#include "../EDXPrerequisites.h"
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

	template<class T>
	class ImageTexture : public Texture<T>
	{
	private:
		int mTexWidth;
		int mTexHeight;

		T* mpTexels;

	public:
		ImageTexture(const char* strFile);
		~ImageTexture()
		{
			SafeDeleteArray(mpTexels);
		}

		T Sample(const Vector2& texCoord) const;

		static T GammaCorrect(T tIn, float fGamma = 2.2f)
		{
			return Math::Pow(tIn, fGamma);
		}
		static T ConvertIn(_byte _In)
		{
			return GammaCorrect(_In * 0.00390625f);
		}
	};
}