#include "AsyncLoader.h"
#include "Logger.h"
#include "App.h"


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
void AsyncLoader::load(const char* filename, LoadCallback loadCallback, void* storage)
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
	glContext = SDL_GL_CreateContext(AppSingleton::getInstance().getWindowId());
	if(SDL_GL_MakeCurrent(AppSingleton::getInstance().getWindowId(), glContext) != 0)
	{
		throw EXCEPTION("Cannot select GL context");
	}


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
			INFO("File \"" << req.filename << "\" loaded");
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

	Response resp = responses.front();
	responses.pop_front();
	lock.unlock();

	filename = resp.filename;
	buff = resp.storage;
	ok = resp.ok;
	return true;
}

