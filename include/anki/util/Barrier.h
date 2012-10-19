#ifndef ANKI_UTIL_BARRIER_H
#define ANKI_UTIL_BARRIER_H

#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"
#include <thread>
#include <condition_variable>
#include <mutex>

namespace anki {

/// A barrier for thread synchronization. It works just like boost::barrier
class Barrier
{
public:
	Barrier(U32 count_)
		: threshold(count_), count(count_), generation(0)
	{
		ANKI_ASSERT(count_ != 0);
	}

	Bool wait()
	{
		std::unique_lock<std::mutex> lock(mtx);
		U32 gen = generation;

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
	U32 threshold;
	U32 count;
	U32 generation;
};

} // end namespace anki

#endif
