#pragma once

#include "../Core/Types.h"
#include "../Containers/Queue.h"
#include "../Containers/String.h"
#include "Base.h"

namespace EDX
{
	inline int GetNumberOfCores()
	{
		static int32 numCores = 0;
		if (numCores == 0)
		{
			// Get the number of logical processors, including hyperthreaded ones.
			SYSTEM_INFO SI;
			GetSystemInfo(&SI);
			numCores = (int32)SI.dwNumberOfProcessors;
		}
		return numCores;
	}

	/**
	* This is the Windows version of a critical section. It uses an aggregate
	* CRITICAL_SECTION to implement its locking.
	*/
	class CriticalSection
	{
	private:
		/**
		* The windows specific critical section
		*/
		CRITICAL_SECTION mCriticalSection;

	public:
		friend class ConditionVar;

		CriticalSection(const CriticalSection&) = delete;
		CriticalSection& operator=(const CriticalSection&) = delete;

		/**
		* Constructor that initializes the aggregated critical section
		*/
		__forceinline CriticalSection()
		{
			InitializeCriticalSection(&mCriticalSection);
			SetCriticalSectionSpinCount(&mCriticalSection, 4000);
		}

		/**
		* Destructor cleaning up the critical section
		*/
		__forceinline ~CriticalSection()
		{
			DeleteCriticalSection(&mCriticalSection);
		}

		/**
		* Locks the critical section
		*/
		__forceinline void Lock()
		{
			// Spin first before entering critical section, causing ring-0 transition and context switch.
			if (TryEnterCriticalSection(&mCriticalSection) == 0)
			{
				EnterCriticalSection(&mCriticalSection);
			}
		}

		/**
		* Attempt to take a lock and returns whether or not a lock was taken.
		*
		* @return true if a lock was taken, false otherwise.
		*/
		__forceinline bool TryLock()
		{
			if (TryEnterCriticalSection(&mCriticalSection))
			{
				return true;
			};
			return false;
		}

		/**
		* Releases the lock on the critical section
		*/
		__forceinline void Unlock()
		{
			LeaveCriticalSection(&mCriticalSection);
		}
	};

	/**
	* Implements a scope lock.
	*
	* This is a utility class that handles scope level locking. It's very useful
	* to keep from causing deadlocks due to exceptions being caught and knowing
	* about the number of locks a given thread has on a resource. Example:
	*
	* <code>
	*	{
	*		// Synchronize thread access to the following data
	*		ScopeLock ScopeLock(SynchObject);
	*		// Access data that is shared among multiple threads
	*		...
	*		// When ScopeLock goes out of scope, other threads can access data
	*	}
	* </code>
	*/
	class ScopeLock
	{
	public:
		/** Default constructor (hidden on purpose). */
		ScopeLock() = delete;

		/** Copy constructor( hidden on purpose). */
		ScopeLock(const ScopeLock& InScopeLock) = delete;

		/** Assignment operator (hidden on purpose). */
		ScopeLock& operator=(ScopeLock& InScopeLock) = delete;

		/**
		* Constructor that performs a lock on the synchronization object
		*
		* @param InSynchObject The synchronization object to manage
		*/
		ScopeLock(CriticalSection* InSynchObject)
			: SynchObject(InSynchObject)
		{
			Assert(SynchObject);
			SynchObject->Lock();
		}

		/** Destructor that performs a release on the synchronization object. */
		~ScopeLock()
		{
			Assert(SynchObject);
			SynchObject->Unlock();
		}

	private:

		// Holds the synchronization object to aggregate and scope manage.
		CriticalSection* SynchObject;
	};

	class ConditionVar
	{
	public:
		CONDITION_VARIABLE mCond;

	public:
		ConditionVar()
		{
			InitializeConditionVariable(&mCond);
		}

		void Wait(CriticalSection& lock)
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

	class WinEvent
	{
	private:
		/** Holds the handle to the event. */
		HANDLE Event;

		/** Whether the signaled state of the event needs to be reset manually. */
		bool ManualReset;

		/** Counter used to generate an unique id for the events. */
		static uint32 EventUniqueId;

		/** An unique id of this event. */
		uint32 EventId;

		/** Greater than 0, if the event called wait. */
		uint32 EventStartCycles;

	public:
		/**
		* Creates the event.
		*
		* Manually reset events stay triggered until reset.
		* Named events share the same underlying event.
		*
		* @param bIsManualReset Whether the event requires manual reseting or not.
		* @return true if the event was created, false otherwise.
		*/
		bool Create(bool bIsManualReset = false)
		{
			// Create the event and default it to non-signaled
			Event = CreateEvent(nullptr, bIsManualReset, 0, nullptr);
			ManualReset = bIsManualReset;

			return Event != nullptr;
		}

		/**
		* Whether the signaled state of this event needs to be reset manually.
		*
		* @return true if the state requires manual resetting, false otherwise.
		* @see Reset
		*/
		bool IsManualReset()
		{
			return ManualReset;
		}

		/**
		* Triggers the event so any waiting threads are activated.
		*
		* @see IsManualReset, Reset
		*/
		void Trigger();

		/**
		* Resets the event to an untriggered (waitable) state.
		*
		* @see IsManualReset, Trigger
		*/
		void Reset();

		/**
		* Waits the specified amount of time for the event to be triggered.
		*
		* A wait time of MAX_uint32 is treated as infinite wait.
		*
		* @param WaitTime The time to wait (in milliseconds).
		* @param bIgnoreThreadIdleStats If true, ignores ThreadIdleStats
		* @return true if the event was triggered, false if the wait timed out.
		*/
		bool Wait(uint32 WaitTime, const bool bIgnoreThreadIdleStats = false);

		/**
		* Waits an infinite amount of time for the event to be triggered.
		*
		* @return true if the event was triggered.
		*/
		bool Wait()
		{
			return Wait(uint32(Math::EDX_INFINITY));
		}

		/** Default constructor. */
		WinEvent()
			: EventId(0)
			, EventStartCycles(0)
		{}

		/** Destructor. */
		~WinEvent()
		{}
	};


	/** Thread safe counter */
	class AtomicCounter
	{
	public:
		typedef int32 IntegerType;

		/**
		* Default constructor.
		*
		* Initializes the counter to 0.
		*/
		AtomicCounter()
		{
			Counter = 0;
		}

		/**
		* Copy Constructor.
		*
		* If the counter in the Other parameter is changing from other threads, there are no
		* guarantees as to which values you will get up to the caller to not care, synchronize
		* or other way to make those guarantees.
		*
		* @param Other The other thread safe counter to copy
		*/
		AtomicCounter(const AtomicCounter& Other)
		{
			Counter = Other.GetValue();
		}

		/**
		* Constructor, initializing counter to passed in value.
		*
		* @param Value	Value to initialize counter to
		*/
		AtomicCounter(int32 Value)
		{
			Counter = Value;
		}


		/** Hidden on purpose as usage wouldn't be thread safe. */
		void operator=(const AtomicCounter& Other) = delete;

		/**
		* Increment and return new value.
		*
		* @return the new, incremented value
		* @see Add, Decrement, Reset, Set, Subtract
		*/
		int32 Increment()
		{
			return WindowsAtomics::InterlockedIncrement(&Counter);
		}

		/**
		* Adds an amount and returns the old value.
		*
		* @param Amount Amount to increase the counter by
		* @return the old value
		* @see Decrement, Increment, Reset, Set, Subtract
		*/
		int32 Add(int32 Amount)
		{
			return WindowsAtomics::InterlockedAdd(&Counter, Amount);
		}

		/**
		* Decrement and return new value.
		*
		* @return the new, decremented value
		* @see Add, Increment, Reset, Set, Subtract
		*/
		int32 Decrement()
		{
			return WindowsAtomics::InterlockedDecrement(&Counter);
		}

		/**
		* Subtracts an amount and returns the old value.
		*
		* @param Amount Amount to decrease the counter by
		* @return the old value
		* @see Add, Decrement, Increment, Reset, Set
		*/
		int32 Subtract(int32 Amount)
		{
			return WindowsAtomics::InterlockedAdd(&Counter, -Amount);
		}

		/**
		* Sets the counter to a specific value and returns the old value.
		*
		* @param Value	Value to set the counter to
		* @return The old value
		* @see Add, Decrement, Increment, Reset, Subtract
		*/
		int32 Set(int32 Value)
		{
			return WindowsAtomics::InterlockedExchange(&Counter, Value);
		}

		/**
		* Resets the counter's value to zero.
		*
		* @return the old value.
		* @see Add, Decrement, Increment, Set, Subtract
		*/
		int32 Reset()
		{
			return WindowsAtomics::InterlockedExchange(&Counter, 0);
		}

		/**
		* Gets the current value.
		*
		* @return the current value
		*/
		int32 GetValue() const
		{
			return Counter;
		}

	private:

		/** Thread-safe counter */
		volatile int32 Counter;
	};


	/**
	* Interface for "runnable" objects.
	*
	* A runnable object is an object that is "run" on an arbitrary thread. The call usage pattern is
	* Init(), Run(), Exit(). The thread that is going to "run" this object always uses those calling
	* semantics. It does this on the thread that is created so that any thread specific uses (TLS, etc.)
	* are available in the contexts of those calls. A "runnable" does all initialization in Init().
	*
	* If initialization fails, the thread stops execution and returns an error code. If it succeeds,
	* Run() is called where the real threaded work is done. Upon completion, Exit() is called to allow
	* correct clean up.
	*/
	class Runnable
	{
	public:

		/**
		* Initializes the runnable object.
		*
		* This method is called in the context of the thread object that aggregates this, not the
		* thread that passes this runnable to a new thread.
		*
		* @return True if initialization was successful, false otherwise
		* @see Run, Stop, Exit
		*/
		virtual bool Init()
		{
			return true;
		}

		/**
		* Runs the runnable object.
		*
		* This is where all per object thread work is done. This is only called if the initialization was successful.
		*
		* @return The exit code of the runnable object
		* @see Init, Stop, Exit
		*/
		virtual uint32 Run() = 0;

		/**
		* Stops the runnable object.
		*
		* This is called if a thread is requested to terminate early.
		* @see Init, Run, Exit
		*/
		virtual void Stop() { }

		/**
		* Exits the runnable object.
		*
		* Called in the context of the aggregating thread to perform any cleanup.
		* @see Init, Run, Stop
		*/
		virtual void Exit() { }

		/**
		* Gets single thread interface pointer used for ticking this runnable when multi-threading is disabled.
		* If the interface is not implemented, this runnable will not be ticked when FPlatformProcess::SupportsMultithreading() is false.
		*
		* @return Pointer to the single thread interface or nullptr if not implemented.
		*/
		virtual class SingleThreadRunnable* GetSingleThreadInterface()
		{
			return nullptr;
		}

		/** Virtual destructor */
		virtual ~Runnable() { }
	};

	/**
	* The list of enumerated thread priorities we support
	*/
	enum EThreadPriority
	{
		TPri_Normal,
		TPri_AboveNormal,
		TPri_BelowNormal,
		TPri_Highest,
		TPri_Lowest,
		TPri_SlightlyBelowNormal,
	};

	/**
	* Interface for runnable threads.
	*
	* This interface specifies the methods used to manage a thread's life cycle.
	*/
	class RunnableThread
	{
	private:
		/** The thread handle for the thread. */
		HANDLE Thread;

		/** ID set during thread creation. */
		uint32 ThreadID;

		/** Holds the name of the thread. */
		String ThreadName;

		/** The runnable object to execute on this thread. */
		Runnable* pRunnable;

		/** Sync event to make sure that Init() has been completed before allowing the main thread to continue. */
		WinEvent ThreadInitSyncEvent;

		/** The priority to run the thread at. */
		EThreadPriority ThreadPriority;

	public:
		/** Default constructor. */
		RunnableThread()
			: Thread(NULL)
		{}

		/** Virtual destructor */
		~RunnableThread()
		{
			// Clean up our thread if it is still active
			if (Thread != NULL)
			{
				Kill(true);
			}
		}

	private:
		/**
		* Helper function to set thread names, visible by the debugger.
		* @param ThreadID		Thread ID for the thread who's name is going to be set
		* @param ThreadName	Name to set
		*/
		static void SetThreadName(uint32 ThreadID, LPCSTR ThreadName)
		{
			/**
			* Code setting the thread name for use in the debugger.
			*
			* http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
			*/
			const uint32 MS_VC_EXCEPTION = 0x406D1388;

			struct THREADNAME_INFO
			{
				uint32 dwType;		// Must be 0x1000.
				LPCSTR szName;		// Pointer to name (in user addr space).
				uint32 dwThreadID;	// Thread ID (-1=caller thread).
				uint32 dwFlags;		// Reserved for future use, must be zero.
			};

			Sleep(10);
			THREADNAME_INFO ThreadNameInfo;
			ThreadNameInfo.dwType = 0x1000;
			ThreadNameInfo.szName = ThreadName;
			ThreadNameInfo.dwThreadID = ThreadID;
			ThreadNameInfo.dwFlags = 0;

#pragma warning(push)
#pragma warning(disable: 6320 6322)
			__try
			{
				RaiseException(MS_VC_EXCEPTION, 0, sizeof(ThreadNameInfo) / sizeof(ULONG_PTR), (ULONG_PTR*)&ThreadNameInfo);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
			}
#pragma warning(pop)
		}

		/**
		* The thread entry point. Simply forwards the call on to the right
		* thread main function
		*/
		static ::DWORD __stdcall _ThreadProc(LPVOID pThis)
		{
			Assert(pThis);
			return ((RunnableThread*)pThis)->Run();
		}

		/**
		* The real thread entry point. It calls the Init/Run/Exit methods on
		* the runnable object
		*/
		uint32 Run();

	public:
		/**
		* Factory method to create a thread with the specified stack size and thread priority.
		*
		* @param InRunnable The runnable object to execute
		* @param ThreadName Name of the thread
		* @param InStackSize The size of the stack to create. 0 means use the current thread's stack size
		* @param InThreadPri Tells the thread whether it needs to adjust its priority or not. Defaults to normal priority
		* @return The newly created thread or nullptr if it failed
		*/
		static RunnableThread* Create(
			class Runnable* InRunnable,
			const TCHAR* ThreadName,
			uint32 InStackSize = 0,
			EThreadPriority InThreadPri = TPri_Normal);


		static int TranslateThreadPriority(EThreadPriority Priority)
		{
			switch (Priority)
			{
			case TPri_AboveNormal: return THREAD_PRIORITY_ABOVE_NORMAL;
			case TPri_Normal: return THREAD_PRIORITY_NORMAL;
			case TPri_BelowNormal: return THREAD_PRIORITY_BELOW_NORMAL;
			case TPri_Highest: return THREAD_PRIORITY_HIGHEST;
			case TPri_Lowest: return THREAD_PRIORITY_LOWEST;
			case TPri_SlightlyBelowNormal: return THREAD_PRIORITY_NORMAL - 1;
			default: return TPri_Normal;
			}
		}

		/**
		* Changes the thread priority of the currently running thread
		*
		* @param NewPriority The thread priority to change to
		*/
		void SetThreadPriority(EThreadPriority NewPriority)
		{
			// Don't bother calling the OS if there is no need
			if (NewPriority != ThreadPriority)
			{
				ThreadPriority = NewPriority;
				// Change the priority on the thread
				::SetThreadPriority(Thread, TranslateThreadPriority(ThreadPriority));
			}
		}

		/**
		* Tells the thread to either pause execution or resume depending on the
		* passed in value.
		*
		* @param bShouldPause Whether to pause the thread (true) or resume (false)
		*/
		void Suspend(bool bShouldPause = true)
		{
			Assert(Thread);
			if (bShouldPause == true)
			{
				SuspendThread(Thread);
			}
			else
			{
				ResumeThread(Thread);
			}
		}

		/**
		* Tells the thread to exit. If the caller needs to know when the thread has exited, it should use the bShouldWait value.
		* It's highly recommended not to kill the thread without waiting for it.
		* Having a thread forcibly destroyed can cause leaks and deadlocks.
		*
		* The kill method is calling Stop() on the runnable to kill the thread gracefully.
		*
		* @param bShouldWait	If true, the call will wait infinitely for the thread to exit.
		* @return Always true
		*/
		bool Kill(bool bShouldWait = false)
		{
			Assertf(Thread, EDX_TEXT("Did you forget to call Create()?"));
			bool bDidExitOK = true;
			// Let the runnable have a chance to stop without brute force killing
			if (pRunnable)
			{
				pRunnable->Stop();
			}
			// If waiting was specified, wait the amount of time. If that fails,
			// brute force kill that thread. Very bad as that might leak.
			if (bShouldWait == true)
			{
				// Wait indefinitely for the thread to finish.  IMPORTANT:  It's not safe to just go and
				// kill the thread with TerminateThread() as it could have a mutex lock that's shared
				// with a thread that's continuing to run, which would cause that other thread to
				// dead-lock.  (This can manifest itself in code as simple as the synchronization
				// object that is used by our logging output classes.  Trust us, we've seen it!)
				WaitForSingleObject(Thread, INFINITE);
			}
			// Now clean up the thread handle so we don't leak
			CloseHandle(Thread);
			Thread = NULL;

			return bDidExitOK;
		}

		/** Halts the caller until this thread is has completed its work. */
		void WaitForCompletion()
		{
			// Block until this thread exits
			WaitForSingleObject(Thread, INFINITE);
		}

		/**
		* Thread ID for this thread
		*
		* @return ID that was set by CreateThread
		* @see GetThreadName
		*/
		const uint32 GetThreadID() const
		{
			return ThreadID;
		}

		/**
		* Retrieves the given name of the thread
		*
		* @return Name that was set by CreateThread
		* @see GetThreadID
		*/
		const String& GetThreadName() const
		{
			return ThreadName;
		}

	protected:

		/**
		* Creates the thread with the specified stack size and thread priority.
		*
		* @param InRunnable The runnable object to execute
		* @param ThreadName Name of the thread
		* @param InStackSize The size of the stack to create. 0 means use the current thread's stack size
		* @param InThreadPri Tells the thread whether it needs to adjust its priority or not. Defaults to normal priority
		* @return True if the thread and all of its initialization was successful, false otherwise
		*/
		bool CreateInternal(Runnable* InRunnable, const TCHAR* InThreadName,
			uint32 InStackSize = 0,
			EThreadPriority InThreadPri = TPri_Normal)
		{
			Assert(InRunnable);
			pRunnable = InRunnable;

			// Create a sync event to guarantee the Init() function is called first
			ThreadInitSyncEvent.Create(true);

			// Create the new thread
			Thread = CreateThread(NULL, InStackSize, _ThreadProc, this, STACK_SIZE_PARAM_IS_A_RESERVATION, (::DWORD *)&ThreadID);

			// If it fails, clear all the vars
			if (Thread == NULL)
			{
				pRunnable = nullptr;
			}
			else
			{
				// Let the thread start up, then set the name for debug purposes.
				ThreadInitSyncEvent.Wait(INFINITE);
				ThreadName = InThreadName ? InThreadName : EDX_TEXT("Unnamed Thread");
				SetThreadName(ThreadID, TCHAR_TO_ANSI(*ThreadName));
				SetThreadPriority(InThreadPri);
			}

			// Cleanup the sync event
			ThreadInitSyncEvent.Reset();

			return Thread != NULL;
		}

	private:

		/** Used by the thread manager to tick threads in single-threaded mode */
		virtual void Tick() {}
	};

	/**
	* Interface for queued work objects.
	*
	* This interface is a type of runnable object that requires no per thread
	* initialization. It is meant to be used with pools of threads in an
	* abstract way that prevents the pool from needing to know any details
	* about the object being run. This allows queuing of disparate tasks and
	* servicing those tasks with a generic thread pool.
	*/
	class QueuedWork
	{
	public:

		/**
		* This is where the real thread work is done. All work that is done for
		* this queued object should be done from within the call to this function.
		*/
		virtual void DoThreadedWork() = 0;

		/**
		* Tells the queued work that it is being abandoned so that it can do
		* per object clean up as needed. This will only be called if it is being
		* abandoned before completion. NOTE: This requires the object to delete
		* itself using whatever heap it was allocated in.
		*/
		virtual void Abandon() = 0;

	public:

		/**
		* Virtual destructor so that child implementations are guaranteed a chance
		* to clean up any resources they allocated.
		*/
		virtual ~QueuedWork() { }
	};

	/**
	* This is the interface used for all poolable threads. The usage pattern for
	* a poolable thread is different from a regular thread and this interface
	* reflects that. Queued threads spend most of their life cycle idle, waiting
	* for work to do. When signaled they perform a job and then return themselves
	* to their owning pool via a callback and go back to an idle state.
	*/
	class QueuedThread : public Runnable
	{
	protected:
		/** The pool this thread belongs to. */
		class QueuedThreadPool* OwningThreadPool;

		/** My Thread  */
		RunnableThread* Thread;

		/**
		* The real thread entry point. It waits for work events to be queued. Once
		* an event is queued, it executes it and goes back to waiting.
		*/
		virtual uint32 Run() override;

	public:

		/** Default constructor **/
		QueuedThread()
			: OwningThreadPool(nullptr)
			, Thread(nullptr)
		{ }

		/**
		* Creates the thread with the specified stack size and creates the various
		* events to be able to communicate with it.
		*
		* @param InPool The thread pool interface used to place this thread back into the pool of available threads when its work is done
		* @param InStackSize The size of the stack to create. 0 means use the current thread's stack size
		* @param ThreadPriority priority of new thread
		* @return True if the thread and all of its initialization was successful, false otherwise
		*/
		bool Create(class QueuedThreadPool* InPool, uint32 InStackSize = 0, EThreadPriority ThreadPriority = TPri_Normal);

		/**
		* Tells the thread to exit. If the caller needs to know when the thread
		* has exited, it should use the bShouldWait value and tell it how long
		* to wait before deciding that it is deadlocked and needs to be destroyed.
		* NOTE: having a thread forcibly destroyed can cause leaks in TLS, etc.
		*
		* @return True if the thread exited graceful, false otherwise
		*/
		bool KillThread();

	};


	/**
	* Interface for queued thread pools.
	*
	* This interface is used by all queued thread pools. It used as a callback by
	* FQueuedThreads and is used to queue asynchronous work for callers.
	*/
	class QueuedThreadPool
	{
	private:
		static QueuedThreadPool* mpInstance;

		/** Default constructor. */
		QueuedThreadPool()
			: bTerminate(false)
		{
		}

	protected:
		/** The work queue to pull from. */
		Queue<QueuedWork*> QueuedWorks;

		/** The thread pool to dole work out to. */
		Array<QueuedThread*> QueuedThreads;

		/** Condition variable for adding new tasks. */
		ConditionVar TaskCondVar;

		/** Condition variable for finishing all tasks. */
		ConditionVar FinishedCondVar;

		/** The synchronization object used to protect access to the queued work. */
		CriticalSection TaskLock;
		CriticalSection FinishedLock;

		/** The atomic counter that keeps record of number of tasks. */
		AtomicCounter TaskCounter;

		/** If true, indicates the destruction process has taken place. */
		bool bTerminate;

	public:
		friend class QueuedThread;

		static QueuedThreadPool* Instance()
		{
			if (!mpInstance)
				mpInstance = new QueuedThreadPool;

			return mpInstance;
		}

		static void DeleteInstance()
		{
			if (mpInstance)
			{
				mpInstance->Destroy();

				delete mpInstance;
				mpInstance = nullptr;
			}
		}
		/**
		*	Stack size for threads created for the thread pool.
		*	Can be overridden by other projects.
		*	If 0 means to use the value passed in the Create method.
		*/
		static uint32 OverrideStackSize;

	public:

		/** Virtual destructor (cleans up the synchronization objects). */
		~QueuedThreadPool();

		bool Create(uint32 InNumQueuedThreads, uint32 StackSize = 0/*(32 * 1024)*/, EThreadPriority ThreadPriority = TPri_Normal);

		void JoinAllThreads();
		void Destroy();

		int32 GetNumQueuedJobs()
		{
			// this is a estimate of the number of queued jobs. 
			// no need for thread safe lock as the queuedWork array isn't moved around in memory so unless this class is being destroyed then we don't need to wrory about it
			return TaskCounter.GetValue();
		}

		int32 GetNumThreads()
		{
			return QueuedThreads.Size();
		}

		void AddQueuedWork(QueuedWork* InQueuedWork);
		
		QueuedWork* GetNextJob(QueuedThread* InQueuedThread);
	};
}