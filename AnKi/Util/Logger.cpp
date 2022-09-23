// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/File.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/System.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#if ANKI_OS_ANDROID
#	include <android/log.h>
#endif
#if ANKI_OS_WINDOWS
#	include <AnKi/Util/Win32Minimal.h>
#endif

namespace anki {

inline constexpr Array<const Char*, U(LoggerMessageType::kCount)> kMessageTypeTxt = {"I", "V", "E", "W", "F"};

Logger::Logger()
{
	addMessageHandler(this, &defaultSystemMessageHandler);

	const Char* envVar = getenv("ANKI_LOG_VERBOSE");
	if(envVar && envVar == CString("1"))
	{
		m_verbosityEnabled = true;
	}
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

void Logger::write(const Char* file, int line, const Char* func, const Char* subsystem, LoggerMessageType type,
				   const Char* threadName, const Char* msg)
{
	// Note: m_verbosityEnabled is not accessed in a thread-safe way. It doesn't really matter though
	if(type == LoggerMessageType::kVerbose && !m_verbosityEnabled)
	{
		return;
	}

	const Char* baseFile = strrchr(file, (ANKI_OS_WINDOWS) ? '\\' : '/');
	baseFile = (baseFile) ? baseFile + 1 : file;

	LoggerMessageInfo inf = {baseFile, line, func, type, msg, subsystem, threadName};

	m_mutex.lock();

	U count = m_handlersCount;
	while(count-- != 0)
	{
		m_handlers[count].m_callback(m_handlers[count].m_data, inf);
	}

	m_mutex.unlock();

	if(type == LoggerMessageType::kFatal)
	{
		abort();
	}
}

void Logger::writeFormated(const Char* file, int line, const Char* func, const Char* subsystem, LoggerMessageType type,
						   const Char* threadName, const Char* fmt, ...)
{
	Array<Char, 256> buffer;
	va_list args;

	va_start(args, fmt);
	I len = vsnprintf(&buffer[0], sizeof(buffer), fmt, args);
	if(len < 0)
	{
		fprintf(stderr, "Logger::writeFormated() failed. Will not recover");
		abort();
	}
	else if(len < I(sizeof(buffer)))
	{
		write(file, line, func, subsystem, type, threadName, &buffer[0]);
		va_end(args);
	}
	else
	{
		// Not enough space.

		va_end(args);
		va_start(args, fmt);

		const PtrSize newSize = len + 1;
		Char* newBuffer = static_cast<Char*>(malloc(newSize));
		len = vsnprintf(newBuffer, newSize, fmt, args);

		write(file, line, func, subsystem, type, threadName, newBuffer);

		free(newBuffer);
		va_end(args);
	}
}

void Logger::defaultSystemMessageHandler(void*, const LoggerMessageInfo& info)
{
#if ANKI_OS_LINUX
	// More info about terminal colors:
	// https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences
	FILE* out = nullptr;
	const Char* terminalColor = nullptr;
	const Char* terminalColorBg = nullptr;
	const Char* endTerminalColor = "\033[0m";

	switch(info.m_type)
	{
	case LoggerMessageType::kNormal:
		out = stdout;
		terminalColor = "\033[0;32m";
		terminalColorBg = "\033[1;42;37m";
		break;
	case LoggerMessageType::kVerbose:
		out = stdout;
		terminalColor = "\033[0;34m";
		terminalColorBg = "\033[1;44;37m";
		break;
	case LoggerMessageType::kError:
		out = stderr;
		terminalColor = "\033[0;31m";
		terminalColorBg = "\033[1;41;37m";
		break;
	case LoggerMessageType::kWarning:
		out = stderr;
		terminalColor = "\033[2;33m";
		terminalColorBg = "\033[1;43;37m";
		break;
	case LoggerMessageType::kFatal:
		out = stderr;
		terminalColor = "\033[0;31m";
		terminalColorBg = "\033[1;41;37m";
		break;
	default:
		ANKI_ASSERT(0);
	}

	static_assert(Thread::kThreadNameMaxLength == 15, "See file");
	if(!runningFromATerminal())
	{
		terminalColor = "";
		terminalColorBg = "";
		endTerminalColor = "";
	}

	fprintf(out, "%s[%s][%s]%s%s %s [%s:%d][%s][%s]%s\n", terminalColorBg, kMessageTypeTxt[U(info.m_type)],
			info.m_subsystem ? info.m_subsystem : "N/A ", endTerminalColor, terminalColor, info.m_msg, info.m_file,
			info.m_line, info.m_func, info.m_threadName, endTerminalColor);
#elif ANKI_OS_WINDOWS
	WORD attribs = 0;
	FILE* out = nullptr;
	switch(info.m_type)
	{
	case LoggerMessageType::kNormal:
		attribs |= FOREGROUND_GREEN;
		out = stdout;
		break;
	case LoggerMessageType::kVerbose:
		attribs |= FOREGROUND_BLUE;
		out = stdout;
		break;
	case LoggerMessageType::kError:
		attribs |= FOREGROUND_RED;
		out = stderr;
		break;
	case LoggerMessageType::kWarning:
		attribs |= FOREGROUND_RED | FOREGROUND_GREEN;
		out = stderr;
		break;
	case LoggerMessageType::kFatal:
		attribs |= FOREGROUND_RED | FOREGROUND_INTENSITY;
		out = stderr;
		break;
	default:
		ANKI_ASSERT(0);
		out = stdout;
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
		fprintf(out, "[%s][%-4s] %s [%s:%d][%s][%s]\n", kMessageTypeTxt[info.m_type],
				info.m_subsystem ? info.m_subsystem : "N/A", info.m_msg, info.m_file, info.m_line, info.m_func,
				info.m_threadName);

		// Restore state
		SetConsoleTextAttribute(consoleHandle, savedAttribs);
	}
#elif ANKI_OS_ANDROID
	I32 andMsgType = ANDROID_LOG_INFO;

	switch(info.m_type)
	{
	case LoggerMessageType::kNormal:
	case LoggerMessageType::kVerbose:
		andMsgType = ANDROID_LOG_INFO;
		break;
	case LoggerMessageType::kError:
		andMsgType = ANDROID_LOG_ERROR;
		break;
	case LoggerMessageType::kWarning:
		andMsgType = ANDROID_LOG_WARN;
		break;
	case LoggerMessageType::kFatal:
		andMsgType = ANDROID_LOG_ERROR;
		break;
	default:
		ANKI_ASSERT(0);
	}

	static_assert(Thread::kThreadNameMaxLength == 15, "See file");
	__android_log_print(andMsgType, "AnKi", "[%s][%-4s] %s [%s:%d][%s][%s]\n", kMessageTypeTxt[info.m_type],
						info.m_subsystem ? info.m_subsystem : "N/A ", info.m_msg, info.m_file, info.m_line, info.m_func,
						info.m_threadName);
#else
#	error "Not implemented"
#endif
}

void Logger::fileMessageHandler(void* pfile, const LoggerMessageInfo& info)
{
	File* file = reinterpret_cast<File*>(pfile);

	Error err = file->writeTextf("[%s] %s (%s:%d %s)\n", kMessageTypeTxt[info.m_type], info.m_msg, info.m_file,
								 info.m_line, info.m_func);

	if(!err)
	{
		err = file->flush();
	}
}

} // end namespace anki
