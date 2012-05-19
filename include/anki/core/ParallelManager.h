#ifndef ANKI_CORE_PARALLEL_MANAGER_H
#define ANKI_CORE_PARALLEL_MANAGER_H

#include "anki/core/ParallelJob.h"
#include <boost/thread.hpp>
#include <boost/ptr_container/ptr_vector.hpp>


namespace anki {


/// The job manager
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
	boost::scoped_ptr<boost::barrier> barrier; ///< Synchronization barrier
};


} // end namespace


#endif
