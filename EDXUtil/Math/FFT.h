#pragma once

#include "../EDXPrerequisites.h"
#include "EDXMath.h"
#include "../Memory/Memory.h"
#include "Constants.h"

namespace EDX
{
	namespace Math
	{
		struct FouriorData
		{
			float x;
			float y;
		};

		class FFT
		{
		private:
			uint miDimention;
			uint miNumButterflies;
			float* mpButterFlyData;

			mutable bool mbIsPingTarget;
			mutable FouriorData* mpFDataPing;
			mutable FouriorData* mpFDataPong;

		public:
			FFT()
				: mpButterFlyData(NULL)
			{
			}
			
			void Init1D(int iDim)
			{
				assert(IsPowOfTwo(iDim));

				miDimention = iDim;

				miNumButterflies = logf(iDim) / logf(2.0f);
				mbIsPingTarget = true;

				CreateButterflyRes();
				mpFDataPing = new FouriorData[miDimention];
				mpFDataPong = new FouriorData[miDimention];
			}

			void Init2D(int iDim)
			{
				assert(IsPowOfTwo(iDim));

				miDimention = iDim;

				miNumButterflies = logf(iDim) / logf(2.0f);
				mbIsPingTarget = true;

				CreateButterflyRes();
				mpFDataPing = new FouriorData[miDimention * miDimention];
				mpFDataPong = new FouriorData[miDimention * miDimention];
			}

			void PerformForward1D(float* pfDataIn, float* pfDataOut) const;
			void Perform1D() const;

			void PerformForward2D(float* pfDataIn, float* pfDataOut) const;
			void PerformInverse2D(float* pfDataIn, float* pfDataOut) const;
			void Perform2D() const;

			void SetDim(uint iDim)
			{
				assert(IsPowOfTwo(iDim));

				miDimention = iDim;
				miNumButterflies = logf(iDim) / logf(2.0f);
				mbIsPingTarget = true;

				SafeDeleteArray(mpButterFlyData);
				SafeDeleteArray(mpFDataPing);
				SafeDeleteArray(mpFDataPong);

				CreateButterflyRes();

				mpFDataPing = new FouriorData[miDimention * miDimention];
				mpFDataPong = new FouriorData[miDimention * miDimention];
			}
			~FFT()
			{
				SafeDeleteArray(mpButterFlyData);
				SafeDeleteArray(mpFDataPing);
				SafeDeleteArray(mpFDataPong);
			}

		private:
			void CreateButterflyRes();
			void CalcIndices(float* pfIndices) const;
			void CalcWeights(float* pfWeights) const;
			void SwitchPingPongTarget(FouriorData*& pSrc, FouriorData*& pDest) const;
			void BitReverse(float* pfIndices, int N, int n) const;
		};
	}
}

