
#include "Thread.h"
#include "../Memory/Memory.h"

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
			reinterpret_cast<LPTHREAD_START_ROUTINE>(&ThreadProc),
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

	void WorkerThread::WorkLoop()
	{
		mpScheduler->mTaskLock.Lock();

		while (mpScheduler->mTasks.empty())
		{
			mpScheduler->mTaskCond.Wait(mpScheduler->mTaskLock);
		}

		Task& task = mpScheduler->mTasks.front();
		mpScheduler->mTasks.pop_front();

		mpScheduler->mTaskLock.Unlock();

		task.Run(task.pRunArgs, mId);
	}

	void ThreadScheduler::InitTAndLaunchThreads()
	{
		mNumThreads = DetectCPUCount();

		mThreads.resize(mNumThreads);
		for (auto i = 0; i < mNumThreads; i++)
		{
			mThreads[i].Init(this, i);
			mThreads[i].Launch();
		}
	}
}