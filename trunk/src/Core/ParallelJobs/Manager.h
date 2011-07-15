#ifndef PARALLEL_JOBS_MANAGER_H
#define PARALLEL_JOBS_MANAGER_H

#include "Job.h"
#include <boost/thread.hpp>
#include <boost/ptr_container/ptr_vector.hpp>


namespace ParallelJobs {


/// The job manager
class Manager
{
	public:
		/// Default constructor
		Manager() {}

		/// Constructor #2
		Manager(uint threadsNum) {init(threadsNum);}

		/// Init the manager
		void init(uint threadsNum);

		/// Assign a job to a working thread
		void assignNewJob(uint threadId, JobCallback job,
			JobParameters& jobParams);

		/// Wait for all jobs to finish
		void waitForAllJobsToFinish();

		uint getThreadsNum() const {return jobs.size();}

	private:
		boost::ptr_vector<Job> jobs; ///< Worker threads
		boost::scoped_ptr<boost::barrier> barrier; ///< Synchronization barrier
};


inline void Manager::assignNewJob(uint jobId,
	JobCallback callback, JobParameters& jobParams)
{
	jobs[jobId].assignNewJob(callback, jobParams);
}


inline void Manager::waitForAllJobsToFinish()
{
	barrier->wait();
}


} // end namespace


#endif
