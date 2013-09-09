#include "anki/core/WorkingThread.h"
#include "anki/core/Logger.h"

namespace anki {

#if 0

//==============================================================================
WorkingThread::WorkingThread(
	U32 id, 
	const std::shared_ptr<WorkingThreadJob>& job)
{}

//==============================================================================
void WorkingThread::start()
{
	ANKI_LOGI("Starting working thread %d", id);
	thread = std::thread(&WorkingThread::workingFunc, this);
}

//==============================================================================
void WorkingThread::workingFunc()
{
	while(1)
	{
		// Wait for something
		{
			std::unique_lock<std::mutex> lock(mutex);
			while(jobDone == true)
			{
				condVar.wait(lock);
			}
		}

		// Exec
		(*job)(id);

		// Nullify
		{
			std::lock_guard<std::mutex> lock(mutex);
			jobDone = true;
		}

		barrier->wait();
	}
}
#endif

} // end namespace anki
