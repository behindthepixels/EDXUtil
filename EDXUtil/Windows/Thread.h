#pragma once

#include "../EDXPrerequisites.h"
#include "Base.h"
#include <deque>
#include <atomic>

namespace EDX
{
	class EDXThread
	{
	protected:
		HANDLE	mhThreadHandle;
		DWORD	mThreadID;
		bool	mbRunning;

		HANDLE	mStopEvent;


	public:
		EDXThread();

		EDXThread(EDXThread&& other)
		{
			operator=(std::move(other));
		}
		EDXThread& operator = (EDXThread&& rhs)
		{
			mhThreadHandle = rhs.mhThreadHandle;
			mThreadID = rhs.mThreadID;
			mbRunning = rhs.mbRunning;
			mStopEvent = rhs.mStopEvent;

			rhs.mhThreadHandle = NULL;
			rhs.mStopEvent = NULL;

			return *this;
		}

		virtual ~EDXThread()
		{
			if (mbRunning)
			{
				CloseHandle(mhThreadHandle);
				mbRunning = false;
			}

			if (mStopEvent)
				CloseHandle(mStopEvent);
		}

		virtual void WorkLoop() = 0;
		bool Launch();

		inline void StopThread()
		{
			if (mbRunning)
			{
				int iErr = SetEvent(mStopEvent);
				if (iErr == 0)
					DWORD dw = GetLastError();

				WaitForSingleObject(mhThreadHandle, INFINITE);
				CloseHandle(mhThreadHandle);
				mbRunning = false;
			}
		}

		inline void SelfTermnate()
		{
			SetEvent(mStopEvent);
			CloseHandle(mhThreadHandle);
			mbRunning = false;
		}
		inline bool IsRunning() { return mbRunning; }
		inline HANDLE GetThreadHandle() { return mhThreadHandle; }

	private:
		static DWORD WINAPI ThreadProc(LPVOID lpParam);
	};

	class ThreadScheduler;
	class WorkerThread : public EDXThread
	{
	private:
		ThreadScheduler* mpScheduler;
		uint mId;

	public:
		WorkerThread()
		{
		}

		void Init(ThreadScheduler* pSche, const uint id)
		{
			mpScheduler = pSche;
			mId = id;
		}

		WorkerThread(WorkerThread&& other)
			: EDXThread(std::move(other))
		{
			mpScheduler = other.mpScheduler;
			mId = other.mId;
			other.mpScheduler = nullptr;
			other.mId = 0;
		}

		void WorkLoop();
	};

	class EDXConditionVar;
	class EDXLock
	{
	public:
		friend class EDXConditionVar;
		EDXLock()
		{
			InitializeCriticalSection(&mCriticalSection);
		}

		virtual ~EDXLock()
		{
			DeleteCriticalSection(&mCriticalSection);
		}

	public:
		void Lock()
		{
			EnterCriticalSection(&mCriticalSection);
		}
		void Unlock()
		{
			LeaveCriticalSection(&mCriticalSection);
		}

	protected:
		CRITICAL_SECTION mCriticalSection;
	};

	class EDXLockApply
	{
	public:
		EDXLockApply(EDXLock& lock)
			: mLock(lock)
		{
			mLock.Lock();
		}
		virtual ~EDXLockApply()
		{
			mLock.Unlock();
		}

	public:
		EDXLockApply();
		EDXLockApply& operator = (const EDXLockApply&) { return *this; }

	protected:
		EDXLock& mLock;
	};

	class EDXConditionVar
	{
	public:
		CONDITION_VARIABLE mCond;

	public:
		EDXConditionVar()
		{
			InitializeConditionVariable(&mCond);
		}

		void Wait(EDXLock& lock)
		{
			SleepConditionVariableCS(&mCond, &lock.mCriticalSection, INFINITE);
		}
		void Signal()
		{
			WakeConditionVariable(&mCond);
		}
		void Broadcast()
		{
			WakeAllConditionVariable(&mCond);
		}
	};


	class Task
	{
	public:
		typedef void(*TaskFunc)(void* pArgs, int idx);

		Task(TaskFunc run, void* pArgs)
			: Run(run)
			, pRunArgs(pArgs)
		{
		}

		TaskFunc Run;
		void* pRunArgs;
	};

	class ThreadScheduler
	{
	public:
		std::deque<Task> mTasks;
		vector<WorkerThread> mThreads;

		int mNumThreads;
		EDXLock mTaskLock;
		EDXConditionVar mTaskCond;

		void InitTAndLaunchThreads();
		int GetThreadCount() const
		{
			return mNumThreads;
		}

		void AddTasks(const Task& task)
		{
			mTaskLock.Lock();
			mTasks.push_back(task);
			mTaskLock.Unlock();

			mTaskCond.Broadcast();
		}
	};

	inline int DetectCPUCount()
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return static_cast<int>(info.dwNumberOfProcessors);
	}
}