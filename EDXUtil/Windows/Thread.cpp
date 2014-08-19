
#include "Thread.h"

namespace EDX
{
	EDXThread::EDXThread()
		: mhThreadHandle(NULL)
		, mThreadID(0)
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
			reinterpret_cast<LPTHREAD_START_ROUTINE>(&WorkerThreadProc),
			this,
			0,
			&dwThreadId);
		if (mhThreadHandle != NULL)
		{
			mThreadID = dwThreadId;
			mbRunning = true;
		}

		return mbRunning;
	}

	DWORD WINAPI EDXThread::ThreadProc(LPVOID lpParam)
	{
		EDXThread* pThread = (EDXThread*)(lpParam);

		while (WaitForSingleObject(pThread->mStopEvent, 0) == WAIT_TIMEOUT)
		{
			pThread->WorkLoop();
		}

		return 0;
	}

	DWORD WINAPI EDXThread::WorkerThreadProc(LPVOID lpParam)
	{
		EDXThread* pThread = (EDXThread*)(lpParam);

		while (WaitForSingleObject(pThread->mStopEvent, 0) == WAIT_TIMEOUT)
		{
			pThread->mpScheduler->mTaskLock.Lock();

			while (pThread->mpScheduler->mTiles.empty())
			{
				pThread->mpScheduler->mTaskCond.Wait(pThread->mpScheduler->mTaskLock);
			}

			Task& task = pThread->mpScheduler->mTiles.front();
			pThread->mpScheduler->mTiles.pop_front();

			pThread->mpScheduler->mTaskLock.Unlock();

			task.Run(task.pRunArgs, pThread->mThreadID);
		}

		return 0;
	}
}