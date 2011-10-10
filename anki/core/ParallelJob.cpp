#include "anki/core/ParallelJob.h"
#include "anki/core/ParallelManager.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
ParallelJob::ParallelJob(int id_, const ParallelManager& manager_,
	boost::barrier& barrier_)
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
void ParallelJob::assignNewJob(ParallelJobCallback callback_, ParallelJobParameters& jobParams_)
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
void ParallelJob::workingFunc()
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
