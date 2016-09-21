#pragma once

#include "../Core/Types.h"
#include "EDXMath.h"
#include "../Core/Memory.h"
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
				Assert(IsPowOfTwo(iDim));

				miDimention = iDim;

				miNumButterflies = logf(iDim) / logf(2.0f);
				mbIsPingTarget = true;

				CreateButterflyRes();
				mpFDataPing = new FouriorData[miDimention];
				mpFDataPong = new FouriorData[miDimention];
			}

			void Init2D(int iDim)
			{
				Assert(IsPowOfTwo(iDim));

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
				Assert(IsPowOfTwo(iDim));

				miDimention = iDim;
				miNumButterflies = logf(iDim) / logf(2.0f);
				mbIsPingTarget = true;

				Memory::SafeDeleteArray(mpButterFlyData);
				Memory::SafeDeleteArray(mpFDataPing);
				Memory::SafeDeleteArray(mpFDataPong);

				CreateButterflyRes();

				mpFDataPing = new FouriorData[miDimention * miDimention];
				mpFDataPong = new FouriorData[miDimention * miDimention];
			}
			~FFT()
			{
				Memory::SafeDeleteArray(mpButterFlyData);
				Memory::SafeDeleteArray(mpFDataPing);
				Memory::SafeDeleteArray(mpFDataPong);
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

