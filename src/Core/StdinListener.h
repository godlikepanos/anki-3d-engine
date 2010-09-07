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
		StdinListener(Object* parent = NULL);
		~StdinListener() {}
		void workingFunc(); ///< The thread function
		string getLine();

	private:
		queue<string> q;
		mutex mtx;

		StdinListener(const StdinListener&): Object(NULL) {} ///< Non copyable
};


inline StdinListener::StdinListener(Object* parent):
	Object(parent)
{}


#endif
