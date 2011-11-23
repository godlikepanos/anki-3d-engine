#ifndef ANKI_RESOURCE_ASYNC_LOADER_H
#define ANKI_RESOURCE_ASYNC_LOADER_H

#include <list>
#include <string>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>


namespace anki {


/// Asynchronous operator
///
/// It creates a thread that executes requests on demand. It contains a queue
/// with requests.
/// @code async.pushBack(new RequestDerived(...)); @endcode
/// The AsyncOperator gets the ownership of the request and de-allocates it
/// when the request is served. Its not meant to be destroyed because of a
/// deadlock.
class AsyncOperator
{
	public:
		/// XXX
		class Request
		{
			public:
				bool ok;

				/// Called in the worker thread
				virtual void exec() = 0;

				/// Called in the main thread after the request is served
				virtual void postExec(AsyncOperator& al) = 0;

				/// XXX
				virtual std::string getInfo() const
				{
					return "no info";
				}
		};

		/// Default constructor starts the thread
		AsyncOperator()
		{
			start();
		}
		
		/// Do nothing
		~AsyncOperator()
		{}

		/// Add a new request in the queue
		void putBack(Request* newReq);

		/// Handle the served requests
		///
		/// Steps:
		/// - Gets the served requests
		/// - Executes the Request::postExec for those requests
		/// - Deletes them
		///
		/// @param[in] availableTime Max time to spend in the Request::postExec
		/// @return The number of requests served
		uint execPostLoad(float availableTime);

	private:
		std::list<Request*> requests;
		std::list<Request*> responses;
		boost::mutex mutexReq; ///< Protect the requests container
		boost::mutex mutexRes; ///< Protect the responses container
		boost::thread thread;
		boost::condition_variable condVar;

		/// The thread function. It waits for something in the requests
		/// container
		void workingFunc();

		void start(); ///< Start thread
};


} // end namespace


#endif
