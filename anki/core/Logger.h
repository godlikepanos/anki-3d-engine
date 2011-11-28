#ifndef ANKI_CORE_LOGGER_H
#define ANKI_CORE_LOGGER_H

#include <boost/array.hpp>
#include <boost/signals2.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>


namespace anki {


/// The logger singleton class. The logger cannot print errors or throw
/// exceptions, it has to recover somehow. Its thread safe
class Logger
{
public:
	enum MessageType
	{
		MT_NORMAL,
		MT_ERROR,
		MT_WARNING
	};

	/// Record the sender
	struct Info
	{
		const char* file;
		int line;
		const char* func;
		MessageType type;
	};

	typedef boost::signals2::signal<void (const char*, int, const char*,
		MessageType, const char*)> Signal; ///< Signal type

	Logger()
	{
		execCommonConstructionCode();
	}

	/// Accessor
	Signal& getSignal() {return sig;}

	/// @name Operators for numbers
	/// @{
	Logger& operator<<(const bool& val)
	{
		return appendUsingLexicalCast(val);
	}
	Logger& operator<<(const short& val)
	{
		return appendUsingLexicalCast(val);
	}
	Logger& operator<<(const unsigned short& val)
	{
		return appendUsingLexicalCast(val);
	}
	Logger& operator<<(const int& val)
	{
		return appendUsingLexicalCast(val);
	}
	Logger& operator<<(const unsigned int& val)
	{
		return appendUsingLexicalCast(val);
	}
	Logger& operator<<(const long& val)
	{
		return appendUsingLexicalCast(val);
	}
	Logger& operator<<(const unsigned long& val)
	{
		return appendUsingLexicalCast(val);
	}
	Logger& operator<<(const float& val)
	{
		return appendUsingLexicalCast(val);
	}
	Logger& operator<<(const double& val)
	{
		return appendUsingLexicalCast(val);
	}
	Logger& operator<<(const long double& val)
	{
		return appendUsingLexicalCast(val);
	}
	/// @}

	/// @name Operators for other types
	/// @{
	Logger& operator<<(const void* val);
	Logger& operator<<(const char* val);
	Logger& operator<<(const std::string& val);
	Logger& operator<<(Logger& (*funcPtr)(Logger&));
	Logger& operator<<(const Info& sender);
	/// @}

	/// @name IO manipulation
	/// @{

	/// Help the Logger to set the sender
	static Info setInfo(const char* file, int line,
		const char* func, MessageType type)
	{
		Info sender = {file, line, func, type};
		return sender;
	}

	/// Add a new line and flush the Logger
	static Logger& endl(Logger& logger) {return logger;}

	/// Flush the Logger
	static Logger& flush(Logger& logger) {return logger;}

	/// @}

	/// An alternative method to write in the Logger
	void write(const char* file, int line, const char* func,
		MessageType type, const char* msg);

	/// Connect to signal
	template<typename F, typename T>
	void connect(F f, T t)
	{
		sig.connect(boost::bind(f, t, _1, _2, _3, _4, _5));
	}

	/// Mutex lock
	void lock()
	{
		mutex.lock();
	}

	/// Mutex unlock
	void unlock()
	{
		mutex.unlock();
	}

private:
	static const int STREAM_SIZE = 2048;
	boost::array<char, STREAM_SIZE> streamBuf;
	char* sptr; ///< Pointer to streamBuf
	Signal sig; ///< The signal
	const char* func; ///< Sender info
	const char* file; ///< Sender info
	int line; ///< Sender info
	MessageType type; ///< The type of the message
	boost::mutex mutex; ///< For thread safety

	/// Called by all the constructors
	void execCommonConstructionCode();

	/// Appends to streamBuf. On overflow it writes what it can and flushes
	void append(const char* cstr, int len);

	/// Append finalize streamBuf and send the signal
	void realFlush();

	/// Because we are bored to write
	template<typename Type>
	Logger& appendUsingLexicalCast(const Type& val);
};


//==============================================================================
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


} // end namespace


//==============================================================================
// Macros                                                                      =
//==============================================================================

#define ANKI_LOGGER_MESSAGE(t, msg) \
	do \
	{ \
		LoggerSingleton::get().lock(); \
		LoggerSingleton::get()  << Logger::setInfo(__FILE__, \
			__LINE__, __func__, t) << msg << Logger::endl; \
		LoggerSingleton::get().unlock(); \
	} while(false);

#define ANKI_INFO(x) ANKI_LOGGER_MESSAGE(Logger::MT_NORMAL, x)

#define ANKI_WARNING(x) ANKI_LOGGER_MESSAGE(Logger::MT_WARNING, x)

#define ANKI_ERROR(x) ANKI_LOGGER_MESSAGE(Logger::MT_ERROR, x)


#endif
