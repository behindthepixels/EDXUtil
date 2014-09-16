#pragma once

#include "../EDXPrerequisites.h"
#include "../Graphics/Color.h"
#include "../Math/Vector.h"
#include "../Memory/BlockedArray.h"

namespace EDX
{
	enum class TextureFilter
	{
		Nearest = 0,
		Linear = 1,
		TriLinear = 2,
		Anisotropic4x = 3,
		Anisotropic8x = 4,
		Anisotropic16x = 5
	};

	enum class TextureWrapMode
	{
		Clamp, Repeat, Mirror
	};

	template<uint Dim, class T>
	class Texture
	{
	public:
		virtual ~Texture() {}
		virtual T Sample(const Vec<Dim, float>& texCoord, const Vec<Dim, float> differentials[Dim]) const = 0;
		virtual void SetFilter(const TextureFilter filter)
		{
		}
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

		T Sample(const Vec<Dim, float>& texCoord, const Vec<Dim, float> differentials[Dim]) const
		{
			return mVal;
		}
	};

	template<class T>
	using ConstantTexture2D = ConstantTexture < 2, T >;
	template<class T>
	using ConstantTexture3D = ConstantTexture < 3, T >;

	template<uint Dim, typename T, typename Container = Array<Dim, T>>
	class Mipmap
	{
	private:
		Vec<Dim, int> mOffsetTable[Math::Pow2<Dim>::Value];
		Vec<Dim, int> mTexDims;
		int mNumLevels;

	public:
		Container* mpLeveledTexels;
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

		T TrilinearSample(const Vec<Dim, float>& texCoord, const Vec<Dim, float> differentials[Dim]) const;
		T SampleLevel_Linear(const Vec<Dim, float>& texCoord, const int level) const;
		T Sample_Nearest(const Vec<Dim, float>& texCoord) const;

		const int GetNumLevels() const
		{
			return mNumLevels;
		}
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
		float mTexInvWidth, mTexInvHeight;
		TextureFilter mTexFilter;
		Mipmap2D<TMem> mTexels;

	public:
		ImageTexture(const char* strFile);
		~ImageTexture()
		{
		}

		TRet Sample(const Vector2& texCoord, const Vector2 differentials[2]) const;
		TRet AnisotropicSample(const Vector2& texCoord, const Vector2 differentials[2], const int maxRate) const;

		void SetFilter(const TextureFilter filter)
		{
			mTexFilter = filter;
		}

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