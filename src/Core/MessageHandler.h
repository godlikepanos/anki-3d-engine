#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <boost/signals2.hpp>
#include <string>
#include "Object.h"


/// A generic message handler.
/// Using @ref write method you send a message to the slots connected to @ref sig signal
class MessageHandler: public Object
{
	public:
		typedef boost::signals2::signal<void (const char*, int, const char*, const std::string&)> Signal;

		MessageHandler(Object* parent = NULL): Object(parent) {}

		/// Emmits a message to the slots connected to @ref sig. A simple wrapper to hide the signal
		/// @param file Who send the message
		/// @param line Who send the message
		/// @param func Who send the message
		/// @param msg The message to emmit
		void write(const char* file, int line, const char* func, const std::string& msg) {sig(file, line, func, msg);}

		/// Accessor
		Signal& getSignal() {return sig;}

	private:
		Signal sig; ///< The signal
};


#endif
