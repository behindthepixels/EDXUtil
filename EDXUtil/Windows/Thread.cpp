
#include "Thread.h"

namespace EDX
{
	EDXThread::EDXThread()
		: mhThreadHandle(NULL)
		, mdwThreadID(0)
		, mbRunning(false)
	{
		mStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	bool EDXThread::Launch()
	{
		if (mbRunning)
			return true;

		DWORD dwThreadId = 0;

		ResetEvent(mStopEvent);

		mhThreadHandle = CreateThread(
			NULL,
			0,
			reinterpret_cast<LPTHREAD_START_ROUTINE>(&WorkThreadProc),
			this,
			0,
			&dwThreadId);
		if (mhThreadHandle != NULL)
		{
			mdwThreadID = dwThreadId;
			mbRunning = true;
		}

		return mbRunning;
	}

	DWORD WINAPI EDXThread::WorkThreadProc(LPVOID lpParam)
	{
		EDXThread* pThread = (EDXThread*)(lpParam);

		while (WaitForSingleObject(pThread->mStopEvent, 0) == WAIT_TIMEOUT)
		{
			pThread->WorkLoop();
		}

		return 0;
	}
}