// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/File.h>
#include <anki/util/Logger.h>
#include <anki/util/System.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#if ANKI_OS_ANDROID
#	include <android/log.h>
#endif
#if ANKI_OS_WINDOWS
#	include <anki/util/Win32Minimal.h>
#endif

namespace anki
{

static const Array<const char*, static_cast<U>(LoggerMessageType::COUNT)> MSG_TEXT = {"I", "E", "W", "F"};

Logger::Logger()
{
	addMessageHandler(this, &defaultSystemMessageHandler);
}

Logger::~Logger()
{
}

void Logger::addMessageHandler(void* data, LoggerMessageHandlerCallback callback)
{
	LockGuard<Mutex> lock(m_mutex);
	m_handlers[m_handlersCount++] = Handler(data, callback);
}

void Logger::removeMessageHandler(void* data, LoggerMessageHandlerCallback callback)
{
	LockGuard<Mutex> lock(m_mutex);

	U i;
	for(i = 0; i < m_handlersCount; ++i)
	{
		if(m_handlers[i].m_callback == callback && m_handlers[i].m_data == data)
		{
			break;
		}
	}

	if(i < m_handlersCount)
	{
		for(U j = i + 1; j < m_handlersCount; ++j)
		{
			m_handlers[j - 1] = m_handlers[j];
		}
		--m_handlersCount;
	}
}

void Logger::write(const char* file, int line, const char* func, const char* subsystem, LoggerMessageType type,
				   ThreadId tid, const char* msg)
{
	m_mutex.lock();

	LoggerMessageInfo inf = {file, line, func, type, msg, subsystem, tid};

	U count = m_handlersCount;
	while(count-- != 0)
	{
		m_handlers[count].m_callback(m_handlers[count].m_data, inf);
	}

	m_mutex.unlock();

	if(type == LoggerMessageType::FATAL)
	{
		abort();
	}
}

void Logger::writeFormated(const char* file, int line, const char* func, const char* subsystem, LoggerMessageType type,
						   ThreadId tid, const char* fmt, ...)
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
		write(file, line, func, subsystem, type, tid, buffer);
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

		write(file, line, func, subsystem, type, tid, newBuffer);

		free(newBuffer);
		va_end(args);
	}
}

void Logger::defaultSystemMessageHandler(void*, const LoggerMessageInfo& info)
{
#if ANKI_OS_LINUX
	FILE* out = nullptr;
	const char* terminalColor = nullptr;
	const char* terminalColorBg = nullptr;
	const char* endTerminalColor = "\033[0m";

	switch(info.m_type)
	{
	case LoggerMessageType::NORMAL:
		out = stdout;
		terminalColor = "\033[0;32m";
		terminalColorBg = "\033[1;42;37m";
		break;
	case LoggerMessageType::ERROR:
		out = stderr;
		terminalColor = "\033[0;31m";
		terminalColorBg = "\033[1;41;37m";
		break;
	case LoggerMessageType::WARNING:
		out = stderr;
		terminalColor = "\033[2;33m";
		terminalColorBg = "\033[1;43;37m";
		break;
	case LoggerMessageType::FATAL:
		out = stderr;
		terminalColor = "\033[0;31m";
		terminalColorBg = "\033[1;41;37m";
		break;
	default:
		ANKI_ASSERT(0);
	}

	const char* fmt = "%s[%s][%s][%" PRIx64 "]%s%s %s (%s:%d %s)%s\n";
	if(!runningFromATerminal())
	{
		terminalColor = "";
		terminalColorBg = "";
		endTerminalColor = "";
	}

	fprintf(out, fmt, terminalColorBg, MSG_TEXT[static_cast<U>(info.m_type)],
			info.m_subsystem ? info.m_subsystem : "N/A ", info.m_tid, endTerminalColor, terminalColor, info.m_msg,
			info.m_file, info.m_line, info.m_func, endTerminalColor);
#elif ANKI_OS_WINDOWS
	WORD attribs = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
	FILE* out = NULL;
	switch(info.m_type)
	{
	case LoggerMessageType::NORMAL:
		attribs |= FOREGROUND_GREEN;
		out = stdout;
		break;
	case LoggerMessageType::ERROR:
		attribs |= FOREGROUND_RED;
		out = stderr;
		break;
	case LoggerMessageType::WARNING:
		attribs |= FOREGROUND_RED;
		out = stderr;
		break;
	case LoggerMessageType::FATAL:
		attribs |= FOREGROUND_RED | FOREGROUND_INTENSITY;
		out = stderr;
		break;
	default:
		ANKI_ASSERT(0);
	}

	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if(consoleHandle != nullptr)
	{
		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		WORD savedAttribs;

		// Save current attributes
		GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
		savedAttribs = consoleInfo.wAttributes;

		// Apply changes
		SetConsoleTextAttribute(consoleHandle, attribs);

		// Print
		fprintf(out, "[%s][%s] %s (%s:%d %s)\n", MSG_TEXT[static_cast<U>(info.m_type)],
				info.m_subsystem ? info.m_subsystem : "N/A ", info.m_msg, info.m_file, info.m_line, info.m_func);

		// Restore state
		SetConsoleTextAttribute(consoleHandle, savedAttribs);
	}
#elif ANKI_OS_ANDROID
	U32 andMsgType = ANDROID_LOG_INFO;

	switch(info.m_type)
	{
	case LoggerMessageType::NORMAL:
		andMsgType = ANDROID_LOG_INFO;
		break;
	case LoggerMessageType::ERROR:
		andMsgType = ANDROID_LOG_ERROR;
		break;
	case LoggerMessageType::WARNING:
		andMsgType = ANDROID_LOG_WARN;
		break;
	case LoggerMessageType::FATAL:
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
	case LoggerMessageType::NORMAL:
		out = stdout;
		break;
	case LoggerMessageType::ERROR:
		out = stderr;
		break;
	case LoggerMessageType::WARNING:
		out = stderr;
		break;
	case LoggerMessageType::FATAL:
		out = stderr;
		break;
	default:
		ANKI_ASSERT(0);
	}

	fprintf(out, "[%s][%s][%" PRIx64 "] %s (%s:%d %s)\n", MSG_TEXT[static_cast<U>(info.m_type)],
			info.m_subsystem ? info.m_subsystem : "N/A ", info.m_tid, info.m_msg, info.m_file, info.m_line,
			info.m_func);

	fflush(out);
#endif
}

void Logger::fileMessageHandler(void* pfile, const LoggerMessageInfo& info)
{
	File* file = reinterpret_cast<File*>(pfile);

	Error err = file->writeText("[%s] %s (%s:%d %s)\n", MSG_TEXT[static_cast<U>(info.m_type)], info.m_msg, info.m_file,
								info.m_line, info.m_func);

	if(!err)
	{
		err = file->flush();
		(void)err;
	}
}

} // end namespace anki
