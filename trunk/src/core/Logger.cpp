#include "anki/core/Logger.h"
#include <cstring>
#include <cstdarg>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android/log.h>
#endif

namespace anki {

//==============================================================================
Logger::Logger()
{
	ANKI_CONNECT(this, messageRecieved, this, defaultSystemMessageHandler);
}

//==============================================================================
void Logger::write(const char* file, int line, const char* func,
	LoggerMessageType type, const char* msg)
{
	mutex.lock();

	Info inf = {file, line, func, type, msg};
	ANKI_EMIT messageRecieved(inf);

	mutex.unlock();
}

//==============================================================================
void Logger::writeFormated(const char* file, int line, const char* func,
	LoggerMessageType type, const char* fmt, ...)
{
	char buffer[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	write(file, line, func, type, buffer);
	va_end(args);
}

//==============================================================================
void Logger::defaultSystemMessageHandler(const Info& info)
{
#if ANKI_OS == ANKI_OS_LINUX
	std::ostream* out = NULL;
	const char* x = NULL;
	const char* terminalColor = nullptr;

	switch(info.type)
	{
	case Logger::LMT_NORMAL:
		out = &std::cout;
		x = "Info";
		terminalColor = "\033[0;32m";
		break;
	case Logger::LMT_ERROR:
		out = &std::cerr;
		x = "Error";
		terminalColor = "\033[0;31m";
		break;
	case Logger::LMT_WARNING:
		out = &std::cerr;
		x = "Warn";
		terminalColor = "\033[0;33m";
		break;
	}

	(*out) << terminalColor << "(" << info.file << ":" << info.line << " "
		<< info.func << ") " << x << ": " << info.msg << "\033[0m" << std::endl;
#elif ANKI_OS == ANKI_OS_ANDROID
	U32 andMsgType = ANDROID_LOG_INFO;

	switch(info.type)
	{
	case Logger::LMT_NORMAL:
		andMsgType = ANDROID_LOG_INFO;
		break;
	case Logger::LMT_ERROR:
		andMsgType = ANDROID_LOG_ERROR;
		break;
	case Logger::LMT_WARNING:
		andMsgType = ANDROID_LOG_WARN;
		break;
	}

	std::stringstream ss;

	__android_log_print(andMsgType, "AnKi", "(%s:%d %s) %s", info.file,
		info.line, info.func, info.msg);
#else
	std::cout << "(" << info.file << ":" << info.line << " "
		<< info.func << ") " << x << ": " << info.msg << std::endl;
#endif
}

} // end namespace anki
