#ifndef LOGGER_H
#define LOGGER_H

#include <boost/array.hpp>
#include <boost/signals2.hpp>
#include <boost/lexical_cast.hpp>


struct LoggerSender;


/// The logger singleton class. The logger cannot print errors or throw exceptions, it has to recover somehow
class Logger
{
	public:
		typedef boost::signals2::signal<void (const char*, int, const char*, const char*)> Signal;

		static Logger& getInstance();

		/// @name Numeric operators
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

		Logger& operator<<(const void* val);
		Logger& operator<<(const char* val);
		Logger& operator<<(Logger& (*funcPtr)(Logger&));
		Logger& operator<<(const LoggerSender& sender);

	private:
		static Logger* instance;
		static const int STREAM_SIZE = 25;
		boost::array<char, STREAM_SIZE> streamBuf;
		char* sptr; ///< Pointer to @ref streamBuf
		Signal sig; ///< The signal
		const char* func;
		const char* file;
		int line;

		/// @name Ensure its singleton
		/// @{
		Logger() {execCommonConstructionCode();}
		Logger(const Logger&) {execCommonConstructionCode();}
		void operator=(const Logger&) {}
		/// @}

		/// Called by all the constructors
		void execCommonConstructionCode();

		/// Appends to streamBuf. On overflow it writes what it cans and flushes
		void append(const char* cstr, int len);

		/// Append finalize streamBuf and send the signal
		void flush();

		template<typename Type>
		Logger& appendUsingLexicalCast(const Type& val);
};


inline Logger& Logger::getInstance()
{
	if(instance == NULL)
	{
		instance = new Logger();
	}
	return *instance;
}


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
// Non-members                                                                                                         =
//======================================================================================================================

/// Add a new line and flush the Logger
inline Logger& endl(Logger& logger)
{
	return logger;
}


struct LoggerSender
{
	const char* file;
	int line;
	const char* func;
};

///
inline LoggerSender setSender(const char* file, int line, const char* func)
{
	LoggerSender sender = {file, line, func};
	return sender;
}


#endif
