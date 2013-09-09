#ifndef ANKI_CORE_WORKING_THREAD_H
#define ANKI_CORE_WORKING_THREAD_H

#include "anki/util/Barrier.h"
#include "anki/util/StdTypes.h"
#include <thread>
#include <memory>

namespace anki {

/// A job assignment for a WorkingThread
struct WorkingThreadJob
{
	virtual ~WorkingThreadJob()
	{}

	virtual void operator()(U32 id) = 0;
};

/// The thread that executes a WorkingThreadJob
class WorkingThread
{
public:
	/// Constructor
	WorkingThread(U32 id, const std::shared_ptr<WorkingThreadJob>& job);

	/// @name Accessors
	/// @{
	U32 getId() const
	{
		return id;
	}
	/// @}

private:
	U32 id = 0; ///< An ID just to identify the thread
	std::thread thread; ///< Runs the workingFunc
	std::mutex mutex; ///< Protect the jobDone
	std::condition_variable condVar; ///< To wake up the thread
	std::shared_ptr<WorkingThreadJob> job;
	Bool jobDone = false;

	/// Start thread
	void start();

	/// Thread loop
	void workingFunc();
};

} // end namespace anki

#endif

