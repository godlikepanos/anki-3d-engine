#ifndef STDIN_LISTENER_H
#define STDIN_LISTENER_H

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <string>
#include <queue>
#include "Object.h"


/// The listener of the stdin
class StdinListener: public Object
{
	public:
		StdinListener(Object* parent = NULL): Object(parent) {}
		~StdinListener() {}
		std::string getLine();
		void start();

	private:
		std::queue<std::string> q;
		boost::mutex mtx;
		boost::thread thrd;

		StdinListener(const StdinListener&); ///< Non copyable
		void workingFunc(); ///< The thread function
};


#endif
