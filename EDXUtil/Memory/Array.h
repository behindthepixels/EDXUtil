#pragma once

#include "../EDXPrerequisites.h"
#include "Memory.h"
#include "../Math/Vector.h"

namespace EDX
{
	template<size_t Dimension>
	class ArrayIndex
	{
	protected:
		Vec<Dimension, size_t> mDim;
		Vec<Dimension, size_t> mStrides;
		size_t mArraySize;

	public:
		void Init(const Vec<Dimension, uint>& size)
		{
			mDim = size;
			CalcStrides();

			mArraySize = 1;
			for (auto i = 0; i < Dimension; i++)
				mArraySize *= mDim[i];
		}

		__forceinline size_t LinearIndex(const Vec<Dimension, uint>& idx) const
		{
			size_t ret = 0;
			for (auto i = 0; i < Dimension; i++)
			{
				ret += idx[i] * mStrides[i];
			}
			assert(ret < mArraySize);
			return ret;
		}
		__forceinline Vec<Dimension, uint> Index(size_t linearIdx) const
		{
			assert(linearIdx < mArraySize);
			Vec<Dimension, uint> vRet;
			for (int i = Dimension - 1; i >= 0; i--)
			{
				vRet[i] = linearIdx / mStrides[i];
				linearIdx %= mStrides[i];
			}
			return vRet;
		}

		void CalcStrides()
		{
			for (auto i = 0; i < Dimension; i++)
			{
				mStrides[i] = 1;
				for (auto dim = 0; dim < i; dim++)
					mStrides[i] *= mDim[dim];
			}
		}
		void SetDim(const Vec<Dimension, uint>& size)
		{
			auto oldSize = mArraySize;

			mDim = size;
			CalcStrides();
			mArraySize = 1;
			for (auto i = 0; i < Dimension; i++)
			{
				mArraySize *= mDim[i];
			}

			assert(mArraySize == oldSize);
		}

		__forceinline size_t LinearSize() const
		{
			return mArraySize;
		}
		__forceinline size_t Size(uint iDim) const
		{
			assert(iDim < Dimension);
			return mDim[iDim];
		}
		__forceinline Vec<Dimension, uint> Size() const
		{
			return mDim;
		}
		__forceinline size_t Stride(uint iDim) const
		{
			assert(iDim < Dimension);
			return mStrides[iDim];
		}
	};

	template<size_t Dimension, class T>
	class Array
	{
	protected:
		ArrayIndex<Dimension> mIndex;

	public:
		T* mpData;

	public:
		Array()
			: mpData(NULL)
		{
		}
		virtual ~Array()
		{
			Free();
		}

		void Init(const Vec<Dimension, uint>& size, bool bClear = true)
		{
			FreeAligned(mpData);
			mIndex.Init(size);

			mpData = AllocAligned<T>(mIndex.LinearSize());
			assert(mpData);

			if (bClear)
				Clear();
		}

		void SetDim(const Vec<Dimension, uint>& size)
		{
			mIndex.SetDim(size);
		}

		__forceinline void Clear()
		{
			SafeClear(mpData, mIndex.LinearSize());
		}

		Array& operator = (const Array& rhs)
		{
			if (mDim != rhs.Size())
			{
				Free();
				mIndex.Init(rhs.Size());
			}
			memcpy(mpData, rhs.mpData, mArraySize * sizeof(T));
			return *this;
		}
		Array& operator = (Array&& rhs)
		{
			if (mDim != rhs.Size())
			{
				Free();
				mIndex.Init(rhs.Size());
			}
			mpData = rhs.mpData;
			rhs.mpData = NULL;
			return *this;
		}

		__forceinline virtual size_t LinearIndex(const Vec<Dimension, uint>& idx) const
		{
			return mIndex.LinearIndex(idx);
		}
		__forceinline virtual Vec<Dimension, uint> Index(size_t linearIdx) const
		{
			return mIndex.Index(linearIdx);
		}
		__forceinline size_t LinearSize() const
		{
			return mIndex.LinearSize();
		}
		__forceinline size_t Size(uint iDim) const
		{
			return mIndex.Size(iDim);
		}
		__forceinline Vec<Dimension, uint> Size() const
		{
			return mIndex.Size();
		}
		__forceinline size_t Stride(uint iDim) const
		{
			return mIndex.Stride(iDim);
		}

		__forceinline T& operator [] (const Vec<Dimension, uint>& idx) { return mpData[LinearIndex(idx)]; }
		__forceinline const T operator [] (const Vec<Dimension, uint>& idx) const { return mpData[LinearIndex(idx)]; }
		__forceinline T& operator [] (const size_t idx) { assert(idx < mIndex.LinearSize()); return mpData[idx]; }
		__forceinline const T operator [] (const size_t idx) const { assert(idx < mIndex.LinearSize()); return mpData[idx]; }
		__forceinline const T* Data() const { return mpData; }
		__forceinline T* ModifiableData() { return mpData; }

		void Free()
		{
			FreeAligned(mpData);
		}
	};

	template<size_t Dimension, class T>
	void ToStlVector(vector<T>& lhs, const Array<Dimension, T>& rhs)
	{
		lhs.clear();
		lhs.resize(rhs.LinearSize());
		memcpy(lhs.data(), rhs.Data(), rhs.LinearSize() * sizeof(T));
	}

	typedef Array<1, float> Array1f;
	typedef Array<2, float> Array2f;
	typedef Array<3, float> Array3f;

	typedef Array<1, double> Array1d;
	typedef Array<2, double> Array2d;
	typedef Array<3, double> Array3d;
}