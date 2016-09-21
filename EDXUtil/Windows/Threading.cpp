
#include "Threading.h"
#include "../Core/Memory.h"

namespace EDX
{
	bool WinEvent::Wait(uint32 WaitTime, const bool bIgnoreThreadIdleStats /*= false*/)
	{
		Assert(Event);

		return (WaitForSingleObject(Event, WaitTime) == WAIT_OBJECT_0);
	}

	void WinEvent::Trigger()
	{
		Assert(Event);
		SetEvent(Event);
	}

	void WinEvent::Reset()
	{
		Assert(Event);
		ResetEvent(Event);
	}

	RunnableThread* RunnableThread::Create(
		class Runnable* InRunnable,
		const TCHAR* ThreadName,
		uint32 InStackSize,
		EThreadPriority InThreadPri)
	{
		Assert(InRunnable);

		// Create a new thread object
		RunnableThread* NewThread = new RunnableThread();
		if (NewThread)
		{
			// Call the thread's create method
			if (NewThread->CreateInternal(InRunnable, ThreadName, InStackSize, InThreadPri) == false)
			{
				// We failed to start the thread correctly so clean up
				Memory::SafeDelete(NewThread);
			}
		}

		return NewThread;
	}

	uint32 RunnableThread::Run()
	{
		// Assume we'll fail init
		uint32 ExitCode = 1;
		Assert(pRunnable);

		// Initialize the runnable object
		if (pRunnable->Init() == true)
		{
			// Initialization has completed, release the sync event
			ThreadInitSyncEvent.Trigger();

			// Now run the task that needs to be done
			ExitCode = pRunnable->Run();
			// Allow any allocated resources to be cleaned up
			pRunnable->Exit();
		}
		else
		{
			// Initialization has failed, release the sync event
			ThreadInitSyncEvent.Trigger();
		}

		return ExitCode;
	}

	uint32 QueuedThread::Run()
	{
		while (!OwningThreadPool->bTerminate)
		{
			OwningThreadPool->TaskLock.Lock();

			while (OwningThreadPool->QueuedWorks.IsEmpty() && !OwningThreadPool->bTerminate)
			{
				OwningThreadPool->TaskCondVar.Wait(OwningThreadPool->TaskLock);
			}
			if (OwningThreadPool->bTerminate)
			{
				OwningThreadPool->TaskLock.Unlock();
				return 0;
			}

			QueuedWork* pWork;
			OwningThreadPool->QueuedWorks.Dequeue(pWork);
			
			OwningThreadPool->TaskLock.Unlock();

			// Tell the object to do the work
			pWork->DoThreadedWork();

			if (OwningThreadPool->TaskCounter.Decrement() == 0)
			{
				OwningThreadPool->FinishedCondVar.Signal();
			}

			//while (pWork)
			//{
			//	// Tell the object to do the work
			//	pWork->DoThreadedWork();

			//	// Let the object cleanup before we remove our ref to it
			//	pWork = OwningThreadPool->GetNextJob(this);

			//	if (OwningThreadPool->TaskCounter.Decrement() == 0)
			//	{
			//		check(pWork == nullptr);
			//		OwningThreadPool->FinishedCondVar.Signal();
			//	}
			//}
		}

		return 0;
	}

	bool QueuedThread::Create(class QueuedThreadPool* InPool, uint32 InStackSize, EThreadPriority ThreadPriority)
	{
		static int32 PoolThreadIndex = 0;
		const String PoolThreadName = String::Printf(EDX_TEXT("PoolThread %d"), PoolThreadIndex);
		PoolThreadIndex++;

		OwningThreadPool = InPool;
		Thread = RunnableThread::Create(this, *PoolThreadName, InStackSize, ThreadPriority);
		Assert(Thread);
		return true;
	}

	bool QueuedThread::KillThread()
	{
		bool bDidExitOK = true;

		// If waiting was specified, wait the amount of time. If that fails,
		// brute force kill that thread. Very bad as that might leak.
		Thread->WaitForCompletion();
		delete Thread;

		return bDidExitOK;
	}

	// Thread pool singleton instance
	QueuedThreadPool* QueuedThreadPool::mpInstance = nullptr;
	uint32 QueuedThreadPool::OverrideStackSize = 0;

	/** Virtual destructor (cleans up the synchronization objects). */
	QueuedThreadPool::~QueuedThreadPool()
	{
		Destroy();
	}

	bool QueuedThreadPool::Create(uint32 InNumQueuedThreads, uint32 StackSize, EThreadPriority ThreadPriority)
	{
		// Make sure we have synch objects
		bool bWasSuccessful = true;
		ScopeLock Lock(&TaskLock);

		// Presize the array so there is no extra memory allocated
		Assert(QueuedThreads.Size() == 0);
		QueuedThreads.Clear(InNumQueuedThreads);

		// Check for stack size override.
		if (OverrideStackSize > StackSize)
		{
			StackSize = OverrideStackSize;
		}

		bTerminate = false;

		// Now create each thread and add it to the array
		for (uint32 Count = 0; Count < InNumQueuedThreads && bWasSuccessful == true; Count++)
		{
			// Create a new queued thread
			QueuedThread* pThread = new QueuedThread();
			// Now create the thread and add it if ok
			if (pThread->Create(this, StackSize, ThreadPriority) == true)
			{
				QueuedThreads.Add(pThread);
			}
			else
			{
				// Failed to fully create so clean up
				bWasSuccessful = false;
				delete pThread;
			}
		}
		// Destroy any created threads if the full set was not successful
		if (bWasSuccessful == false)
		{
			Destroy();
		}

		return bWasSuccessful;
	}

	void QueuedThreadPool::JoinAllThreads()
	{
		ScopeLock Lock(&FinishedLock);

		while (TaskCounter.GetValue())
		{
			FinishedCondVar.Wait(FinishedLock);
		}
	}

	void QueuedThreadPool::Destroy()
	{
		{
			ScopeLock Lock(&TaskLock);
			bTerminate = true;

			// Clean up all queued objects
			QueuedWork* pWork = nullptr;
			while (QueuedWorks.Dequeue(pWork))
			{
				pWork->Abandon();
			}

			// Empty out the invalid pointers
			QueuedWorks.Clear();
		}

		JoinAllThreads();

		TaskLock.Lock();
		TaskCondVar.Broadcast();
		TaskLock.Unlock();

		// Delete all threads
		for (int32 Index = 0; Index < QueuedThreads.Size(); Index++)
		{
			QueuedThreads[Index]->KillThread();
			delete QueuedThreads[Index];
		}
		QueuedThreads.Clear();
	}

	void QueuedThreadPool::AddQueuedWork(QueuedWork* InQueuedWork)
	{
		if (bTerminate)
		{
			InQueuedWork->Abandon();
			return;
		}

		TaskLock.Lock();
		QueuedWorks.Enqueue(InQueuedWork);
		TaskLock.Unlock();

		TaskCounter.Increment();
		TaskCondVar.Broadcast();
	}

	QueuedWork* QueuedThreadPool::GetNextJob(QueuedThread* InQueuedThread)
	{
		Assert(InQueuedThread != nullptr);
		QueuedWork* Work = nullptr;
		// Check to see if there is any work to be done
		ScopeLock Lock(&TaskLock);

		if (bTerminate)
		{
			Assert(QueuedWorks.IsEmpty());  // we better not have anything if we are dying
		}
		if (TaskCounter.GetValue() > 0)
		{
			QueuedWorks.Dequeue(Work);
		}

		return Work;
	}
}