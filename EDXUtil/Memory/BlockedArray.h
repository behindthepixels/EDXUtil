#pragma once

#include "../EDXPrerequisites.h"
#include "Array.h"
#include "Memory.h"
#include "../Math/Vector.h"

namespace EDX
{
	template<size_t Dimension, class T, int LogBlockSize = 2>
	class BlockedArray : public Array<Dimension, T>
	{
	private:
		uint mBlockCount;
		uint mLogBlockElemCount;
		uint mOrgLinearSize;
		Vec<Dimension, uint> mOrgDim;

		ArrayIndex<Dimension> mOrgIndex;
		ArrayIndex<Dimension> mBlockIndex;
		ArrayIndex<Dimension> mIntraBlockIndex;

	public:
		void Init(const Vec<Dimension, uint>& size, bool bClear = true)
		{
			mOrgDim = size;
			mOrgLinearSize = size.Product();
			mOrgIndex.Init(size);
			Vec<Dimension, uint> roundUpSize = RoundUp(size);
			Array<Dimension, T>::Init(roundUpSize, bClear);

			mBlockIndex.Init(mIndex.Size() >> LogBlockSize);
			mIntraBlockIndex.Init(Vec<Dimension, uint>(1 << LogBlockSize));

			mBlockCount = mBlockIndex.LinearSize();
			mLogBlockElemCount = LogBlockSize * Dimension;
		}

		void SetData(const T* pData)
		{
			for (size_t i = 0; i < LinearSize(); i++)
				mpData[mIndex.LinearIndex(Index(i))] = pData[i];
		}

		__forceinline size_t LinearIndex(const Vec<Dimension, uint>& idx) const
		{
			Vec<Dimension, uint> blockIdx = Block(idx);
			Vec<Dimension, uint> blockOffset = Offset(idx);

			size_t blockLinearIdx = mBlockIndex.LinearIndex(blockIdx);
			size_t intraBlockLinearIdx = mIntraBlockIndex.LinearIndex(blockOffset);
			size_t ret = (blockLinearIdx << mLogBlockElemCount) + intraBlockLinearIdx;

			assert(ret < mIndex.LinearSize());
			return ret;
		}

		__forceinline Vec<Dimension, uint> Index(size_t linearIdx) const
		{
			assert(linearIdx < mIndex.LinearSize());
			Vec<Dimension, uint> vRet;

			uint blockLinearIdx = linearIdx >> mLogBlockElemCount;
			uint intraBlockLinearIdx = linearIdx & (mIntraBlockIndex.LinearSize() - 1);

			Vec<Dimension, uint> blockIdx = mBlockIndex.Index(blockLinearIdx);
			Vec<Dimension, uint> intraBlockIdx = mIntraBlockIndex.Index(intraBlockLinearIdx);

			return (blockIdx << LogBlockSize) + intraBlockIdx;
		}

		__forceinline size_t LinearSize() const
		{
			return mOrgLinearSize;
		}
		__forceinline size_t Size(uint iDim) const
		{
			return mOrgDim[iDim];
		}
		__forceinline Vec<Dimension, uint> Size() const
		{
			return mOrgDim;
		}

	private:
		Vec<Dimension, uint> Block(const Vec<Dimension, uint>& idx) const
		{
			return idx >> LogBlockSize;
		}
		Vec<Dimension, uint> Offset(const Vec<Dimension, uint>& idx) const
		{
			return (idx & (BlockSize() - 1));
		}
		uint BlockSize() const
		{
			return 1 << LogBlockSize;
		}
		Vec<Dimension, uint> RoundUp(const Vec<Dimension, uint>& size)
		{
			Vec<Dimension, uint> ret;
			for (auto d = 0; d < Dimension; d++)
				ret[d] = (size[d] + BlockSize() - 1) & ~(BlockSize() - 1);

			return ret;
		}
	};
}