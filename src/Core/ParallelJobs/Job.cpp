#include "Job.h"
#include "Manager.h"


namespace ParallelJobs {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Job::Job(int id_, const Manager& manager_, boost::barrier& barrier_)
:	id(id_),
 	barrier(barrier_),
	callback(NULL),
	manager(manager_)
{
	start();
}


//==============================================================================
// assignNewJob                                                                =
//==============================================================================
void Job::assignNewJob(JobCallback callback_, JobParameters& jobParams_)
{
	boost::mutex::scoped_lock lock(mutex);
	callback = callback_;
	params = &jobParams_;

	lock.unlock();
	condVar.notify_one();
}


//==============================================================================
// workingFunc                                                                 =
//==============================================================================
void Job::workingFunc()
{
	while(1)
	{
		// Wait for something
		{
			boost::mutex::scoped_lock lock(mutex);
			while(callback == NULL)
			{
				condVar.wait(lock);
			}
		}

		// Exec
		callback(*params, *this);

		// Nullify
		{
			boost::mutex::scoped_lock lock(mutex);
			callback = NULL;
		}

		barrier.wait();
	}
}


} // end namespace
