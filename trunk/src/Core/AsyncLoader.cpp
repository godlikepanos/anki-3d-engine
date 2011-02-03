#include <fstream>
#include "AsyncLoader.h"
#include "Logger.h"


//======================================================================================================================
// start                                                                                                               =
//======================================================================================================================
void AsyncLoader::start()
{
	thread = boost::thread(&AsyncLoader::workingFunc, this);
}


//======================================================================================================================
// loadInPreallocatedBuff                                                                                              =
//======================================================================================================================
void AsyncLoader::loadInPreallocatedBuff(const char* filename, void* buff, size_t size)
{
	//std::cout << "pushing " << filename << "..." << std::endl;
	boost::mutex::scoped_lock lock(mutexIn);
	Request f = {filename, buff, size};
	in.push_back(f);
	lock.unlock();

	condVar.notify_one();
}


//======================================================================================================================
// loadInNewBuff                                                                                                       =
//======================================================================================================================
void AsyncLoader::loadInNewBuff(const char* filename)
{
	loadInPreallocatedBuff(filename, NULL, 0);
}


//======================================================================================================================
// workingFunc                                                                                                         =
//======================================================================================================================
void AsyncLoader::workingFunc()
{
	while(1)
	{
		Request f;

		// Wait for something
		{
			boost::mutex::scoped_lock lock(mutexIn);
			while(in.empty())
			{
				INFO("Waiting...");
				condVar.wait(lock);
			}

			f = in.front();
			in.pop_front();
		}

		// Load the file
		INFO("Loading \"" << f.filename << "\"...");

		std::ifstream is;
		is.open(f.filename.c_str(), std::ios::binary);

		if(!is.good())
		{
			ERROR("Cannot open \"" << f.filename << "\"");
			continue;
		}

		// Get size of file
		is.seekg(0, std::ios::end);
		size_t size = is.tellg();
		is.seekg(0, std::ios::beg);

		// Alloc (if needed)
		if(f.data == NULL && f.size == 0)
		{
			f.size = size;
			f.data = new char[f.size];
		}
		else if(f.size != size)
		{
			ERROR("Size mismatch \"" << f.filename << "\"");
			is.close();
			continue;
		}

		is.read((char*)f.data, f.size);

		if(!is.good())
		{
			ERROR("Cannot read \"" << f.filename << "\"");
			is.close();
			continue;
		}

		is.close();
		INFO("Request \"" << f.filename << "\"");

		// Put the data in the out
		{
			boost::mutex::scoped_lock lock(mutexOut);
			out.push_back(f);
		}
	} // end thread loop
}


//======================================================================================================================
// getLoaded                                                                                                           =
//======================================================================================================================
bool AsyncLoader::getLoaded(std::string& filename, void*& buff, size_t& size)
{
	boost::mutex::scoped_lock lock(mutexOut);
	if(out.empty())
	{
		return false;
	}

	Request f = out.back();
	out.pop_back();
	lock.unlock();

	filename = f.filename;
	buff = f.data;
	size = f.size;
	return true;
}

