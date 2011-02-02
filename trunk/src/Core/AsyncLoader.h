#ifndef ASYNC_LOADER_H
#define ASYNC_LOADER_H

#include <list>
#include <string>
#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>


///
class AsyncLoader
{
	public:
		/// Default constructor starts the thread
		AsyncLoader() {start();}
		
		// Do nothing
		~AsyncLoader() {}

		/// Load a binary file and put the data in the preallocated buffer
		void loadInPreallocatedBuff(const char* filename, void* buff, size_t size);

		/// Load in a new buff
		void loadInNewBuff(const char* filename);

		/// Query the loader and see if its got something
		/// @param[out] filename The file that finished loading
		/// @param[out] buff The data are stored in this buffer
		/// @param[out] size The buffer size
		/// @return Return true if there is something that finished loading
		bool getLoaded(std::string& filename, void*& buff, size_t& size);

	private:
		struct File
		{
			std::string filename;
			void* data;
			size_t size;
		};

		std::list<File> in;
		std::list<File> out;
		boost::mutex mutexIn;
		boost::mutex mutexOut;
		boost::thread thread;
		boost::condition_variable condVar;

		void workingFunc(); ///< The thread function

		/// Start thread
		void start();
};


#endif
