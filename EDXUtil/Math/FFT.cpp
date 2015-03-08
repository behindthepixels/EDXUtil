#include "FFT.h"

#include <ppl.h>
using namespace concurrency;

namespace EDX
{
	namespace Math
	{
		void FFT::PerformForward1D(float* pfDataIn, float* pfDataOut) const
		{
			mbIsPingTarget = true;

			// Copy source data to the pong buffer
			for(auto i = 0; i < miDimention; i++)
			{
				mpFDataPong[i].x = pfDataIn[i];
				mpFDataPong[i].y = 0.0f;
			}

			Perform1D();

			FouriorData* pTempData = NULL;
			if (mbIsPingTarget)
				pTempData = mpFDataPong;
			else
				pTempData = mpFDataPing;

			for(auto i = 0; i < miDimention; i++)
			{
				pfDataOut[2 * i] = pTempData[i].x / float(miDimention);
				pfDataOut[2 * i + 1] = pTempData[i].y / float(miDimention);
			}
		}

		void FFT::Perform1D() const
		{
			for(auto i = 0; i < miNumButterflies; i++)
			{
				int rowAdd = 4 * i * miDimention;
				FouriorData* pixelSrc = NULL, *pixelDest = NULL;
				SwitchPingPongTarget(pixelSrc, pixelDest);
				for(auto x = 0; x < miDimention; x++)
				{
					int colAdd = 4 * x;
					int fIndexA = int(mpButterFlyData[rowAdd + colAdd]);
					int fIndexB = int(mpButterFlyData[rowAdd + colAdd + 1]);
					float fWeightA = mpButterFlyData[rowAdd + colAdd + 2];
					float fWeightB = mpButterFlyData[rowAdd + colAdd + 3];

					float vSrcA[2] = {0};
					float vSrcB[2] = {0};

					vSrcA[0] = pixelSrc[fIndexA].x;
					vSrcA[1] = pixelSrc[fIndexA].y;
					vSrcB[0] = pixelSrc[fIndexB].x;
					vSrcB[1] = pixelSrc[fIndexB].y;

					float vRes[2] = {0};
					vRes[0] = fWeightA * vSrcB[0] - fWeightB * vSrcB[1];
					vRes[1] = fWeightB * vSrcB[0] + fWeightA * vSrcB[1];

					vSrcA[0] += vRes[0];
					vSrcA[1] += vRes[1];

					if (x == fIndexA)
					{
						pixelDest[fIndexA].x = vSrcA[0];
						pixelDest[fIndexA].y = vSrcA[1];
					}
					else
					{
						pixelDest[x].x = vSrcA[0];
						pixelDest[x].y = vSrcA[1];
					}
				}
			}
		}

		void FFT::PerformForward2D(float* pfDataIn, float* pfDataOut) const
		{
			mbIsPingTarget = true;
			
			// Copy source data to the pong buffer
			//for(auto i = 0; i < miDimention; i++)
			parallel_for(0u, miDimention, [&](int i)
			{
				for(auto j = 0; j < miDimention; j++)
				{
					mpFDataPong[i * miDimention + j].x = pfDataIn[i * miDimention + j];
					mpFDataPong[i * miDimention + j].y = 0.0f;
				}
			});

			Perform2D();

			FouriorData* pTempData = NULL;

			if (mbIsPingTarget)
				pTempData = mpFDataPong;
			else
				pTempData = mpFDataPing;

			//for(auto i = 0; i < miDimention; i++)
			parallel_for(0u, miDimention, [&](int i)
			{
				for(auto j = 0; j < miDimention; j++)
				{
					pfDataOut[i * 2 * miDimention + 2 * j] = pTempData[i * miDimention + j].x;
					pfDataOut[i * 2 * miDimention + 2 * j + 1] = pTempData[i * miDimention + j].y;
				}
			});
		}

		void FFT::PerformInverse2D(float* pfDataIn, float* pfDataOut) const
		{
			mbIsPingTarget = true;

			// Copy source data to the pong buffer
			//for(auto i = 0; i < miDimention; i++)
			parallel_for(0u, miDimention, [&](int i)
			{
				for(auto j = 0; j < miDimention; j++)
				{
					mpFDataPong[i * miDimention + j].x = pfDataIn[i * 2 * miDimention + 2 * j];
					mpFDataPong[i * miDimention + j].y = pfDataIn[i * 2 * miDimention + 2 * j + 1];
				}
			});

			Perform2D();

			FouriorData* pTempData = NULL;

			if (mbIsPingTarget)
				pTempData = mpFDataPong;
			else
				pTempData = mpFDataPing;

			//for(auto i = 0; i < miDimention; i++)
			parallel_for(0u, miDimention, [&](int i)
			{
				for(auto j = 0; j < miDimention; j++)
				{
					pTempData[i * miDimention + j].x *= (i + j) % 2 == 1 ? 1.0f : -1.0f;
					pfDataOut[i * miDimention + j] = (pTempData[i * miDimention + j].x / float(miDimention * miDimention));
				}
			});
		}

		void FFT::Perform2D() const
		{
			// Horizontal pass.
			for (auto i = 0; i < miNumButterflies; i++)
			{
				int rowAdd = 4 * i * miDimention;
				FouriorData* pixelSrc = NULL, *pixelDest = NULL;
				SwitchPingPongTarget(pixelSrc, pixelDest);
				//for (auto x = 0; x < miDimention; x++)
				parallel_for(0u, miDimention, [&](int x)
				{
					int colAdd = 4 * x;
					int fIndexA = int(mpButterFlyData[rowAdd + colAdd]);
					int fIndexB = int(mpButterFlyData[rowAdd + colAdd + 1]);
					float fWeightA = mpButterFlyData[rowAdd + colAdd + 2];
					float fWeightB = mpButterFlyData[rowAdd + colAdd + 3];

					for (auto y = 0; y < miDimention; y++)
					{
						float vSrcA[2] = {0};
						float vSrcB[2] = {0};

						vSrcA[0] = pixelSrc[y * miDimention + fIndexA].x;
						vSrcA[1] = pixelSrc[y * miDimention + fIndexA].y;
						vSrcB[0] = pixelSrc[y * miDimention + fIndexB].x;
						vSrcB[1] = pixelSrc[y * miDimention + fIndexB].y;

						float vRes[2] = {0};
						vRes[0] = fWeightA * vSrcB[0] - fWeightB * vSrcB[1];
						vRes[1] = fWeightB * vSrcB[0] + fWeightA * vSrcB[1];

						vSrcA[0] += vRes[0];
						vSrcA[1] += vRes[1];

						if (x == fIndexA)
						{
							pixelDest[y * miDimention + fIndexA].x = vSrcA[0];
							pixelDest[y * miDimention + fIndexA].y = vSrcA[1];
						}
						else
						{
							pixelDest[y * miDimention + x].x = vSrcA[0];
							pixelDest[y * miDimention + x].y = vSrcA[1];
						}
					}
				});
			}


			// Vertical pass.
			for (auto i = 0; i < miNumButterflies; i++)
			{
				int rowAdd = 4 * i * miDimention;
				FouriorData* pixelSrc = NULL, *pixelDest = NULL;
				SwitchPingPongTarget(pixelSrc, pixelDest);
				//for (auto y = 0; y < miDimention; y++)
				parallel_for(0u, miDimention, [&](int y)
				{
					int colAdd = 4 * y;
					int fIndexA = int(mpButterFlyData[rowAdd + colAdd]);
					int fIndexB = int(mpButterFlyData[rowAdd + colAdd + 1]);
					float fWeightA = mpButterFlyData[rowAdd + colAdd + 2];
					float fWeightB = mpButterFlyData[rowAdd + colAdd + 3];

					for (auto x = 0; x < miDimention; x++)
					{
						float vSrcA[2] = {0};
						float vSrcB[2] = {0};

						vSrcA[0] = pixelSrc[fIndexA * miDimention + x].x;
						vSrcA[1] = pixelSrc[fIndexA * miDimention + x].y;
						vSrcB[0] = pixelSrc[fIndexB * miDimention + x].x;
						vSrcB[1] = pixelSrc[fIndexB * miDimention + x].y;

						float vRes[2] = {0};
						vRes[0] = fWeightA * vSrcB[0] - fWeightB * vSrcB[1];
						vRes[1] = fWeightB * vSrcB[0] + fWeightA * vSrcB[1];

						vSrcA[0] += vRes[0];
						vSrcA[1] += vRes[1];

						if (y == fIndexA)
						{
							pixelDest[fIndexA * miDimention + x].x = vSrcA[0];
							pixelDest[fIndexA * miDimention + x].y = vSrcA[1];
						}
						else
						{
							pixelDest[y * miDimention + x].x = vSrcA[0];
							pixelDest[y * miDimention + x].y = vSrcA[1];
						}
					}
				});
			}
		}

		void FFT::CreateButterflyRes()
		{
			mpButterFlyData = new float[miNumButterflies * 4 * miDimention];
			float* pfIndices = new float[miNumButterflies * 2 * miDimention];
			float* pfWeights = new float[miNumButterflies * 2 * miDimention];

			CalcIndices(pfIndices);
			CalcWeights(pfWeights);

			for (auto row = 0; row < miNumButterflies; row++)
			{
				auto rowAdd = 4 * row * miDimention;
				for (auto col = 0; col < miDimention; col++)
				{
					int colAdd = 4 * col;
					mpButterFlyData[rowAdd + colAdd] = pfIndices[row * 2 * miDimention + 2 * col];
					mpButterFlyData[rowAdd + colAdd + 1] = pfIndices[row * 2 * miDimention + 2 * col + 1];
					mpButterFlyData[rowAdd + colAdd + 2] = pfWeights[row * 2 * miDimention + 2 * col];
					mpButterFlyData[rowAdd + colAdd + 3] = pfWeights[row * 2 * miDimention + 2 * col + 1];
				}
			}

			SafeDeleteArray(pfIndices);
			SafeDeleteArray(pfWeights);
		}

		void FFT::CalcIndices(float* pfIndices) const
		{
			int iNumIter = miDimention;
			int iOffset = 1;
			int iStep;
			for (auto i = 0; i < miNumButterflies; i++)
			{
				iNumIter = iNumIter >> 1;
				iStep = 2 * iOffset;
				int iEnd = iStep;
				int iStart = 0;
				int iP = 0;
				for (auto j = 0; j < iNumIter; j++)
				{
					for (auto k = iStart, l = 0, v = iP; k < iEnd; k += 2, l += 2, v++)
					{
						pfIndices[i * 2 * miDimention + k] = float(v);
						pfIndices[i * 2 * miDimention + k + 1] = float(v + iOffset);
						pfIndices[i * 2 * miDimention + l + iEnd] = float(v);
						pfIndices[i * 2 * miDimention + l + iEnd + 1] = float(v + iOffset);
					}
					iStart += 2 * iStep;
					iEnd += 2 * iStep;
					iP += iStep;
				}
				iOffset = iOffset << 1;
			}

			float* pfIndicesTmp = new float[2 * miDimention];
			for (auto i = 0; i < 2 * miDimention; i += 2)
			{
				pfIndicesTmp[i] = pfIndices[i];
				pfIndicesTmp[i + 1] = pfIndices[i + 1];
			}

			BitReverse(pfIndicesTmp, 2 * miDimention, miNumButterflies);

			for (auto i = 0; i < 2 * miDimention; i += 2)
			{
				pfIndices[i] = pfIndicesTmp[i];
				pfIndices[i + 1] = pfIndicesTmp[i + 1];
			}
		}

		void FFT::CalcWeights(float* pfWeights) const
		{
			int iNumIter = miDimention / 2;
			int iNumK = 1;
			for (auto i = 0; i < miNumButterflies; i++)
			{
				int iStart = 0;
				int iEnd = 2 * iNumK;

				for (auto b = 0; b < iNumIter; b++)
				{
					int K = 0;
					for (auto k = iStart; k < iEnd; k += 2)
					{
						pfWeights[i * 2 * miDimention + k] = float(Math::Cos(2.0f * float(float(Math::EDX_PI)) * K * iNumIter / float(miDimention)));
						pfWeights[i * 2 * miDimention + k + 1] = float(-Math::Sin(2.0f * float(float(Math::EDX_PI)) * K * iNumIter / float(miDimention)));
						pfWeights[i * 2 * miDimention + k + 2 * iNumK] = float(-Math::Cos(2.0f * float(float(Math::EDX_PI)) * K * iNumIter / float(miDimention)));
						pfWeights[i * 2 * miDimention + k + 2 * iNumK + 1] = float(Math::Sin(2.0f * float(float(Math::EDX_PI)) * K * iNumIter / float(miDimention)));
						K++;
					}
					iStart += 4 * iNumK;
					iEnd = iStart + 2 * iNumK;
				}
				iNumIter = iNumIter >> 1;
				iNumK = iNumK << 1;
			}
		}

		void FFT::SwitchPingPongTarget(FouriorData*& pSrc, FouriorData*& pDest) const
		{
			if (mbIsPingTarget)
			{
				pSrc = mpFDataPong;
				pDest = mpFDataPing;
			}
			else
			{
				pSrc = mpFDataPing;
				pDest = mpFDataPong;
			}
			mbIsPingTarget = !mbIsPingTarget;
		}

		void FFT::BitReverse(float* pfIndices, int N, int n) const
		{
			uint mask = 0x1;
			for (auto j = 0; j < N; j++)
			{
				uint val = 0x0;
				uint temp = uint(pfIndices[j]);
				for (auto i = 0; i < n; i++)
				{
					uint t = (mask & temp);
					val = (val << 1) | t;
					temp = temp >> 1;
				}
				pfIndices[j] = float(val);
			}
		}
	}
}
