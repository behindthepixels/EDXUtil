#pragma once

#include "../EDXPrerequisites.h"
#include "../Math/EDXMath.h"

namespace EDX
{
	template<class T>
	inline void SafeDelete(T*& pPtr)
	{
		if (pPtr != NULL)
		{
			delete pPtr;
			pPtr = NULL;
		}
	}

	template<class T>
	inline void SafeDeleteArray(T*& pPtr)
	{
		if (pPtr != NULL)
		{
			delete[] pPtr;
			pPtr = NULL;
		}
	}

	template<class T>
	inline void SafeClear(T* pPtr, size_t iSize)
	{
		if (pPtr != NULL)
		{
			memset(pPtr, 0, sizeof(T) * iSize);
		}
	}

	template<class T> T* AllocAligned(uint uiCount, uint uiAligned = 64/*for 64-byte cache line*/)
	{
		return (T*)_aligned_malloc(uiCount * sizeof(T), uiAligned);
	}

	template<class T> void FreeAligned(T*& pPtr)
	{
		if (pPtr)
		{
			_aligned_free(pPtr);
			pPtr = NULL;
		}
	}

	class MemoryArena
	{
	private:
		uint mBlockSize;
		uint mCurrOffset;
		_byte* mpCurrentBlock;

		vector<_byte*> mUsedBlocks, mAvailableBlocks;

	public:
		MemoryArena(uint uiSize = 32768)
		{
			mBlockSize = uiSize;
			mCurrOffset = 0;
			mpCurrentBlock = AllocAligned<_byte>(mBlockSize);
		}
		~MemoryArena()
		{
			// Free all memories in the destructor
			FreeAligned(mpCurrentBlock);

			for (uint i = 0; i < mUsedBlocks.size(); i++)
			{
				FreeAligned(mUsedBlocks[i]);
			}

			for (uint i = 0; i < mAvailableBlocks.size(); i++)
			{
				FreeAligned(mAvailableBlocks[i]);
			}
		}

		template<class T>
		inline T* Alloc(uint uiCount = 1)
		{
			// Calculate the size of requested memory in terms of byte
			uint uiSize = uiCount * sizeof(T);

			// Make it aligned to 16 byte
			uiSize = (uiSize + 15) & (~15);

			// Handle situation where the current block is used up
			if (mCurrOffset + uiSize > mBlockSize)
			{
				// Cache the current block for future use
				mUsedBlocks.push_back(mpCurrentBlock);

				// Use previously allocated blocks if the requested size is within block size
				if (mAvailableBlocks.size() > 0 && uiSize < mBlockSize)
				{
					mpCurrentBlock = mAvailableBlocks.back();
					mAvailableBlocks.pop_back();
				}
				else // Else allocate new block in requested size
				{
					mpCurrentBlock = AllocAligned<_byte>(Math::Max(uiSize, mBlockSize));
				}
				// Clear the current block offset
				mCurrOffset = 0;
			}

			T* pRet = (T*)(mpCurrentBlock + mCurrOffset);
			mCurrOffset += uiSize;

			return pRet;
		}

		inline void FreeAll()
		{
			mCurrOffset = 0;
			while (mUsedBlocks.size())
			{
				mAvailableBlocks.push_back(mUsedBlocks.back());
				mUsedBlocks.pop_back();
			}
		}
	};
}