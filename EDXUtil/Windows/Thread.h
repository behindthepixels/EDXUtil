#pragma once

#include "../EDXPrerequisites.h"
#include "Base.h"
#include <atomic>

namespace EDX
{
	class EDXThread
	{
	protected:
		HANDLE	mhThreadHandle;
		DWORD	mdwThreadID;
		bool	mbRunning;

		HANDLE	mStopEvent;

	public:
		EDXThread();

		virtual ~EDXThread()
		{
			if (mbRunning)
			{
				CloseHandle(mhThreadHandle);
				mbRunning = false;
			}

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
		static DWORD WINAPI WorkThreadProc(LPVOID lpParam);
	};

	class EDXLock
	{
	public:
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

	inline int DetectCPUCount()
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return static_cast<int>(info.dwNumberOfProcessors);
	}
}