// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Logger.h"
#include "anki/util/File.h"
#include <cstring>
#include <cstdarg>
#include <cstdio>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android/log.h>
#endif

namespace anki {

//==============================================================================
static const Array<const char*, static_cast<U>(Logger::MessageType::COUNT)>
	MSG_TEXT = {{"Info", "Error", "Warning", "Fatal"}};

//==============================================================================
Logger::Logger()
{
	addMessageHandler(this, &defaultSystemMessageHandler);
}

//==============================================================================
Logger::~Logger()
{}

//==============================================================================
void Logger::addMessageHandler(void* data, MessageHandlerCallback callback)
{
	m_handlers[m_handlersCount++] =  Handler(data, callback);
}

//==============================================================================
void Logger::write(const char* file, int line, const char* func,
	MessageType type, const char* msg)
{
	m_mutex.lock();

	Info inf = {file, line, func, type, msg};

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

//==============================================================================
void Logger::writeFormated(const char* file, int line, const char* func,
	MessageType type, const char* fmt, ...)
{
	char buffer[1024 * 20];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	write(file, line, func, type, buffer);
	va_end(args);
}

//==============================================================================
void Logger::defaultSystemMessageHandler(void*, const Info& info)
{
#if ANKI_OS == ANKI_OS_LINUX
	FILE* out = nullptr;
	const char* terminalColor = nullptr;

	switch(info.m_type)
	{
	case Logger::MessageType::NORMAL:
		out = stdout;
		terminalColor = "\033[0;32m";
		break;
	case Logger::MessageType::ERROR:
		out = stderr;
		terminalColor = "\033[0;31m";
		break;
	case Logger::MessageType::WARNING:
		out = stderr;
		terminalColor = "\033[0;33m";
		break;
	case Logger::MessageType::FATAL:
		out = stderr;
		terminalColor = "\033[0;31m";
		break;
	default:
		ANKI_ASSERT(0);
	}

	fprintf(out, "%s(%s:%d %s) %s: %s\033[0m\n", terminalColor, info.m_file,
		info.m_line, info.m_func, MSG_TEXT[static_cast<U>(info.m_type)],
		info.m_msg);
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

	__android_log_print(andMsgType, "AnKi", "(%s:%d %s) %s", info.m_file,
		info.m_line, info.m_func, info.m_msg);
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

	fprintf(out, "(%s:%d %s) %s: %s\n", info.m_file,
		info.m_line, info.m_func, MSG_TEXT[static_cast<U>(info.m_type)],
		info.m_msg);

	fflush(out);
#endif
}

//==============================================================================
void Logger::fileMessageHandler(void* pfile, const Info& info)
{
	File* file = reinterpret_cast<File*>(pfile);

	Error err = file->writeText("(%s:%d %s) %s: %s\n",
		info.m_file, info.m_line, info.m_func,
		MSG_TEXT[static_cast<U>(info.m_type)], info.m_msg);

	if(!err)
	{
		err = file->flush();
		(void)err;
	}
}

} // end namespace anki
