#ifndef ANKI_CORE_STDIN_LISTENER_H
#define ANKI_CORE_STDIN_LISTENER_H

#include "anki/util/Singleton.h"
#include <thread>
#include <mutex>
#include <string>
#include <queue>

namespace anki {

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
	std::mutex mtx; ///< Protect the queue
	std::thread thrd; ///< The thread

	void workingFunc(); ///< The thread function
};

/// Singleton
typedef Singleton<StdinListener> StdinListenerSingleton;

} // end namespace anki

#endif
