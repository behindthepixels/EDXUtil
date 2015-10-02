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
		bool mWaiting;

	public:
		WorkerThread()
		{
		}

		void Init(ThreadScheduler* pSche, const uint id)
		{
			mpScheduler = pSche;
			mId = id;
			mWaiting = true;
		}

		WorkerThread(WorkerThread&& other)
			: EDXThread(std::move(other))
		{
			mpScheduler = other.mpScheduler;
			mId = other.mId;
			other.mpScheduler = nullptr;
			other.mId = 0;
		}

		void SetAffinity(size_t affinity)
		{
			int groups = GetActiveProcessorGroupCount();
			int totalProcessors = 0, group = 0, number = 0;
			for (int i = 0; i < groups; i++)
			{
				int processors = GetActiveProcessorCount(i);
				if (totalProcessors + processors > affinity)
				{
					group = i;
					number = (int)affinity - totalProcessors;
					break;
				}
				totalProcessors += processors;
			}

			GROUP_AFFINITY groupAffinity;
			groupAffinity.Group = (WORD)group;
			groupAffinity.Mask = (KAFFINITY)(uint64(1) << number);
			groupAffinity.Reserved[0] = 0;
			groupAffinity.Reserved[1] = 0;
			groupAffinity.Reserved[2] = 0;
			if (!SetThreadGroupAffinity(mhThreadHandle, &groupAffinity, NULL))
				throw std::runtime_error("cannot set thread group affinity");

			PROCESSOR_NUMBER processorNumber;
			processorNumber.Group = group;
			processorNumber.Number = number;
			processorNumber.Reserved = 0;
			if (!SetThreadIdealProcessorEx(mhThreadHandle, &processorNumber, NULL))
				throw std::runtime_error("cannot set thread ideal processor");
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
			// Spin first before entering critical section, causing ring-0 transition and context switch.
			if (TryEnterCriticalSection(&mCriticalSection) == 0)
			{
				EnterCriticalSection(&mCriticalSection);
			}
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
		bool mTerminate;
		EDXLock mTaskLock;
		EDXLock mSystemLock;
		EDXLock mFinishedLock;
		EDXConditionVar mTaskCond;
		EDXConditionVar mFinishedCond;
		std::atomic_int mActiveTasks;


	private:
		ThreadScheduler()
			: mNumThreads(0)
			, mTerminate(true)
		{
			mActiveTasks = 0;
		}

		static ThreadScheduler* mpInstance;

	public:
		static ThreadScheduler* Instance()
		{
			if (!mpInstance)
				mpInstance = new ThreadScheduler;

			return mpInstance;
		}

		static void DeleteInstance()
		{
			if (mpInstance)
			{
				mpInstance->ReleaseAndStopThreads();

				delete mpInstance;
				mpInstance = nullptr;
			}
		}

		void InitAndLaunchThreads();
		void ReleaseAndStopThreads();
		int GetThreadCount() const
		{
			return mNumThreads;
		}

		void AddTasks(const Task& task)
		{
			mTaskLock.Lock();
			mTasks.push_back(task);
			mTaskLock.Unlock();

			mActiveTasks++;
			mTaskCond.Broadcast();
		}

		void JoinAllTasks()
		{
			mFinishedLock.Lock();
			while (mActiveTasks)
			{
				mFinishedCond.Wait(mFinishedLock);
			}
			mFinishedLock.Unlock();
		}
	};

	inline int DetectCPUCount()
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return static_cast<int>(info.dwNumberOfProcessors);
	}
}