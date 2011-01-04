#include <cstdio>
#include "Logger.h"


Logger* Logger::instance = NULL;


//======================================================================================================================
// operator<< [const char*]                                                                                            =
//======================================================================================================================
Logger& Logger::operator<<(const char* val)
{
	append(val, strlen(val));
	return *this;
}


//======================================================================================================================
// operator<< [Logger& (*funcPtr)(Logger&)]                                                                            =
//======================================================================================================================
Logger& Logger::operator<<(Logger& (*funcPtr)(Logger&))
{
	printf("Got some func\n");
	if(funcPtr == endl)
	{
		append("\n", 1);
		flush();
	}
	return *this;
}


//======================================================================================================================
//                                                                                                                     =
//======================================================================================================================
Logger& Logger::operator<<(const LoggerSender& sender)
{
	file = sender.file;
	line = sender.line;
	func = sender.func;
	return *this;
}


//======================================================================================================================
// execCommonConstructionCode                                                                                          =
//======================================================================================================================
void Logger::execCommonConstructionCode()
{
	sptr = &streamBuf[0];
	memset(sptr, '?', STREAM_SIZE);
	func = file = "error";
	line = -1;
}


//======================================================================================================================
// append                                                                                                              =
//======================================================================================================================
void Logger::append(const char* cstr, int len)
{
	if(len > STREAM_SIZE - 1)
	{
		return;
	}

	int charsLeft = &streamBuf[STREAM_SIZE - 1] - sptr; // Leaving an extra char for the '\0'

	// Overflow
	if(len > charsLeft)
	{
		// Handle overflow
		memcpy(sptr, cstr, charsLeft);
		sptr += charsLeft;
		flush();
		append(cstr + charsLeft, len - charsLeft);
		return;
	}

	memcpy(sptr, cstr, len);
	sptr += len;
}


//======================================================================================================================
// flush                                                                                                               =
//======================================================================================================================
void Logger::flush()
{
	*sptr = '\0';
	sig(file, line, func, &streamBuf[0]);

	// Reset
	sptr = &streamBuf[0];
	func = file = "error";
	line = -1;

	printf("Flushing: |%s|\n", &streamBuf[0]);
}
