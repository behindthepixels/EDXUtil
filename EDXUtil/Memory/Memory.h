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
		uint miBlockSize;
		uint miCurrOffset;
		byte* mpCurrentBlock;

		vector<byte*> mvUsedBlocks, mvAvailableBlocks;

	public:
		MemoryArena(uint uiSize = 32768)
		{
			miBlockSize = uiSize;
			miCurrOffset = 0;
			mpCurrentBlock = AllocAligned<byte>(miBlockSize);
		}
		~MemoryArena()
		{
			// Free all memories in the destructor
			FreeAligned(mpCurrentBlock);

			for (uint i = 0; i < mvUsedBlocks.size(); i++)
			{
				FreeAligned(mvUsedBlocks[i]);
			}

			for (uint i = 0; i < mvAvailableBlocks.size(); i++)
			{
				FreeAligned(mvAvailableBlocks[i]);
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
			if (miCurrOffset + uiSize > miBlockSize)
			{
				// Cache the current block for future use
				mvUsedBlocks.push_back(mpCurrentBlock);

				// Use previously allocated blocks if the requested size is within block size
				if (mvAvailableBlocks.size() > 0 && uiSize < miBlockSize)
				{
					mpCurrentBlock = mvAvailableBlocks.back();
					mvAvailableBlocks.pop_back();
				}
				else // Else allocate new block in requested size
				{
					mpCurrentBlock = AllocAligned<byte>(Math::Max(uiSize, miBlockSize));
				}
				// Clear the current block offset
				miCurrOffset = 0;
			}

			T* pRet = (T*)(mpCurrentBlock + miCurrOffset);
			miCurrOffset += uiSize;

			return pRet;
		}

		inline void FreeAll()
		{
			miCurrOffset = 0;
			while (mvUsedBlocks.size())
			{
				mvAvailableBlocks.push_back(mvUsedBlocks.back());
				mvUsedBlocks.pop_back();
			}
		}
	};
}