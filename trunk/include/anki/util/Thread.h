#ifndef ANKI_UTIL_THREAD_H
#define ANKI_UTIL_THREAD_H

#include "anki/Config.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"
#include "anki/util/Vector.h"
#include <condition_variable>
#include <mutex>
#include <thread>

namespace anki {

#define ANKI_DISABLE_THREADPOOL_THREADING 0

class Threadpool;

typedef U32 ThreadId;

/// A barrier for thread synchronization. It works just like boost::barrier
class Barrier
{
public:
	Barrier(U32 count_)
		: threshold(count_), count(count_), generation(0)
	{
		ANKI_ASSERT(count_ != 0);
	}

	Bool wait()
	{
		std::unique_lock<std::mutex> lock(mtx);
		U32 gen = generation;

		if(--count == 0)
		{
			generation++;
			count = threshold;
			cond.notify_all();
			return true;
		}

		while(gen == generation)
		{
			cond.wait(lock);
		}
		return false;
	}

private:
	std::mutex mtx;
	std::condition_variable cond;
	U32 threshold;
	U32 count;
	U32 generation;
};

/// A task assignment to threads
struct ThreadTask
{
	virtual ~ThreadTask()
	{}

	virtual void operator()(ThreadId threadId) = 0;
};

/// This is a thread with 2 sync points
class DualSyncThread
{
public:
	DualSyncThread(ThreadId threadId_)
		:	id(threadId_),
			barriers{{{2}, {2}}},
			task(nullptr)
	{}

	/// The thread does not own the task
	/// @note This operation is not thread safe. Call it between syncs
	/// @note This class will not own the task_
	void setTask(ThreadTask* task_)
	{
		task = task_;
	}

	/// Start the thread
	void start()
	{
		thread = std::thread(&DualSyncThread::workingFunc, this);
	}

	/// Sync with one of the 2 sync points
	void sync(U syncPoint)
	{
		barriers[syncPoint].wait();
	}

private:
	ThreadId id;
	std::thread thread; ///< Runs the workingFunc
	Array<Barrier, 2> barriers;
	ThreadTask* task; ///< Its nullptr if there are no pending task

	/// Thread loop
	void workingFunc();
};

/// A task assignment for a ThreadpoolThread
struct ThreadpoolTask
{
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
struct DummyThreadpoolTask: ThreadpoolTask
{
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
	ThreadId id; ///< An ID
	std::thread thread; ///< Runs the workingFunc
	std::mutex mutex; ///< Protect the task
	std::condition_variable condVar; ///< To wake up the thread
	Barrier* barrier; ///< For synchronization
	ThreadpoolTask* task; ///< Its NULL if there are no pending task
	Threadpool* threadpool;

	/// Start thread
	void start()
	{
		thread = std::thread(&ThreadpoolThread::workingFunc, this);
	}

	/// Thread loop
	void workingFunc();
};

/// Parallel task dispatcher.You feed it with tasks and sends them for
/// execution in parallel and then waits for all to finish
class Threadpool
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
		threads[taskId]->assignNewTask(task);
	}

	/// Wait for all tasks to finish
	void waitForAllThreadsToFinish()
	{
#if !ANKI_DISABLE_THREADPOOL_THREADING
		barrier->wait();
#endif
	}

	U32 getThreadsCount() const
	{
		return threads.size();
	}

private:
	Vector<ThreadpoolThread*> threads; ///< Worker threads
	std::unique_ptr<Barrier> barrier; ///< Synchronization barrier
};

} // end namespace anki

#endif

