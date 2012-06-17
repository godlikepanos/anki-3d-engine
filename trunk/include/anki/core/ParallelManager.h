#ifndef ANKI_CORE_PARALLEL_MANAGER_H
#define ANKI_CORE_PARALLEL_MANAGER_H

#include "anki/util/Barrier.h"
#include "anki/util/Singleton.h"
#include <thread>
#include <boost/ptr_container/ptr_vector.hpp>

namespace anki {

class ParallelManager;
class ParallelJob;

/// The base class of the parameters the we pass in the job
struct ParallelJobParameters
{};

/// The callback that we feed to the job
typedef void (*ParallelJobCallback)(ParallelJobParameters&, const ParallelJob&);

/// The thread that executes a ParallelJobCallback
class ParallelJob
{
public:
	/// Constructor
	ParallelJob(uint32_t id, const ParallelManager* manager,
		Barrier* barrier);

	/// @name Accessors
	/// @{
	uint32_t getId() const
	{
		return id;
	}

	const ParallelManager& getManager() const
	{
		return *manager;
	}
	/// @}

	/// Assign new job to the thread
	void assignNewJob(ParallelJobCallback callback,
		ParallelJobParameters& jobParams);

private:
	uint32_t id; ///< An ID
	std::thread thread; ///< Runs the workingFunc
	std::mutex mutex; ///< Protect the ParallelJob::job
	std::condition_variable condVar; ///< To wake up the thread
	Barrier* barrier; ///< For synchronization
	ParallelJobCallback callback; ///< Its NULL if there are no pending job
	ParallelJobParameters* params;
	/// Know your father and pass him to the jobs
	const ParallelManager* manager;

	/// Start thread
	void start()
	{
		thread = std::thread(&ParallelJob::workingFunc, this);
	}

	/// Thread loop
	void workingFunc();
};

/// Parallel job dispatcher.You feed it with jobs and sends them for
/// execution in parallel and then waits for all to finish
class ParallelManager
{
public:
	/// Default constructor
	ParallelManager()
	{}

	/// Constructor #2
	ParallelManager(uint threadsNum)
	{
		init(threadsNum);
	}

	/// Init the manager
	void init(uint threadsNum);

	/// Assign a job to a working thread
	void assignNewJob(uint jobId, ParallelJobCallback callback,
		ParallelJobParameters& jobParams)
	{
		jobs[jobId].assignNewJob(callback, jobParams);
	}

	/// Wait for all jobs to finish
	void waitForAllJobsToFinish()
	{
		barrier->wait();
	}

	uint getThreadsNum() const
	{
		return jobs.size();
	}

private:
	boost::ptr_vector<ParallelJob> jobs; ///< Worker threads
	std::unique_ptr<Barrier> barrier; ///< Synchronization barrier
};

/// Singleton
typedef Singleton<ParallelManager> ParallelManagerSingleton;

} // end namespace anki

#endif
