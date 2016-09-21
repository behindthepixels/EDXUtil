#pragma once

#include "../Containers/Array.h"

namespace EDX
{

	class MemoryPool
	{
	private:
		uint mBlockSize;
		uint mCurrOffset;
		_byte* mpCurrentBlock;

		Array<_byte*> mUsedBlocks, mAvailableBlocks;

	public:
		MemoryPool(uint uiSize = 32768)
		{
			mBlockSize = uiSize;
			mCurrOffset = 0;
			mpCurrentBlock = Memory::AlignedAlloc<_byte>(mBlockSize, 64);
		}
		~MemoryPool()
		{
			// Free all memories in the destructor
			Memory::Free(mpCurrentBlock);

			for (uint i = 0; i < mUsedBlocks.Size(); i++)
			{
				Memory::Free(mUsedBlocks[i]);
			}

			for (uint i = 0; i < mAvailableBlocks.Size(); i++)
			{
				Memory::Free(mAvailableBlocks[i]);
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
				mUsedBlocks.Add(mpCurrentBlock);

				// Use previously allocated blocks if the requested size is within block size
				if (mAvailableBlocks.Size() > 0 && uiSize < mBlockSize)
				{
					mpCurrentBlock = mAvailableBlocks.Top();
					mAvailableBlocks.Pop();
				}
				else // Else allocate new block in requested size
				{
					mpCurrentBlock = Memory::AlignedAlloc<_byte>(Math::Max(uiSize, mBlockSize));
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
			while (mUsedBlocks.Size())
			{
				mAvailableBlocks.Add(mUsedBlocks.Top());
				mUsedBlocks.Pop();
			}
		}
	};
}
