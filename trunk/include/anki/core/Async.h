// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_CORE_ASYNC_H
#define ANKI_CORE_ASYNC_H

#include "anki/util/StdTypes.h"
#include "anki/util/Assert.h"
#include <list>
#include <thread>
#include <condition_variable>
#include <mutex>

namespace anki {

/// An asynchronous job
struct AsyncJob
{
	Bool garbageCollect = false;

	virtual ~AsyncJob()
	{}

	/// Call this from the thread
	virtual void operator()() = 0;

	/// Call this from the main thread after operator() is done
	virtual void post() = 0;
};

/// Asynchronous job executor
class Async
{
public:
	Async()
	{}

	/// Do nothing
	~Async();

	void start();
	void stop();
	
	void assignNewJob(AsyncJob* job)
	{
		ANKI_ASSERT(job != nullptr);
		assignNewJobInternal(job);
	}

	/// Call post for the finished jobs
	/// @param maxTime Try not spending much time on that
	void cleanupFinishedJobs(F32 maxTime);

private:
	std::list<AsyncJob*> pendingJobs;
	std::list<AsyncJob*> finishedJobs;
	std::mutex pendingJobsMtx; ///< Protect jobs
	std::mutex finishedJobsMtx; ///< Protect jobs
	std::thread thread;
	std::condition_variable condVar;
	Bool started = false;

	/// The thread function. It waits for some jobs to do
	void workingFunc();

	void assignNewJobInternal(AsyncJob* job);
};

} // end namespace anki

#endif
