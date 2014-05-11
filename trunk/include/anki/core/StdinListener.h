#ifndef ANKI_CORE_STDIN_LISTENER_H
#define ANKI_CORE_STDIN_LISTENER_H

#include "anki/util/Singleton.h"
#include "anki/util/String.h"
#include <thread>
#include <mutex>
#include <queue>

namespace anki {

/// @addtogroup core
/// @{

/// The listener of the stdin.
/// It initiates a thread that constantly reads the stdin and puts the results
/// in a queue
class StdinListener
{
public:
	/// Get line from the queue or return an empty string
	String getLine();

	/// Start reading
	void start();

private:
	std::queue<String> m_q;
	std::mutex m_mtx; ///< Protect the queue
	std::thread m_thrd; ///< The thread

	void workingFunc(); ///< The thread function
};

/// Singleton
typedef Singleton<StdinListener> StdinListenerSingleton;

/// @}

} // end namespace anki

#endif
