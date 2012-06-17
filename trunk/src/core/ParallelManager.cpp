#include "anki/core/ParallelManager.h"

namespace anki {

//==============================================================================
// ParallelJob                                                                 =
//==============================================================================

//==============================================================================
ParallelJob::ParallelJob(uint32_t id_, const ParallelManager* manager_,
	Barrier* barrier_)
	: id(id_), barrier(barrier_), callback(nullptr), manager(manager_)
{
	start();
}

//==============================================================================
void ParallelJob::assignNewJob(ParallelJobCallback callback_,
	ParallelJobParameters& jobParams_)
{
	mutex.lock();
	callback = callback_;
	params = &jobParams_;
	mutex.unlock();
	condVar.notify_one();
}

//==============================================================================
void ParallelJob::workingFunc()
{
	while(1)
	{
		// Wait for something
		{
			std::unique_lock<std::mutex> lock(mutex);
			while(callback == nullptr)
			{
				condVar.wait(lock);
			}
		}

		// Exec
		callback(*params, *this);

		// Nullify
		{
			std::lock_guard<std::mutex> lock(mutex);
			callback = nullptr;
		}

		barrier->wait();
	}
}

//==============================================================================
// ParallelManager                                                             =
//==============================================================================

//==============================================================================
void ParallelManager::init(uint threadsNum)
{
	barrier.reset(new Barrier(threadsNum + 1));

	for(uint i = 0; i < threadsNum; i++)
	{
		jobs.push_back(new ParallelJob(i, this, barrier.get()));
	}
}


} // end namespace
