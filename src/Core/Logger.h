#ifndef LOGGER_H
#define LOGGER_H

#include <boost/array.hpp>
#include <boost/signals2.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include "Singleton.h"


struct LoggerSender;


/// The logger singleton class. The logger cannot print errors or throw exceptions, it has to recover somehow. Its
/// thread safe
class Logger
{
	public:
		typedef boost::signals2::signal<void (const char*, int, const char*, const char*)> Signal; ///< Signal type

		Logger() {execCommonConstructionCode();}

		/// Accessor
		Signal& getSignal() {return sig;}

		/// @name Operators for numbers
		/// @{
		Logger& operator<<(const bool& val) {return appendUsingLexicalCast(val);}
		Logger& operator<<(const short& val) {return appendUsingLexicalCast(val);}
		Logger& operator<<(const unsigned short& val) {return appendUsingLexicalCast(val);}
		Logger& operator<<(const int& val) {return appendUsingLexicalCast(val);}
		Logger& operator<<(const unsigned int& val) {return appendUsingLexicalCast(val);}
		Logger& operator<<(const long& val) {return appendUsingLexicalCast(val);}
		Logger& operator<<(const unsigned long& val) {return appendUsingLexicalCast(val);}
		Logger& operator<<(const float& val) {return appendUsingLexicalCast(val);}
		Logger& operator<<(const double& val) {return appendUsingLexicalCast(val);}
		Logger& operator<<(const long double& val) {return appendUsingLexicalCast(val);}
		/// @}

		/// @name Operators for other types
		/// @{
		Logger& operator<<(const void* val);
		Logger& operator<<(const char* val);
		Logger& operator<<(const std::string& val);
		Logger& operator<<(Logger& (*funcPtr)(Logger&));
		Logger& operator<<(const LoggerSender& sender);
		/// @}

		/// An alternative method to write in the Logger
		void write(const char* file, int line, const char* func, const char* msg);

	private:
		static const int STREAM_SIZE = 2048;
		boost::array<char, STREAM_SIZE> streamBuf;
		char* sptr; ///< Pointer to streamBuf
		Signal sig; ///< The signal
		const char* func; ///< Sender info
		const char* file; ///< Sender info
		int line; ///< Sender info
		boost::mutex mutex; ///< For thread safety

		/// Called by all the constructors
		void execCommonConstructionCode();

		/// Appends to streamBuf. On overflow it writes what it cans and flushes
		void append(const char* cstr, int len);

		/// Append finalize streamBuf and send the signal
		void flush();

		/// Because we are bored to write
		template<typename Type>
		Logger& appendUsingLexicalCast(const Type& val);
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

template<typename Type>
Logger& Logger::appendUsingLexicalCast(const Type& val)
{
	std::string out;
	try
	{
		out = boost::lexical_cast<std::string>(val);
	}
	catch(...)
	{
		out = "*error*";
	}
	append(out.c_str(), out.length());
	return *this;
}


//======================================================================================================================
// IO manipulation non-members                                                                                         =
//======================================================================================================================

/// Add a new line and flush the Logger
inline Logger& endl(Logger& logger)
{
	return logger;
}


/// Flush the Logger
inline Logger& flush(Logger& logger)
{
	return logger;
}


/// Record the sender
struct LoggerSender
{
	const char* file;
	int line;
	const char* func;
};


/// Help the Logger to set the sender
inline LoggerSender setSender(const char* file, int line, const char* func)
{
	LoggerSender sender = {file, line, func};
	return sender;
}


//======================================================================================================================
// Macros                                                                                                              =
//======================================================================================================================

#define LOGGER_MESSAGE(x) \
	LoggerSingleton::getInstance()  << setSender(__FILE__, __LINE__, __func__) << x << endl;

#define INFO(x) LOGGER_MESSAGE("Info: " << x)

#define WARNING(x) LOGGER_MESSAGE("Warning: " << x)

#define ERROR(x) LOGGER_MESSAGE("Error: " << x)


//======================================================================================================================
// Singleton                                                                                                           =
//======================================================================================================================

typedef Singleton<Logger> LoggerSingleton;


#endif
