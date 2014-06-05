// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/AsyncOperator.h"
#include "anki/core/Logger.h"
#include "anki/util/HighRezTimer.h"


namespace anki {
/*

//==============================================================================
void AsyncOperator::start()
{
	ANKI_LOGI("Starting async operator thread...");
	thread = boost::thread(&AsyncOperator::workingFunc, this);
}


//==============================================================================
void AsyncOperator::putBack(Request* newReq)
{
	ANKI_LOGI("New request (" << newReq->getInfo() << ")");
	boost::mutex::scoped_lock lock(mutexReq);
	requests.push_back(newReq);
	lock.unlock();

	condVar.notify_one();
}


//==============================================================================
void AsyncOperator::workingFunc()
{
	while(1)
	{
		Request* req;

		// Wait for something
		{
			boost::mutex::scoped_lock lock(mutexReq);
			while(requests.empty())
			{
				ANKI_LOGI("Waiting...");
				condVar.wait(lock);
			}

			req = requests.front();
			requests.pop_front();
		}

		if(req == NULL)
		{
			return;
		}

		// Exec the loader
		bool ok = true;
		try
		{
			req->exec();
			ANKI_LOGI("Request served (" << req->getInfo() << ")");
		}
		catch(const std::exception& e)
		{
			ANKI_LOGE("Request failed (" << req->getInfo() << "): " <<
				e.what());
			ok = false;
		}

		req->ok = ok;

		// Put back the response
		{
			boost::mutex::scoped_lock lock(mutexRes);
			responses.push_back(req);
		}
	} // end thread loop
}


//==============================================================================
uint AsyncOperator::execPostLoad(float maxTime)
{
	uint count = 0;
	HighRezTimer t;
	t.start();

	while(true)
	{
		boost::mutex::scoped_lock lock(mutexRes);
		if(responses.empty())
		{
			break;
		}

		Request* resp = responses.front();
		responses.pop_front();
		lock.unlock();

		resp->postExec(*this);
		delete resp;
		++count;

		// Leave if you passed the max time
		if(t.getElapsedTime() >= maxTime)
		{
			break;
		}
	}

	return count;
}
*/

} // end namespace
