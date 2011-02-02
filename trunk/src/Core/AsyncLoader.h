#ifndef ASYNC_LOADER_H
#define ASYNC_LOADER_H

#include <queue>
#include <string>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>


///
class AsyncLoader
{
	public:
		void loadThis(const char* filename);

	private:
		std::queue<std::string> in;
		std::queue<std::string> out;
		boost::mutex mutex;
		boost::thread thread;
};


#endif
