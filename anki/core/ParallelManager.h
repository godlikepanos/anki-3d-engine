#ifndef PARALLEL_MANAGER_H
#define PARALLEL_MANAGER_H

#include "anki/core/ParallelJob.h"
#include <boost/thread.hpp>
#include <boost/ptr_container/ptr_vector.hpp>


/// The job manager
class ParallelManager
{
	public:
		/// Default constructor
		ParallelManager() {}

		/// Constructor #2
		ParallelManager(uint threadsNum) {init(threadsNum);}

		/// Init the manager
		void init(uint threadsNum);

		/// Assign a job to a working thread
		void assignNewJob(uint threadId, ParallelJobCallback job,
			ParallelJobParameters& jobParams);

		/// Wait for all jobs to finish
		void waitForAllJobsToFinish();

		uint getThreadsNum() const {return jobs.size();}

	private:
		boost::ptr_vector<ParallelJob> jobs; ///< Worker threads
		boost::scoped_ptr<boost::barrier> barrier; ///< Synchronization barrier
};


inline void ParallelManager::assignNewJob(uint jobId,
	ParallelJobCallback callback, ParallelJobParameters& jobParams)
{
	jobs[jobId].assignNewJob(callback, jobParams);
}


inline void ParallelManager::waitForAllJobsToFinish()
{
	barrier->wait();
}


#endif
