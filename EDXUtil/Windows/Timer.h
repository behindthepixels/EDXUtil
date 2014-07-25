#pragma once

#include "../EDXPrerequisites.h"

#include "Base.h"

namespace EDX
{
	class Timer
	{
	public:
		double mLastElapsedAbsoluteTime;
		double mBaseAbsoluteTime;

		double mLastElapsedTime;
		double mBaseTime;
		double mStopTime;
		bool mbTimerStopped;

		wchar_t mFrameRate[16];
		dword mNumFrames;
		double mLastFPSTime;

		LARGE_INTEGER mPerfFreq;

		Timer()
		{
			QueryPerformanceFrequency(&mPerfFreq);
			double fTime = GetAbsoluteTime();

			mBaseAbsoluteTime = fTime;
			mLastElapsedAbsoluteTime = fTime;

			mBaseTime = fTime;
			mStopTime = 0.0;
			mLastElapsedTime = fTime;
			mbTimerStopped = false;

			mFrameRate[0] = L'\0';
			mNumFrames = 0;
			mLastFPSTime = fTime;
		}

		double GetAbsoluteTime()
		{
			LARGE_INTEGER Time;
			QueryPerformanceCounter(&Time);
			double fTime = (double)Time.QuadPart / (double)mPerfFreq.QuadPart;
			return fTime;
		}

		double GetTime()
		{
			// Get either the current time or the stop time, depending
			// on whether we're stopped and what comand was sent
			return (mStopTime != 0.0) ? mStopTime : GetAbsoluteTime();
		}

		double GetElapsedTime()
		{
			double fTime = GetAbsoluteTime();

			double fElapsedAbsoluteTime = (double)(fTime - mLastElapsedAbsoluteTime);
			mLastElapsedAbsoluteTime = fTime;
			return fElapsedAbsoluteTime;
		}

		// Return the current time
		double GetAppTime()
		{
			return GetTime() - mBaseTime;
		}

		// Reset the timer
		double Reset()
		{
			double fTime = GetTime();

			mBaseTime = fTime;
			mLastElapsedTime = fTime;
			mStopTime = 0;
			mbTimerStopped = false;
			return 0.0;
		}

		// Start the timer
		void Start()
		{
			double fTime = GetAbsoluteTime();

			if (mbTimerStopped)
				mBaseTime += fTime - mStopTime;
			mStopTime = 0.0;
			mLastElapsedTime = fTime;
			mbTimerStopped = false;
		}

		// Stop the timer
		void Stop()
		{
			double fTime = GetTime();

			if (!mbTimerStopped)
			{
				mStopTime = fTime;
				mLastElapsedTime = fTime;
				mbTimerStopped = true;
			}
		}

		// Advance the timer by 1/10th second
		void SingleStep(double fTimeAdvance)
		{
			mStopTime += fTimeAdvance;
		}

		void MarkFrame()
		{
			mNumFrames++;
		}

		wchar_t* GetFrameRate()
		{
			double fTime = GetAbsoluteTime();

			// Only re-compute the FPS (frames per second) once per second
			if (fTime - mLastFPSTime > 1.0)
			{
				double fFPS = mNumFrames / (fTime - mLastFPSTime);
				mLastFPSTime = fTime;
				mNumFrames = 0L;
				swprintf_s(mFrameRate, L"%0.02f fps", (float)fFPS);
			}
			return mFrameRate;
		}
	};
}