#ifndef PARALLEL_JOB_H
#define PARALLEL_JOB_H

#include <boost/thread.hpp>


namespace parallel {


class Manager;
class Job;


/// The base class of the parameters the we pass in the job
struct JobParameters
{};


/// The callback that we feed to the job
typedef void (*JobCallback)(JobParameters&, const Job&);


/// The thread that executes a JobCallback
class Job
{
	public:
		/// Constructor
		Job(int id, const Manager& manager, boost::barrier& barrier);

		/// @name Accessors
		/// @{
		uint getId() const
		{
			return id;
		}

		const Manager& getManager() const
		{
			return manager;
		}
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
		const Manager& manager;

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
