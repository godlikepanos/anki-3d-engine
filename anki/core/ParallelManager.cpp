#include "anki/core/ParallelManager.h"


//==============================================================================
// init                                                                        =
//==============================================================================
void ParallelManager::init(uint threadsNum)
{
	barrier.reset(new boost::barrier(threadsNum + 1));

	for(uint i = 0; i < threadsNum; i++)
	{
		jobs.push_back(new ParallelJob(i, *this, *barrier.get()));
	}
}
