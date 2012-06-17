#ifndef ANKI_UTIL_BARRIER_H
#define ANKI_UTIL_BARRIER_H

#include "anki/util/Assert.h"
#include <thread>
#include <condition_variable>
#include <mutex>

namespace anki {

/// A barrier for thread synchronization. It works just like boost::barrier
class Barrier
{
public:
	Barrier(uint32_t count_)
		: threshold(count_), count(count_), generation(0)
	{
		ANKI_ASSERT(count_ != 0);
	}

	bool wait()
	{
		std::unique_lock<std::mutex> lock(mtx);
		uint32_t gen = generation;

		if(--count == 0)
		{
			generation++;
			count = threshold;
			cond.notify_all();
			return true;
		}

		while(gen == generation)
		{
			cond.wait(lock);
		}
		return false;
	}

private:
	std::mutex mtx;
	std::condition_variable cond;
	uint32_t threshold;
	uint32_t count;
	uint32_t generation;
};

} // end namespace anki

#endif
