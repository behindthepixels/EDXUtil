#pragma once

#include "../EDXPrerequisites.h"

#include <Windows.h>

namespace EDX
{
	class Timer
	{
	public:
		double mfLastElapsedAbsoluteTime;
		double mfBaseAbsoluteTime;

		double mfLastElapsedTime;
		double mfBaseTime;
		double mfStopTime;
		bool mbTimerStopped;

		wchar_t mstrFrameRate[16];
		dword mdwNumFrames;
		double mfLastFPSTime;

		LARGE_INTEGER mPerfFreq;

		Timer()
		{
			QueryPerformanceFrequency(&mPerfFreq);
			double fTime = GetAbsoluteTime();

			mfBaseAbsoluteTime = fTime;
			mfLastElapsedAbsoluteTime = fTime;

			mfBaseTime = fTime;
			mfStopTime = 0.0;
			mfLastElapsedTime = fTime;
			mbTimerStopped = false;

			mstrFrameRate[0] = L'\0';
			mdwNumFrames = 0;
			mfLastFPSTime = fTime;
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
			// on whether we're stopped and what command was sent
			return (mfStopTime != 0.0) ? mfStopTime : GetAbsoluteTime();
		}

		double GetElapsedTime()
		{
			double fTime = GetAbsoluteTime();

			double fElapsedAbsoluteTime = (double)(fTime - mfLastElapsedAbsoluteTime);
			mfLastElapsedAbsoluteTime = fTime;
			return fElapsedAbsoluteTime;
		}

		// Return the current time
		double GetAppTime()
		{
			return GetTime() - mfBaseTime;
		}

		// Reset the timer
		double Reset()
		{
			double fTime = GetTime();

			mfBaseTime = fTime;
			mfLastElapsedTime = fTime;
			mfStopTime = 0;
			mbTimerStopped = false;
			return 0.0;
		}

		// Start the timer
		void Start()
		{
			double fTime = GetAbsoluteTime();

			if (mbTimerStopped)
				mfBaseTime += fTime - mfStopTime;
			mfStopTime = 0.0;
			mfLastElapsedTime = fTime;
			mbTimerStopped = false;
		}

		// Stop the timer
		void Stop()
		{
			double fTime = GetTime();

			if (!mbTimerStopped)
			{
				mfStopTime = fTime;
				mfLastElapsedTime = fTime;
				mbTimerStopped = true;
			}
		}

		// Advance the timer by 1/10th second
		void SingleStep(double fTimeAdvance)
		{
			mfStopTime += fTimeAdvance;
		}

		void MarkFrame()
		{
			mdwNumFrames++;
		}

		wchar_t* GetFrameRate()
		{
			double fTime = GetAbsoluteTime();

			// Only re-compute the FPS (frames per second) once per second
			if (fTime - mfLastFPSTime > 1.0)
			{
				double fFPS = mdwNumFrames / (fTime - mfLastFPSTime);
				mfLastFPSTime = fTime;
				mdwNumFrames = 0L;
				swprintf_s(mstrFrameRate, L"%0.02f fps", (float)fFPS);
			}
			return mstrFrameRate;
		}
	};
}