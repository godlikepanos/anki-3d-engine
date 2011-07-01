#ifndef JOB_MANAGER_H
#define JOB_MANAGER_H

#include <boost/thread.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include "Util/Accessors.h"


class JobManager;


/// The thread that executes a job
class WorkerThread
{
	public:
		typedef void (*JobCallback)(void*, const WorkerThread&);

		/// Constructor
		WorkerThread(int id, const JobManager& jobManager,
			boost::barrier* barrier);

		/// @name Accessors
		/// @{
		GETTER_R(uint, id, getId)
		const JobManager& getJobManager() const {return jobManager;}
		/// @}

		/// Assign new job to the thread
		void assignNewJob(JobCallback job, void* jobParams);

	private:
		uint id; ///< An ID
		boost::thread thread; ///< Runs the workingFunc
		boost::mutex mutex; ///< Protect the WorkerThread::job
		boost::condition_variable condVar; ///< To wake up the thread
		boost::barrier* barrier;
		JobCallback job; ///< Its NULL if there are no pending jobs
		void* jobParams;
		/// Know your father and pass him to the jobs
		const JobManager& jobManager;

		/// Start thread
		void start();

		/// Thread loop
		void workingFunc();
};


inline WorkerThread::WorkerThread(int id_, const JobManager& jobManager_,
	boost::barrier* barrier_)
:	id(id_),
	barrier(barrier_),
	job(NULL),
	jobManager(jobManager_)
{
	start();
}


inline void WorkerThread::start()
{
	thread = boost::thread(&WorkerThread::workingFunc, this);
}


/// The job manager
class JobManager
{
	public:
		/// Default constructor
		JobManager() {}

		/// Constructor #2
		JobManager(uint threadsNum) {init(threadsNum);}

		/// Init the manager
		void init(uint threadsNum);

		/// Assign a job to a working thread
		void assignNewJob(uint threadId, WorkerThread::JobCallback job,
			void* jobParams);

		/// Wait for all jobs to finish
		void waitForAllJobsToFinish();

		uint getThreadsNum() const {return workers.size();}

	private:
		boost::ptr_vector<WorkerThread> workers; ///< Worker threads
		boost::scoped_ptr<boost::barrier> barrier; ///< Synchronization barrier
};


inline void JobManager::assignNewJob(uint threadId,
	WorkerThread::JobCallback job, void* jobParams)
{
	workers[threadId].assignNewJob(job, jobParams);
}


inline void JobManager::waitForAllJobsToFinish()
{
	barrier->wait();
}


#endif
