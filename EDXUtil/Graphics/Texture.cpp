#include "Texture.h"
#include "Color.h"
#include "../Math/EDXMath.h"
#include "../Windows/Bitmap.h"

namespace EDX
{
	template<uint Dim, typename T>
	void Mipmap<Dim, T>::Generate(const Vec<Dim, int>& dims, const T* pRawTex)
	{
		mTexDims = dims;
		mNumLevels = Math::CeilLog2(Math::Max(mTexDims));

		mpLeveledTexels = new Array<Dim, T>[mNumLevels];
		mpLeveledTexels[0].Init(mTexDims);
		memcpy(mpLeveledTexels[0].ModifiableData(), pRawTex, mpLeveledTexels[0].LinearSize() * sizeof(T));

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
					const auto& vOffset = mOffsetTable[i];
					Vec<Dim, int> sampleIndex;
					for (auto d = 0; d < Dim; d++)
						sampleIndex[d] = mOffsetTable[s][d] == 0 ? idx0[d] : idx1[d];

					sum += mpLeveledTexels[prevLevel][sampleIndex];
				}

				mpLeveledTexels[l][i] = sum / Math::Pow2<Dim>::Value;
			}
		}
	}

	// Specialization for Color4b in case of unsigned char overflow
	void Mipmap<2, Color4b>::Generate(const Vector2i& dims, const Color4b* pRawTex)
	{
		mTexDims = dims;
		mNumLevels = Math::CeilLog2(Math::Max(mTexDims));

		mpLeveledTexels = new Array<2, Color4b>[mNumLevels];
		mpLeveledTexels[0].Init(mTexDims);
		memcpy(mpLeveledTexels[0].ModifiableData(), pRawTex, mpLeveledTexels[0].LinearSize() * sizeof(Color4b));

		Vec<2, int> levelDims = mTexDims;
		for (auto l = 1; l < mNumLevels; l++)
		{
			const auto level = l;
			const auto prevLevel = l - 1;
			levelDims >>= 1;
			levelDims[0] = Math::Max(1, levelDims[0]);
			levelDims[1] = Math::Max(1, levelDims[1]);

			mpLeveledTexels[l].Init(levelDims);
			for (auto i = 0; i < mpLeveledTexels[l].LinearSize(); i++)
			{
				const Vector2i idx = mpLeveledTexels[l].Index(i);
				Vector2i idx0, idx1;
				for (auto d = 0; d < 2; d++)
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

				const Color4b& s0 = mpLeveledTexels[prevLevel][idx0];
				const Color4b& s1 = mpLeveledTexels[prevLevel][Vector2i(idx0.u, idx1.v)];
				const Color4b& s2 = mpLeveledTexels[prevLevel][Vector2i(idx1.u, idx0.v)];
				const Color4b& s3 = mpLeveledTexels[prevLevel][idx1];

				mpLeveledTexels[l][i] = Color4b((s0.r + s1.r + s2.r + s3.r) >> 2,
					(s0.g + s1.g + s2.g + s3.g) >> 2,
					(s0.b + s1.b + s2.b + s3.b) >> 2,
					(s0.a + s1.a + s2.a + s3.a) >> 2);
			}
		}
	}

	template<uint Dim, typename T>
	T Mipmap<Dim, T>::Sample(const Vec<Dim, float>& texCoord, const int lod) const
	{
		assert(lod < mNumLevels);
		const Array<Dim, T>& sampledLevel = mpLeveledTexels[lod];

		Vec<Dim, int> u = texCoord * sampledLevel.Size();
		for (auto d = 0; d < Dim; d++)
		{
			u[d] = u[d] % sampledLevel.Size(d);
			if (u[d] < 0)
				u[d] += sampledLevel.Size(d);
		}

		return sampledLevel[u];
	}


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

		mTexels.Generate(Vector2i(mTexWidth, mTexHeight), (TMem*)pRawTex);

		SafeDeleteArray(pRawTex);

	}

	template<typename TRet, typename TMem>
	TRet ImageTexture<TRet, TMem>::Sample(const Vector2& texCoord) const
	{
		return TRet(mTexels.Sample(texCoord, 0));
	}

	template class ImageTexture<Color, Color4b>;
	template class Mipmap<2, Color4b>;
}