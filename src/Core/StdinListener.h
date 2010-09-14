#ifndef STDIN_LISTENER_H
#define STDIN_LISTENER_H

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <string>
#include <queue>
#include "Common.h"
#include "Object.h"


/**
 * The listener of the stdin
 */
class StdinListener: public Object
{
	public:
		StdinListener(Object* parent = NULL): Object(parent) {}
		~StdinListener() {}
		string getLine();
		void start();

	private:
		queue<string> q;
		mutex mtx;
		thread thrd;

		StdinListener(const StdinListener&); ///< Non copyable
		void workingFunc(); ///< The thread function
};


#endif
