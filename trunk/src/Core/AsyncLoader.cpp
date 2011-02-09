#include "AsyncLoader.h"
#include "Logger.h"


//======================================================================================================================
// start                                                                                                               =
//======================================================================================================================
void AsyncLoader::start()
{
	INFO("Starting async loader thread...");
	thread = boost::thread(&AsyncLoader::workingFunc, this);
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void AsyncLoader::load(const char* filename, void (*loadCallback)(const char*, void*), void* storage)
{
	INFO("New load request for \"" << filename << "\"");
	boost::mutex::scoped_lock lock(mutexReq);
	Request f = {filename, loadCallback, storage};
	requests.push_back(f);
	lock.unlock();

	condVar.notify_one();
}


//======================================================================================================================
// workingFunc                                                                                                         =
//======================================================================================================================
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
				INFO("Waiting...");
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
		}
		catch(std::exception& e)
		{
			ERROR("Loading \"" << req.filename << "\" failed: " << e.what());
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


//======================================================================================================================
// pollForFinished                                                                                                     =
//======================================================================================================================
bool AsyncLoader::pollForFinished(std::string& filename, void* buff, bool& ok)
{
	boost::mutex::scoped_lock lock(mutexResp);
	if(responses.empty())
	{
		return false;
	}

	Response resp = responses.back();
	responses.pop_back();
	lock.unlock();

	filename = resp.filename;
	buff = resp.storage;
	ok = resp.ok;
	return true;
}

