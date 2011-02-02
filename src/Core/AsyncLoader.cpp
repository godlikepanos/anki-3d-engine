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
	File f = {filename, buff, size};
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
		File f;

		// Wait for something
		{
			boost::mutex::scoped_lock lock(mutexIn);
			while(in.empty())
			{
				std::cout << "waiting..." << std::endl;
				condVar.wait(lock);
			}

			f = in.front();
			in.pop_front();
		}

		// Load the file
		std::cout << "loading " << f.filename << "..." << std::endl;

		std::ifstream is;
		is.open(f.filename.c_str(), std::ios::binary);

		if(!is.good())
		{
			std::cerr << "error opening" << f.filename << std::endl;
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
			std::cout << "allocating " << f.size << " " << f.data << std::endl;
		}
		else if(f.size != size)
		{
			std::cerr << "error: size mismatch " << f.filename << std::endl;
			is.close();
			continue;
		}

		is.read((char*)f.data, f.size);

		if(!is.good())
		{
			std::cerr << "error reading" << f.filename << std::endl;
			is.close();
			continue;
		}

		is.close();
		std::cout << "" << f.filename << " loaded" << std::endl;

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

	File f = out.back();
	out.pop_back();
	lock.unlock();

	filename = f.filename;
	buff = f.data;
	size = f.size;
	return true;
}

