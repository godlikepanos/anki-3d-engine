#ifndef ANKI_CORE_PARALLEL_MANAGER_H
#define ANKI_CORE_PARALLEL_MANAGER_H

#include "anki/util/Barrier.h"
#include "anki/util/Singleton.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include <thread>

namespace anki {

/// Debug flag to disable threadpool threading capabilities
#define ANKI_DISABLE_THREADPOOL_THREADING 0

// Forward
class ThreadPool;

/// A job assignment for a ThreadWorker
struct ThreadJob
{
	virtual ~ThreadJob()
	{}

	virtual void operator()(U threadId, U threadsCount) = 0;

	/// Chose a starting and end index
	void choseStartEnd(U threadId, U threadsCount, U64 elementsCount,
		U64& start, U64& end)
	{
		start = threadId * (elementsCount / threadsCount);
		end = (threadId == threadsCount - 1)
			? elementsCount
			: start + elementsCount / threadsCount;
	}
};

/// A dummy job
struct ThreadJobDummy: ThreadJob
{
	void operator()(U threadId, U threadsCount)
	{
		(void)threadId;
		(void)threadsCount;
	}
};

/// The thread that executes a ThreadJob
class ThreadWorker
{
public:
	/// Constructor
	ThreadWorker(U32 id, Barrier* barrier, ThreadPool* threadPool);

	/// @name Accessors
	/// @{
	U32 getId() const
	{
		return id;
	}
	/// @}

	/// Assign new job to the thread
	void assignNewJob(ThreadJob* job);

private:
	U32 id = 0; ///< An ID
	std::thread thread; ///< Runs the workingFunc
	std::mutex mutex; ///< Protect the ThreadWorker::job
	std::condition_variable condVar; ///< To wake up the thread
	Barrier* barrier = nullptr; ///< For synchronization
	ThreadJob* job = nullptr; ///< Its NULL if there are no pending job
	ThreadPool* threadPool;

	/// Start thread
	void start()
	{
		thread = std::thread(&ThreadWorker::workingFunc, this);
	}

	/// Thread loop
	void workingFunc();
};

/// Parallel job dispatcher.You feed it with jobs and sends them for
/// execution in parallel and then waits for all to finish
class ThreadPool
{
public:
	static constexpr U MAX_THREADS = 32; ///< An absolute limit

	/// Default constructor
	ThreadPool()
	{}

	/// Constructor #2
	ThreadPool(U threadsNum)
	{
		init(threadsNum);
	}

	~ThreadPool();

	/// Init the manager
	void init(U threadsNum);

	/// Assign a job to a working thread
	void assignNewJob(U jobId, ThreadJob* job)
	{
		jobs[jobId]->assignNewJob(job);
	}

	/// Wait for all jobs to finish
	void waitForAllJobsToFinish()
	{
#if !ANKI_DISABLE_THREADPOOL_THREADING
		barrier->wait();
#endif
	}

	U32 getThreadsCount() const
	{
		return jobs.size();
	}

private:
	Vector<ThreadWorker*> jobs; ///< Worker threads
	std::unique_ptr<Barrier> barrier; ///< Synchronization barrier
};

/// Singleton
typedef Singleton<ThreadPool> ThreadPoolSingleton;

} // end namespace anki

#endif
