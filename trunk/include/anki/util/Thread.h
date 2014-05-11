#ifndef ANKI_UTIL_THREAD_H
#define ANKI_UTIL_THREAD_H

#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"
#include "anki/util/NonCopyable.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

#define ANKI_DISABLE_THREADPOOL_THREADING 0

namespace anki {

// Forward
class Threadpool;

/// @addtogroup util_thread
/// @{

typedef U32 ThreadId;

/// Set the name of the current thead
void setCurrentThreadName(const char* name);

/// Spin lock. Good if the critical section will be executed in a short period
/// of time
class SpinLock
{
public:
	/// Lock 
	void lock()
	{
		while(m_lock.test_and_set(std::memory_order_acquire))
		{}
	}

	/// Unlock
	void unlock()
	{
		m_lock.clear(std::memory_order_release);
	}

private:
	std::atomic_flag m_lock = ATOMIC_FLAG_INIT;	
};

/// A barrier for thread synchronization. It works just like boost::barrier
class Barrier: public NonCopyable
{
public:
	Barrier(U32 count)
		: m_threshold(count), m_count(count), m_generation(0)
	{
		ANKI_ASSERT(count != 0);
	}

	Bool wait()
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		U32 gen = m_generation;

		if(--m_count == 0)
		{
			m_generation++;
			m_count = m_threshold;
			m_cond.notify_all();
			return true;
		}

		while(gen == m_generation)
		{
			m_cond.wait(lock);
		}
		return false;
	}

private:
	std::mutex m_mtx;
	std::condition_variable m_cond;
	U32 m_threshold;
	U32 m_count;
	U32 m_generation;
};

/// A task assignment to threads
class ThreadTask
{
public:
	virtual ~ThreadTask()
	{}

	virtual void operator()(ThreadId threadId) = 0;
};

/// This is a thread with 2 sync points
class DualSyncThread
{
public:
	DualSyncThread(ThreadId threadId)
		:	m_id(threadId),
			m_barriers{{{2}, {2}}},
			m_task(nullptr)
	{}

	/// The thread does not own the task
	/// @note This operation is not thread safe. Call it between syncs
	/// @note This class will not own the task
	void setTask(ThreadTask* task)
	{
		m_task = task;
	}

	/// Start the thread
	void start()
	{
		m_thread = std::thread(&DualSyncThread::workingFunc, this);
	}

	/// Sync with one of the 2 sync points
	void sync(U syncPoint)
	{
		m_barriers[syncPoint].wait();
	}

private:
	ThreadId m_id;
	std::thread m_thread; ///< Runs the workingFunc
	Array<Barrier, 2> m_barriers;
	ThreadTask* m_task; ///< Its nullptr if there are no pending task

	/// Thread loop
	void workingFunc();
};

/// A task assignment for a ThreadpoolThread
class ThreadpoolTask
{
public:
	virtual ~ThreadpoolTask()
	{}

	virtual void operator()(ThreadId threadId, U threadsCount) = 0;

	/// Chose a starting and end index
	void choseStartEnd(ThreadId threadId, PtrSize threadsCount, 
		PtrSize elementsCount, PtrSize& start, PtrSize& end)
	{
		start = threadId * (elementsCount / threadsCount);
		end = (threadId == threadsCount - 1)
			? elementsCount
			: start + elementsCount / threadsCount;
	}
};

/// A dummy thread pool task
class DummyThreadpoolTask: public ThreadpoolTask
{
public:
	void operator()(ThreadId threadId, U threadsCount)
	{
		(void)threadId;
		(void)threadsCount;
	}
};

/// The thread that executes a ThreadpoolTask
class ThreadpoolThread
{
public:
	/// Constructor
	ThreadpoolThread(ThreadId id, Barrier* barrier, Threadpool* threadpool);

	/// Assign new task to the thread
	/// @note 
	void assignNewTask(ThreadpoolTask* task);

private:
	ThreadId m_id; ///< An ID
	std::thread m_thread; ///< Runs the workingFunc
	std::mutex m_mutex; ///< Protect the task
	std::condition_variable m_condVar; ///< To wake up the thread
	Barrier* m_barrier; ///< For synchronization
	ThreadpoolTask* m_task; ///< Its NULL if there are no pending task
	Threadpool* m_threadpool;

	/// Start thread
	void start()
	{
		m_thread = std::thread(&ThreadpoolThread::workingFunc, this);
	}

	/// Thread loop
	void workingFunc();
};

/// Parallel task dispatcher.You feed it with tasks and sends them for
/// execution in parallel and then waits for all to finish
class Threadpool: public NonCopyable
{
public:
	static constexpr U MAX_THREADS = 32; ///< An absolute limit

	/// Default constructor
	Threadpool()
	{}

	/// Constructor #2
	Threadpool(U threadsNum)
	{
		init(threadsNum);
	}

	~Threadpool();

	/// Init the manager
	void init(U threadsNum);

	/// Assign a task to a working thread
	void assignNewTask(U taskId, ThreadpoolTask* task)
	{
		ANKI_ASSERT(taskId < getThreadsCount());
		m_threads[taskId]->assignNewTask(task);
	}

	/// Wait for all tasks to finish
	void waitForAllThreadsToFinish()
	{
#if !ANKI_DISABLE_THREADPOOL_THREADING
		m_barrier->wait();
#endif
	}

	U getThreadsCount() const
	{
		return m_threadsCount;
	}

private:
	ThreadpoolThread** m_threads = nullptr; ///< Worker threads array
	U8 m_threadsCount = 0;
	std::unique_ptr<Barrier> m_barrier; ///< Synchronization barrier
};

/// @}

} // end namespace anki

#endif

