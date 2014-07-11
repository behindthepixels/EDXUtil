///////////////////////////////////////////////////////////////////////////// 
// The Software is provided "AS IS" and possibly with faults.  
// Intel disclaims any and all warranties and guarantees, express, implied or 
// otherwise, arising, with respect to the software delivered hereunder, 
// including but not limted to the warranty of merchantability, the warranty 
// of fitness for a particular purpose, and any warranty of non-infringement 
// of the intellectual property rights of any third party. 
// Intel neither assumes nor authorizes any person to assume for it any other 
// liability. Customer will use the software at its own risk. Intel will not 
// be liable to customer for any direct or indirect damages incurred in using 
// the software. In no event will Intel be liable for loss of profits, loss of 
// use, loss of data, business interruption, nor for punitive, incidental, 
// consequential, or special damages of any kind, even if advised of 
// the possibility of such damages. 
// 
// Copyright (c) 2003 Intel Corporation 
// 
// Third-party brands and names are the property of their respective owners 
// 
/////////////////////////////////////////////////////////////////////////// 
// RandomGen Number Generation for SSE / SSE2 
// Source File 
// Version 0.1 
// Author Kipp Owens, Rajiv Parikh 
//////////////////////////////////////////////////////////////////////// 
#pragma once

#include "emmintrin.h"

#include "../EDXPrerequisites.h"
#include "../Math/EDXMath.h"
#include <ctime>

//#define COMPATABILITY 
//define this if you wish to return values simlar to the standard rand();

namespace EDX
{
	class RandomGen
	{
	private:
		__m128i* mpCurSeed;

	public:
		RandomGen(unsigned int seed = (unsigned int)time(NULL))
		{
			mpCurSeed = (__m128i*)_aligned_malloc(sizeof(__m128i), 16);
			*mpCurSeed = _mm_set_epi32(seed, seed + 1, seed, seed + 1);
		}

		~RandomGen()
		{
			_aligned_free(mpCurSeed);
		}

		inline unsigned int UnsignedInt()
		{
			__declspec(align(16)) unsigned int result[4];
			__declspec(align(16)) __m128i cur_seed_split;
			__declspec(align(16)) __m128i multiplier;
			__declspec(align(16)) __m128i adder;
			__declspec(align(16)) __m128i mod_mask;
			__declspec(align(16)) __m128i sra_mask;
			__declspec(align(16)) static const unsigned int mult[4] =
			{ 214013, 17405, 214013, 69069 };
			__declspec(align(16)) static const unsigned int gadd[4] =
			{ 2531011, 10395331, 13737667, 1 };
			__declspec(align(16)) static const unsigned int mask[4] =
			{ 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 };
			__declspec(align(16)) static const unsigned int masklo[4] =
			{ 0x00007FFF, 0x00007FFF, 0x00007FFF, 0x00007FFF };

			adder = _mm_load_si128((__m128i*) gadd);
			multiplier = _mm_load_si128((__m128i*) mult);
			mod_mask = _mm_load_si128((__m128i*) mask);
			sra_mask = _mm_load_si128((__m128i*) masklo);
			cur_seed_split = _mm_shuffle_epi32(*mpCurSeed, _MM_SHUFFLE(2, 3, 0, 1));

			*mpCurSeed = _mm_mul_epu32(*mpCurSeed, multiplier);
			multiplier = _mm_shuffle_epi32(multiplier, _MM_SHUFFLE(2, 3, 0, 1));
			cur_seed_split = _mm_mul_epu32(cur_seed_split, multiplier);
			*mpCurSeed = _mm_and_si128(*mpCurSeed, mod_mask);
			cur_seed_split = _mm_and_si128(cur_seed_split, mod_mask);
			cur_seed_split = _mm_shuffle_epi32(cur_seed_split, _MM_SHUFFLE(2, 3, 0, 1));
			*mpCurSeed = _mm_or_si128(*mpCurSeed, cur_seed_split);
			*mpCurSeed = _mm_add_epi32(*mpCurSeed, adder);

#ifdef COMPATABILITY 
			__declspec(align(16)) __m128i sseresult;
			// Add the lines below if you wish to reduce your results to 16-bit vals... 
			sseresult = _mm_srai_epi32(*mpCurSeed, 16);
			sseresult = _mm_and_si128(sseresult, sra_mask);
			_mm_storeu_si128((__m128i*) result, sseresult);
			return;
#endif 
			_mm_storeu_si128((__m128i*) result, *mpCurSeed);
			return result[0];
		}

		inline float Float()
		{
			__declspec(align(16)) unsigned int temp;
			temp = UnsignedInt();
#ifdef COMPATABILITY
			return temp / (float)(0xFFFF);
#else
			return temp / (float)(0xFFFFFFFF);
#endif
		}

		inline float GaussFloat()
		{
			float u1 = Float();
			float u2 = Float();
			if (u1 < 1e-6f)
				u1 = 1e-6f;
			return Math::Sqrt(-2.0f * logf(u1)) * Math::Cos(2 * float(Math::EDX_PI) * u2);
		}
	};
}
