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
		virtual T Sample(const Vec<Dim, float>& texCoord, const Vec<Dim, float> differentials[Dim], TextureFilter filter) const = 0;
		virtual bool HasAlpha() const { return false; }
		virtual int Width() const { return 0; }
		virtual int Height() const { return 0; }
		virtual void SetFilter(const TextureFilter filter)
		{
		}

		virtual bool Editable() const { return false; }
		virtual T GetValue() const = 0;
		virtual void SetValue(const T& value) = 0;
	};

	template<class T>
	using Texture2D = Texture < 2, T >;
	template<class T>
	using Texture3D = Texture < 3, T >;

	template<uint Dim, class T>
	class ConstantTexture : public Texture < Dim, T >
	{
	private:
		T mVal;

	public:
		ConstantTexture(const T& val)
			: mVal(val) {}

		__forceinline T Sample(const Vec<Dim, float>& texCoord, const Vec<Dim, float> differentials[Dim]) const
		{
			return mVal;
		}
		__forceinline T Sample(const Vec<Dim, float>& texCoord, const Vec<Dim, float> differentials[Dim], TextureFilter filter) const
		{
			return mVal;
		}

		bool Editable() const { return true; }
		T GetValue() const
		{
			return mVal;
		}
		void SetValue(const T& value)
		{
			this->mVal = value;
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

		T LinearSample(const Vec<Dim, float>& texCoord, const Vec<Dim, float> differentials[Dim]) const;
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
		bool mHasAlpha;
		TextureFilter mTexFilter;
		Mipmap2D<TMem> mTexels;

	public:
		ImageTexture(const char* strFile, const float gamma = 2.2f);
		ImageTexture(const TMem* pTexels, const int width, const int height);
		~ImageTexture()
		{
		}

		TRet Sample(const Vector2& texCoord, const Vector2 differentials[2]) const;
		TRet Sample(const Vector2& texCoord, const Vector2 differentials[2], TextureFilter filter) const;
		TRet AnisotropicSample(const Vector2& texCoord, const Vector2 differentials[2], const int maxRate) const;

		void SetFilter(const TextureFilter filter)
		{
			mTexFilter = filter;
		}
		int Width() const
		{
			return mTexWidth;
		}
		int Height() const
		{
			return mTexHeight;
		}
		bool HasAlpha() const
		{
			return mHasAlpha;
		}

		bool Editable() const { return false; }
		TRet GetValue() const
		{
			return TRet(0);
		}
		void SetValue(const TRet& value)
		{
		}

		static TMem GammaCorrect(TMem tIn, float fGamma = 2.2)
		{
			return Math::Pow(tIn, fGamma);
		}
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