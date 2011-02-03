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
		
		/// Do nothing
		~AsyncLoader() {}

		/// Tell me what to load, how to load it and where to store it
		/// @param filename The file to load
		/// @param func How to load the file
		/// @param storage This points to the storage
		void load(const char* filename, bool (*func)(const char*, void*), void* storage);

		/// Query the loader and see if its got something
		/// @param[out] filename The file that finished loading
		/// @param[out] storage The data are stored in this buffer
		/// @return Return true if there is something that finished loading
		bool getLoaded(std::string& filename, void* storage, bool& ok);

	private:
		struct Request
		{
			std::string filename;
			bool (*func)(const char*, void*);
			void* storage;
		};

		/// It contains a few things to identify the response
		struct Response
		{
			std::string filename;
			void* storage;
			bool ok;
		};

		std::list<Request> requests;
		std::list<Response> responses;
		boost::mutex mutexReq;
		boost::mutex mutexResp;
		boost::thread thread;
		boost::condition_variable condVar;

		void workingFunc(); ///< The thread function
		void start(); ///< Start thread
};


#endif
