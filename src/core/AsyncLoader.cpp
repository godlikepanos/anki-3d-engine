#include "anki/core/AsyncLoader.h"
#include "anki/core/Logger.h"
#include "anki/core/App.h"

namespace anki {
/*
//==============================================================================
void AsyncLoader::start()
{
	ANKI_LOGI("Starting async loader thread...");
	thread = boost::thread(&AsyncLoader::workingFunc, this);
}

//==============================================================================
void AsyncLoader::load(const char* filename, LoadCallback loadCallback,
	void* storage)
{
	ANKI_LOGI("New load request for: " << filename);
	mutexReq.lock();
	Request f = {filename, loadCallback, storage};
	requests.push_back(f);
	mutexReq.unlock();

	condVar.notify_one();
}

//==============================================================================
void AsyncLoader::workingFunc()
{
	while(1)
	{
		Request req;

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

		// Exec the loader
		bool ok = true;
		try
		{
			req.loadCallback(req.filename.c_str(), req.storage);
			ANKI_LOGI("File \"" << req.filename << "\" loaded");
		}
		catch(std::exception& e)
		{
			ANKI_LOGE("Loading \"" << req.filename <<
				"\" failed: " << e.what());
			ok = false;
		}

		// Put back the response
		{
			boost::mutex::scoped_lock lock(mutexResp);
			Response resp = {req.filename, req.storage, ok};
			responses.push_back(resp);
		}
	} // end thread loop
}


//==============================================================================
// pollForFinished                                                             =
//==============================================================================
bool AsyncLoader::pollForFinished(std::string& filename, void* buff, bool& ok)
{
	boost::mutex::scoped_lock lock(mutexResp);
	if(responses.empty())
	{
		return false;
	}

	Response resp = responses.front();
	responses.pop_front();
	lock.unlock();

	filename = resp.filename;
	buff = resp.storage;
	ok = resp.ok;
	return true;
}
*/

} // end namespace
