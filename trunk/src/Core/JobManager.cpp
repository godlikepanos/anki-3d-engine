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
		job(id, jobParams);

		// Nullify
		{
			boost::mutex::scoped_lock lock(mutex);
			job = NULL;
		}

		barrier->wait();
	}
}


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
JobManager::JobManager(uint threadsNum):
	barrier(threadsNum + 1)
{
	for(uint i = 0; i < threadsNum; i++)
	{
		workers.push_back(new WorkerThread(i, &barrier));
	}
}



