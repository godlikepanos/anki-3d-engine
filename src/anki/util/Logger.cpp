// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Logger.h>
#include <anki/util/File.h>
#include <anki/util/System.h>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android/log.h>
#endif

namespace anki
{

static const Array<const char*, static_cast<U>(Logger::MessageType::COUNT)> MSG_TEXT = {{"I", "E", "W", "F"}};

Logger::Logger()
{
	addMessageHandler(this, &defaultSystemMessageHandler);
}

Logger::~Logger()
{
}

void Logger::addMessageHandler(void* data, MessageHandlerCallback callback)
{
	m_handlers[m_handlersCount++] = Handler(data, callback);
}

void Logger::write(
	const char* file, int line, const char* func, const char* subsystem, MessageType type, const char* msg)
{
	m_mutex.lock();

	Info inf = {file, line, func, type, msg, subsystem};

	U count = m_handlersCount;
	while(count-- != 0)
	{
		m_handlers[count].m_callback(m_handlers[count].m_data, inf);
	}

	m_mutex.unlock();

	if(type == MessageType::FATAL)
	{
		abort();
	}
}

void Logger::writeFormated(
	const char* file, int line, const char* func, const char* subsystem, MessageType type, const char* fmt, ...)
{
	char buffer[1024 * 10];
	va_list args;

	va_start(args, fmt);
	I len = vsnprintf(buffer, sizeof(buffer), fmt, args);
	if(len < 0)
	{
		fprintf(stderr, "Logger::writeFormated() failed. Will not recover");
		abort();
	}
	else if(len < I(sizeof(buffer)))
	{
		write(file, line, func, subsystem, type, buffer);
		va_end(args);
	}
	else
	{
		// Not enough space.

		va_end(args);
		va_start(args, fmt);

		const PtrSize newSize = len + 1;
		char* newBuffer = static_cast<char*>(malloc(newSize));
		len = vsnprintf(newBuffer, newSize, fmt, args);

		write(file, line, func, subsystem, type, newBuffer);

		free(newBuffer);
		va_end(args);
	}
}

void Logger::defaultSystemMessageHandler(void*, const Info& info)
{
#if ANKI_OS == ANKI_OS_LINUX
	FILE* out = nullptr;
	const char* terminalColor = nullptr;
	const char* terminalColorBg = nullptr;
	const char* endTerminalColor = "\033[0m";

	switch(info.m_type)
	{
	case Logger::MessageType::NORMAL:
		out = stdout;
		terminalColor = "\033[0;32m";
		terminalColorBg = "\033[1;42;37m";
		break;
	case Logger::MessageType::ERROR:
		out = stderr;
		terminalColor = "\033[0;31m";
		terminalColorBg = "\033[1;41;37m";
		break;
	case Logger::MessageType::WARNING:
		out = stderr;
		terminalColor = "\033[2;33m";
		terminalColorBg = "\033[1;43;37m";
		break;
	case Logger::MessageType::FATAL:
		out = stderr;
		terminalColor = "\033[0;31m";
		terminalColorBg = "\033[1;41;37m";
		break;
	default:
		ANKI_ASSERT(0);
	}

	const char* fmt = "%s[%s][%s]%s%s %s (%s:%d %s)%s\n";
	if(!runningFromATerminal())
	{
		terminalColor = "";
		terminalColorBg = "";
		endTerminalColor = "";
	}

	fprintf(out,
		fmt,
		terminalColorBg,
		MSG_TEXT[static_cast<U>(info.m_type)],
		info.m_subsystem ? info.m_subsystem : "N/A ",
		endTerminalColor,
		terminalColor,
		info.m_msg,
		info.m_file,
		info.m_line,
		info.m_func,
		endTerminalColor);
#elif ANKI_OS == ANKI_OS_ANDROID
	U32 andMsgType = ANDROID_LOG_INFO;

	switch(info.m_type)
	{
	case Logger::MessageType::NORMAL:
		andMsgType = ANDROID_LOG_INFO;
		break;
	case Logger::MessageType::ERROR:
		andMsgType = ANDROID_LOG_ERROR;
		break;
	case Logger::MessageType::WARNING:
		andMsgType = ANDROID_LOG_WARN;
		break;
	case Logger::MessageType::FATAL:
		andMsgType = ANDROID_LOG_ERROR;
		break;
	default:
		ANKI_ASSERT(0);
	}

	std::stringstream ss;

	__android_log_print(andMsgType, "AnKi", "%s (%s:%d %s)", info.m_msg, info.m_file, info.m_line, info.m_func);
#else
	FILE* out = NULL;

	switch(info.m_type)
	{
	case Logger::MessageType::NORMAL:
		out = stdout;
		break;
	case Logger::MessageType::ERROR:
		out = stderr;
		break;
	case Logger::MessageType::WARNING:
		out = stderr;
		break;
	case Logger::MessageType::FATAL:
		out = stderr;
		break;
	default:
		ANKI_ASSERT(0);
	}

	fprintf(out,
		"[%s][%s] %s (%s:%d %s)\n",
		MSG_TEXT[static_cast<U>(info.m_type)],
		info.m_subsystem ? info.m_subsystem : "N/A ",
		info.m_msg,
		info.m_file,
		info.m_line,
		info.m_func);

	fflush(out);
#endif
}

void Logger::fileMessageHandler(void* pfile, const Info& info)
{
	File* file = reinterpret_cast<File*>(pfile);

	Error err = file->writeText("[%s] %s (%s:%d %s)\n",
		MSG_TEXT[static_cast<U>(info.m_type)],
		info.m_msg,
		info.m_file,
		info.m_line,
		info.m_func);

	if(!err)
	{
		err = file->flush();
		(void)err;
	}
}

} // end namespace anki
