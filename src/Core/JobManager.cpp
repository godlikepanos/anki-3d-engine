#include "JobManager.h"


//======================================================================================================================
// assignNewJob                                                                                                        =
//======================================================================================================================
void WorkerThread::assignNewJob(JobCallback job_, void* jobParams_)
{
	boost::mutex::scoped_lock lock(mutex);
	job = job_;
	jobParams = jobParams_;

	lock.unlock();
	condVar.notify_one();
}


//======================================================================================================================
// workingFunc                                                                                                         =
//======================================================================================================================
void WorkerThread::workingFunc()
{
	while(1)
	{
		// Wait for something
		{
			boost::mutex::scoped_lock lock(mutex);
			while(job == NULL)
			{
				condVar.wait(lock);
			}
		}

		// Exec
		job(jobParams, *this);

		// Nullify
		{
			boost::mutex::scoped_lock lock(mutex);
			job = NULL;
		}

		barrier->wait();
	}
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void JobManager::init(uint threadsNum)
{
	barrier.reset(new boost::barrier(threadsNum + 1));

	for(uint i = 0; i < threadsNum; i++)
	{
		workers.push_back(new WorkerThread(i, *this, barrier.get()));
	}
}



