#include "Texture.h"
#include "Color.h"
#include "../Math/EDXMath.h"
#include "../Windows/Bitmap.h"

namespace EDX
{
	template<uint Dim, typename T, typename Container>
	void Mipmap<Dim, T, Container>::Generate(const Vec<Dim, int>& dims, const T* pRawTex)
	{
		mTexDims = dims;
		mNumLevels = Math::CeilLog2(Math::Max(mTexDims));

		mpLeveledTexels = new Container[mNumLevels];
		mpLeveledTexels[0].Init(mTexDims);
		mpLeveledTexels[0].SetData(pRawTex);

		Vec<Dim, int> levelDims = mTexDims;
		for (auto l = 1; l < mNumLevels; l++)
		{
			const auto level = l;
			const auto prevLevel = l - 1;
			levelDims >>= 1;
			for (auto d = 0; d < Dim; d++)
				levelDims[d] = Math::Max(1, levelDims[d]);

			mpLeveledTexels[l].Init(levelDims);
			for (auto i = 0; i < mpLeveledTexels[l].LinearSize(); i++)
			{
				const Vec<Dim, int> idx = mpLeveledTexels[l].Index(i);
				Vec<Dim, int> idx0, idx1;
				for (auto d = 0; d < Dim; d++)
				{
					if (levelDims[d] < mpLeveledTexels[prevLevel].Size(d))
					{
						idx0[d] = 2 * idx[d];
						idx1[d] = 2 * idx[d] + 1;
					}
					else
					{
						idx0[d] = idx1[d] = idx[d];
					}
				}

				T sum = 0;
				for (auto s = 0; s < Math::Pow2<Dim>::Value; s++)
				{
					Vec<Dim, int> sampleIndex;
					for (auto d = 0; d < Dim; d++)
						sampleIndex[d] = mOffsetTable[s][d] == 0 ? idx0[d] : idx1[d];

					sum += mpLeveledTexels[prevLevel][sampleIndex] / Math::Pow2<Dim>::Value;
				}

				mpLeveledTexels[l][idx] = sum;
			}
		}
	}

	inline float fast_log2(float val)
	{
		int * const    exp_ptr = reinterpret_cast <int *> (&val);
		int            x = *exp_ptr;
		const int      log_2 = ((x >> 23) & 255) - 128;
		x &= ~(255 << 23);
		x += 127 << 23;
		*exp_ptr = x;

		val = ((-1.0f / 3) * val + 2) * val - 2.0f / 3;

		return (val + log_2);
	}

	template<uint Dim, typename T, typename Container>
	T Mipmap<Dim, T, Container>::TrilinearSample(const Vec<Dim, float>& texCoord, const Vec<Dim, float> differentials[Dim]) const
	{
		float filterWidth = Math::EDX_NEG_INFINITY;
		for (auto d = 0; d < Dim; d++)
			filterWidth = Math::Max(filterWidth, Math::Length(differentials[d]));

		float lod = mNumLevels - 1 + fast_log2(Math::Max(filterWidth, 1e-8f));
		if (lod < 0)
			return SampleLevel_Linear(texCoord, 0);
		if (lod >= mNumLevels - 1)
			return mpLeveledTexels[mNumLevels - 1][0];

		uint lodBase = Math::FloorToInt(lod);
		float lin = lod - lodBase;
		if (lin < 0.2f)
			return SampleLevel_Linear(texCoord, lodBase);
		if (lin > 0.8f)
			return SampleLevel_Linear(texCoord, lodBase + 1);

		return Math::Lerp(SampleLevel_Linear(texCoord, lodBase), SampleLevel_Linear(texCoord, lodBase + 1), lin);
	}

	template<uint Dim, typename T, typename Container>
	T Mipmap<Dim, T, Container>::LinearSample(const Vec<Dim, float>& texCoord, const Vec<Dim, float> differentials[Dim]) const
	{
		float filterWidth = Math::EDX_NEG_INFINITY;
		for (auto d = 0; d < Dim; d++)
			filterWidth = Math::Max(filterWidth, Math::Length(differentials[d]));

		float lod = mNumLevels - 1 + fast_log2(Math::Max(filterWidth, 1e-8f));
		if (lod < 0)
			return SampleLevel_Linear(texCoord, 0);
		else
			return Sample_Nearest(texCoord);
	}

	inline int FastFloor(float x)
	{
		int i = (int)x;
		return i - (i > x);
	}

	template<uint Dim, typename T, typename Container>
	T Mipmap<Dim, T, Container>::SampleLevel_Linear(const Vec<Dim, float>& texCoord, const int level) const
	{
		const Container& sampledLevel = mpLeveledTexels[level];

		Vec<Dim, float> coord = texCoord * sampledLevel.Size() - Vec<Dim, float>(0.5f);

		Vec<Dim, int> coordBase;
		for (auto d = 0; d < Dim; d++)
			coordBase[d] = FastFloor(coord[d]);

		if (coord == coordBase)
			return sampledLevel[coordBase];
		
		Color4b values[Math::Pow2<Dim>::Value];
		for (uint i = 0; i < Math::Pow2<Dim>::Value; i++)
		{
			Vec<Dim, int> sampledCoord = coordBase + mOffsetTable[i];
			for (auto d = 0; d < Dim; d++)
				sampledCoord[d] = Math::Min(sampledCoord[d], sampledLevel.Size(d) - 1);

			values[i] = sampledLevel[sampledCoord];
		}

		return Math::Lerp<Dim>(values, coord - coordBase);
	}

	template<uint Dim, typename T, typename Container>
	T Mipmap<Dim, T, Container>::Sample_Nearest(const Vec<Dim, float>& texCoord) const
	{
		const Container& sampledLevel = mpLeveledTexels[0];
		Vec<Dim, int> u = texCoord * sampledLevel.Size() - Vec<Dim, float>(0.5f);

		return sampledLevel[u];
	}


	template<typename TRet, typename TMem>
	ImageTexture<TRet, TMem>::ImageTexture(const char* strFile, const float gamma)
		: mTexWidth(0)
		, mTexHeight(0)
	{
		int iChannel;
		TMem* pRawTex = (TMem*)Bitmap::ReadFromFileByte(strFile, &mTexWidth, &mTexHeight, &iChannel);
		if (!pRawTex)
		{
			throw std::exception("Texture file load failed.");
		}

		for (auto i = 0; i < mTexWidth * mTexHeight; i++)
			pRawTex[i] = GammaCorrect(pRawTex[i], gamma);

		if (iChannel == 4)
			mHasAlpha = true;
		else
			mHasAlpha = false;

		mTexels.Generate(Vector2i(mTexWidth, mTexHeight), pRawTex);
		mTexInvWidth = 1.0f / float(mTexWidth);
		mTexInvHeight = 1.0f / float(mTexHeight);

		SafeDeleteArray(pRawTex);
	}

	template<typename TRet, typename TMem>
	TRet ImageTexture<TRet, TMem>::Sample(const Vector2& texCoord, const Vector2 differentials[2]) const
	{
		// TODO: Add more wrap modes
		Vector2 wrappedTexCoord;
		wrappedTexCoord.u = texCoord.u - FastFloor(texCoord.u);
		wrappedTexCoord.v = texCoord.v - FastFloor(texCoord.v);

		switch (mTexFilter)
		{
		case TextureFilter::Nearest:
			return mTexels.Sample_Nearest(wrappedTexCoord);
		case TextureFilter::Linear:
			return mTexels.SampleLevel_Linear(wrappedTexCoord, 0);
		case TextureFilter::TriLinear:
			return mTexels.TrilinearSample(wrappedTexCoord, differentials);
		case TextureFilter::Anisotropic4x:
			return AnisotropicSample(wrappedTexCoord, differentials, 4);
		case TextureFilter::Anisotropic8x:
			return AnisotropicSample(wrappedTexCoord, differentials, 8);
		case TextureFilter::Anisotropic16x:
			return AnisotropicSample(wrappedTexCoord, differentials, 16);
		}

		return TRet(0);
	}

	template<typename TRet, typename TMem>
	TRet ImageTexture<TRet, TMem>::Sample(const Vector2& texCoord, const Vector2 differentials[2], TextureFilter filter) const
	{
		// TODO: Add more wrap modes
		Vector2 wrappedTexCoord;
		wrappedTexCoord.u = texCoord.u - FastFloor(texCoord.u);
		wrappedTexCoord.v = texCoord.v - FastFloor(texCoord.v);

		switch (filter)
		{
		case TextureFilter::Nearest:
			return mTexels.Sample_Nearest(wrappedTexCoord);
		case TextureFilter::Linear:
			return mTexels.LinearSample(wrappedTexCoord, differentials);
		case TextureFilter::TriLinear:
			return mTexels.TrilinearSample(wrappedTexCoord, differentials);
		case TextureFilter::Anisotropic4x:
			return AnisotropicSample(wrappedTexCoord, differentials, 4);
		case TextureFilter::Anisotropic8x:
			return AnisotropicSample(wrappedTexCoord, differentials, 8);
		case TextureFilter::Anisotropic16x:
			return AnisotropicSample(wrappedTexCoord, differentials, 16);
		}

		return TRet(0);
	}

	template<typename TRet, typename TMem>
	TRet ImageTexture<TRet, TMem>::AnisotropicSample(const Vector2& texCoord, const Vector2 differentials[2], const int maxRate) const
	{
		float dudx = differentials[0].u;
		float dvdx = differentials[0].v;
		float dudy = differentials[1].u;
		float dvdy = differentials[1].v;

		float A = dvdx * dvdx + dvdy * dvdy;
		float B = -2.0f * (dudx * dvdx + dudy * dvdy);
		float C = dudx * dudx + dudy * dudy;
		float F = (dudx * dvdy - dudy * dvdx);
		F *= F;
		float p = A - C;
		float q = A + C;
		float t = sqrt(p * p + B * B);

		dudx *= mTexWidth; dudy *= mTexWidth;
		dvdx *= mTexHeight; dvdy *= mTexHeight;

		float squaredLengthX = dudx*dudx + dvdx*dvdx;
		float squaredLengthY = dudy*dudy + dvdy*dvdy;
		float determinant = Math::Abs(dudx*dvdy - dvdx*dudy);
		bool isMajorX = squaredLengthX > squaredLengthY;
		float squaredLengthMajor = isMajorX ? squaredLengthX : squaredLengthY;
		float lengthMajor = Math::Sqrt(squaredLengthMajor);
		float normMajor = 1.f / lengthMajor;

		Vector2 anisoDir;
		anisoDir.x = (isMajorX ? dudx : dudy) * normMajor;
		anisoDir.y = (isMajorX ? dvdx : dvdy) * normMajor;

		float ratioOfAnisotropy = squaredLengthMajor / determinant;

		// clamp ratio and compute LOD
		float lengthMinor;
		if (ratioOfAnisotropy > maxRate) // maxAniso comes from a Sampler state.
		{
			// ratio is clamped - LOD is based on ratio (preserves area)
			ratioOfAnisotropy = (float)maxRate;
			lengthMinor = lengthMajor / ratioOfAnisotropy;
		}
		else
		{
			// ratio not clamped - LOD is based on area
			lengthMinor = determinant / lengthMajor;
		}

		// clamp to top LOD
		if (lengthMinor < 1.0f)
		{
			ratioOfAnisotropy = Math::Max(1.0f, ratioOfAnisotropy * lengthMinor);
			lengthMinor = 1.0f;
		}

		float LOD = fast_log2(lengthMinor);
		float invRate = 1.0f / (int)ratioOfAnisotropy;
		float startU = texCoord.u * mTexWidth - lengthMajor * anisoDir.x * 0.5f;
		float startV = texCoord.v * mTexHeight - lengthMajor * anisoDir.y * 0.5f;
		float stepU = lengthMajor * anisoDir.x * invRate;
		float stepV = lengthMajor * anisoDir.y * invRate;
		int lod1, lod2;
		lod1 = Math::Min(Math::FloorToInt(LOD), mTexels.GetNumLevels() - 1);
		lod2 = Math::Min(Math::CeilToInt(LOD), mTexels.GetNumLevels() - 1);

		TMem ret;
		Vector2 uv;
		for (int i = 0; i < (int)ratioOfAnisotropy; i++)
		{
			uv.u = (startU + stepU * (i + 0.5f)) * mTexInvWidth;
			uv.v = (startV + stepV * (i + 0.5f)) * mTexInvHeight;
			float lin = LOD - lod1;
			if (lin < 0.2f)
				ret += mTexels.SampleLevel_Linear(uv, lod1) * invRate;
			else if (lin > 0.8f)
				ret += mTexels.SampleLevel_Linear(uv, lod2) * invRate;
			else
				ret += Math::Lerp(mTexels.SampleLevel_Linear(uv, lod1), mTexels.SampleLevel_Linear(uv, lod2), LOD - lod1) * invRate;
		}

		return ret;
	}

	template class ImageTexture<Color, Color4b>;
	template class Mipmap<2, Color4b>;
}