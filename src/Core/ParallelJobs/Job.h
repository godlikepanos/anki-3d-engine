#ifndef PARALLEL_JOBS_JOB_H
#define PARALLEL_JOBS_JOB_H

#include <boost/thread.hpp>
#include "Util/Accessors.h"


namespace ParallelJobs {


class Manager;
class Job;


/// The base class of the parameters the we pass in the job
struct JobParameters
{
	const Job* job;
};


/// The callback that we feed to the job
typedef void (*JobCallback)(JobParameters&);


/// The thread that executes a JobCallback
class Job
{
	public:
		/// Constructor
		Job(int id, const Manager& manager, boost::barrier& barrier);

		/// @name Accessors
		/// @{
		GETTER_R(uint, id, getId)
		GETTER_R(Manager, manager, getManager)
		/// @}

		/// Assign new job to the thread
		void assignNewJob(JobCallback callback, JobParameters& jobParams);

	private:
		uint id; ///< An ID
		boost::thread thread; ///< Runs the workingFunc
		boost::mutex mutex; ///< Protect the Job::job
		boost::condition_variable condVar; ///< To wake up the thread
		boost::barrier& barrier; ///< For synchronization
		JobCallback callback; ///< Its NULL if there are no pending job
		JobParameters* params;
		/// Know your father and pass him to the jobs
		const Manager& manager; ///< Know your father

		/// Start thread
		void start();

		/// Thread loop
		void workingFunc();
};


inline void Job::start()
{
	thread = boost::thread(&Job::workingFunc, this);
}


} // end namespace


#endif
