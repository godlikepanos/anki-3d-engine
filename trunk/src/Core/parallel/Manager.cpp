#include "Manager.h"


namespace parallel {


//==============================================================================
// init                                                                        =
//==============================================================================
void Manager::init(uint threadsNum)
{
	barrier.reset(new boost::barrier(threadsNum + 1));

	for(uint i = 0; i < threadsNum; i++)
	{
		jobs.push_back(new Job(i, *this, *barrier.get()));
	}
}


} // end namespace
