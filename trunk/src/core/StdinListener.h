#ifndef STDIN_LISTENER_H
#define STDIN_LISTENER_H

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <string>
#include <queue>
#include "util/Singleton.h"


/// The listener of the stdin.
/// It initiates a thread that constantly reads the stdin and puts the results
/// in a queue
class StdinListener
{
	public:
		/// Get line from the queue or return an empty string
		std::string getLine();

		/// Start reading
		void start();

	private:
		std::queue<std::string> q;
		boost::mutex mtx; ///< Protect the queue
		boost::thread thrd; ///< The thread

		void workingFunc(); ///< The thread function
};


#endif
