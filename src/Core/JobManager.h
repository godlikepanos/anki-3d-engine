#ifndef JOB_MANAGER_H
#define JOB_MANAGER_H

#include <boost/thread.hpp>
#include <boost/ptr_container/ptr_vector.hpp>


/// @todo
class WorkerThread
{
	public:
		typedef void (*JobCallback)(uint jobId, void*);

		/// Constructor
		WorkerThread(int id, boost::barrier* barrier);

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

		/// Start thread
		void start();

		/// Thread loop
		void workingFunc();
};


inline WorkerThread::WorkerThread(int id_, boost::barrier* barrier_):
	id(id_),
	barrier(barrier_),
	job(NULL)
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
		/// Constructor
		JobManager(uint threadsNum);

		/// Assign a job to a working thread
		void assignNewJob(uint threadId, WorkerThread::JobCallback job, void* jobParams);

		/// Wait for all jobs to finish
		void waitForAllJobsToFinish();

		uint getThreadsNum() const {return workers.size();}

	private:
		boost::ptr_vector<WorkerThread> workers;
		boost::barrier barrier;
};


inline void JobManager::assignNewJob(uint threadId, WorkerThread::JobCallback job, void* jobParams)
{
	workers[threadId].assignNewJob(job, jobParams);
}


inline void JobManager::waitForAllJobsToFinish()
{
	barrier.wait();
}


#endif
