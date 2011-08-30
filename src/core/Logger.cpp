#include <cstring>
#include "Logger.h"


//==============================================================================
// operator<< [const void*]                                                    =
//==============================================================================
Logger& Logger::operator<<(const void* val)
{
	return appendUsingLexicalCast(val);
}


//==============================================================================
// operator<< [const char*]                                                    =
//==============================================================================
Logger& Logger::operator<<(const char* val)
{
	append(val, strlen(val));
	return *this;
}


//==============================================================================
// operator<< [std::string]                                                    =
//==============================================================================
Logger& Logger::operator<<(const std::string& val)
{
	append(val.c_str(), val.length());
	return *this;
}


//==============================================================================
// operator<< [Logger& (*funcPtr)(Logger&)]                                    =
//==============================================================================
Logger& Logger::operator<<(Logger& (*funcPtr)(Logger&))
{
	if(funcPtr == endl)
	{
		append("\n", 1);
		realFlush();
	}
	else if(funcPtr == flush)
	{
		realFlush();
	}

	return *this;
}


//==============================================================================
// operator<< [Info]                                                           =
//==============================================================================
Logger& Logger::operator<<(const Info& sender)
{
	file = sender.file;
	line = sender.line;
	func = sender.func;
	type = sender.type;
	return *this;
}


//==============================================================================
// write                                                                       =
//==============================================================================
void Logger::write(const char* file_, int line_, const char* func_,
	MessageType type_, const char* msg)
{
	file = file_;
	line = line_;
	func = func_;
	type = type_;
	append(msg, strlen(msg));
	realFlush();
}


//==============================================================================
// execCommonConstructionCode                                                  =
//==============================================================================
void Logger::execCommonConstructionCode()
{
	sptr = &streamBuf[0];
	memset(sptr, '?', STREAM_SIZE);
	func = file = "error";
	line = -1;
}


//==============================================================================
// append                                                                      =
//==============================================================================
void Logger::append(const char* cstr, int len)
{
	if(len > STREAM_SIZE - 1)
	{
		const char ERR[] = "**logger buffer overflow**";
		cstr = ERR;
		len = sizeof(ERR);
	}

	int charsLeft = &streamBuf[STREAM_SIZE - 1] - sptr; // Leaving an extra
	                                                    // char for the '\0'

	// Overflow
	if(len > charsLeft)
	{
		// Handle overflow
		memcpy(sptr, cstr, charsLeft);
		sptr += charsLeft;
		realFlush();
		append(cstr + charsLeft, len - charsLeft);
		return;
	}

	memcpy(sptr, cstr, len);
	sptr += len;
}


//==============================================================================
// realFlush                                                                   =
//==============================================================================
void Logger::realFlush()
{
	*sptr = '\0';
	sig(file, line, func, type, &streamBuf[0]);

	// Reset
	sptr = &streamBuf[0];
	func = file = "error";
	line = -1;
}
