#pragma once

#include "../Core/Types.h"
#include "DimensionalArray.h"
#include "Memory.h"
#include "../Math/Vector.h"

namespace EDX
{
	template<size_t Dimension, class T, int LogBlockSize = 2>
	class BlockedDimensionalArray
	{
	private:
		static const int BLOCK_SIZE = 1 << LogBlockSize;

		T* mpData;

		uint mLogBlockElemCount;
		uint mRoundedSize;
		ArrayIndex<Dimension> mOrgIndex;
		ArrayIndex<Dimension> mBlockIndex;
		ArrayIndex<Dimension> mIntraBlockIndex;

	public:
		BlockedDimensionalArray()
			: mpData(NULL)
		{
		}
		virtual ~BlockedDimensionalArray()
		{
			Free();
		}

		BlockedDimensionalArray& operator = (const BlockedDimensionalArray& rhs)
		{
			if (Size() != rhs.Size())
			{
				Free();
				Init(rhs.Size());
			}

			Memory::Memcpy(mpData, rhs.mpData, mRoundedSize * sizeof(T));
			return *this;
		}
		BlockedDimensionalArray& operator = (BlockedDimensionalArray&& rhs)
		{
			if (Size() != rhs.Size())
			{
				Free();
				Init(rhs.Size());
			}
			mpData = rhs.mpData;
			rhs.mpData = NULL;
			return *this;
		}

		void Init(const Vec<Dimension, uint>& size, bool bClear = true)
		{
			mOrgIndex.Init(size);
			Vec<Dimension, uint> roundUpSize = RoundUp(size);
			mRoundedSize = roundUpSize.Product();

			Memory::Free(mpData);
			mpData = Memory::AlignedAlloc<T>(mRoundedSize);
			Assert(mpData);

			if (bClear)
				Clear();

			mBlockIndex.Init(roundUpSize >> LogBlockSize);
			mIntraBlockIndex.Init(Vec<Dimension, uint>(1 << LogBlockSize));

			mLogBlockElemCount = LogBlockSize * Dimension;
		}

		__forceinline void Clear()
		{
			Memory::SafeClear(mpData, mRoundedSize);
		}

		void SetData(const T* pData)
		{
			for (size_t i = 0; i < LinearSize(); i++)
				mpData[LinearIndex(Index(i))] = pData[i];
		}

		__forceinline size_t LinearIndex(const Vec<Dimension, uint>& idx) const
		{
			Vec<Dimension, uint> blockIdx = Block(idx);
			Vec<Dimension, uint> blockOffset = Offset(idx);

			size_t blockLinearIdx = mBlockIndex.LinearIndex(blockIdx);
			size_t intraBlockLinearIdx = mIntraBlockIndex.LinearIndex(blockOffset);
			size_t ret = (blockLinearIdx << mLogBlockElemCount) + intraBlockLinearIdx;

			Assert(ret < mRoundedSize);
			return ret;
		}

		__forceinline Vec<Dimension, uint> Index(size_t linearIdx) const
		{
			return mOrgIndex.Index(linearIdx);
		}
		__forceinline size_t LinearSize() const
		{
			return mOrgIndex.LinearSize();
		}
		__forceinline size_t Size(uint iDim) const
		{
			return mOrgIndex.Size(iDim);
		}
		__forceinline Vec<Dimension, uint> Size() const
		{
			return mOrgIndex.Size();
		}
		__forceinline size_t Stride(uint iDim) const
		{
			return mOrgIndex.Stride(iDim);
		}

		__forceinline T& operator [] (const Vec<Dimension, uint>& idx) { return mpData[LinearIndex(idx)]; }
		__forceinline const T operator [] (const Vec<Dimension, uint>& idx) const { return mpData[LinearIndex(idx)]; }
		__forceinline T& operator [] (const size_t idx) { Assert(idx < mRoundedSize); return mpData[idx]; }
		__forceinline const T operator [] (const size_t idx) const { Assert(idx <mRoundedSize); return mpData[idx]; }
		__forceinline const T* Data() const { return mpData; }
		__forceinline T* ModifiableData() { return mpData; }

		void Free()
		{
			Memory::Free(mpData);
		}

	private:
		Vec<Dimension, uint> Block(const Vec<Dimension, uint>& idx) const
		{
			return idx >> LogBlockSize;
		}
		Vec<Dimension, uint> Offset(const Vec<Dimension, uint>& idx) const
		{
			return (idx & (BLOCK_SIZE - 1));
		}
		Vec<Dimension, uint> RoundUp(const Vec<Dimension, uint>& size)
		{
			Vec<Dimension, uint> ret;
			for (auto d = 0; d < Dimension; d++)
				ret[d] = (size[d] + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1);

			return ret;
		}
	};
}