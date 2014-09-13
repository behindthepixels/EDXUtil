#pragma once

#include "../EDXPrerequisites.h"
#include "../Graphics/Color.h"
#include "../Math/Vector.h"
#include "../Memory/Array.h"

namespace EDX
{
	template<uint Dim, class T>
	class Texture
	{
	public:
		virtual ~Texture() {}
		virtual T Sample(const Vec<Dim, float>& texCoord) const = 0;
	};

	template<class T>
	using Texture2D = Texture < 2, T >;
	template<class T>
	using Texture3D = Texture < 3, T >;

	template<uint Dim, class T>
	class ConstantTexture : public Texture<Dim, T>
	{
	private:
		T mVal;

	public:
		ConstantTexture(const T& val)
			: mVal(val) {}

		T Sample(const Vec<Dim, float>& texCoord) const
		{
			return mVal;
		}
	};

	template<class T>
	using ConstantTexture2D = ConstantTexture < 2, T >;
	template<class T>
	using ConstantTexture3D = ConstantTexture < 3, T >;

	template<uint Dim, typename T>
	class Mipmap
	{
	private:
		Vec<Dim, int> mOffsetTable[Math::Pow2<Dim>::Value];
		Vec<Dim, int> mTexDims;
		int mNumLevels;

	public:
		Array<Dim, T>* mpLeveledTexels;
		Mipmap()
		{
			for (uint i = 0; i < Math::Pow2<Dim>::Value; i++)
				for (uint d = 0; d < Dim; d++)
					mOffsetTable[i][d] = (i & (1 << d)) != 0;
		}

		~Mipmap()
		{
			SafeDeleteArray(mpLeveledTexels);
		}

		void Generate(const Vec<Dim, int>& dims, const T* pRawTex);

		T Sample(const Vec<Dim, float>& texCoord, const int lod) const;
	};

	template<class T>
	using Mipmap2D = Mipmap < 2, T >;
	template<class T>
	using Mipmap3D = Mipmap < 3, T >;

	template<typename TRet, typename TMem>
	class ImageTexture : public Texture2D<TRet>
	{
	private:
		int mTexWidth;
		int mTexHeight;

		TMem* mpTexels;

		Mipmap2D<TMem> mTexels;

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