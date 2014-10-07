
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

		while (mpScheduler->mTasks.empty() && !mpScheduler->mTerminate)
		{
			mpScheduler->mTaskCond.Wait(mpScheduler->mTaskLock);
		}
		if (mpScheduler->mTerminate)
		{
			mpScheduler->mTaskLock.Unlock();
			return;
		}

		Task& task = mpScheduler->mTasks.front();
		mpScheduler->mTasks.pop_front();

		mpScheduler->mTaskLock.Unlock();

		task.Run(task.pRunArgs, mId);
		if (--mpScheduler->mActiveTasks == 0)
		{
			mpScheduler->mFinishedCond.Signal();
		}
	}

	ThreadScheduler* ThreadScheduler::mpInstance = nullptr;

	void ThreadScheduler::InitAndLaunchThreads()
	{
		mNumThreads = DetectCPUCount();
		mTerminate = false;

		mThreads.resize(mNumThreads);
		for (auto i = 0; i < mNumThreads; i++)
		{
			mThreads[i].Init(this, i);
			mThreads[i].Launch();
		}
	}

	void ThreadScheduler::ReleaseAndStopThreads()
	{
		mTerminate = true;

		mTaskLock.Lock();
		mTaskCond.Broadcast();
		mTaskLock.Unlock();

		for (auto i = 0; i < mNumThreads; i++)
		{
			mThreads[i].StopThread();
		}
	}
}