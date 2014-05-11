#include "anki/core/Logger.h"
#include "anki/core/App.h"
#include <cstring>
#include <cstdarg>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android/log.h>
#endif

namespace anki {

//==============================================================================
void Logger::init(InitFlags flags, HeapAllocator<U8>& alloc)
{
	if((flags & InitFlags::WITH_SYSTEM_MESSAGE_HANDLER) != InitFlags::NONE)
	{
		addMessageHandler(this, &defaultSystemMessageHandler);
	}

	if((flags & InitFlags::WITH_LOG_FILE_MESSAGE_HANDLER) != InitFlags::NONE)
	{
		addMessageHandler(this, &logfileMessageHandler);
	}
}

//==============================================================================
void Logger::addMessageHandler(void* data, MessageHandlerCallback callback)
{
	m_handlers.push_back(Handler{data, callback});
}

//==============================================================================
void Logger::write(const char* file, int line, const char* func,
	MessageType type, const char* msg)
{
	m_mutex.lock();

	Info inf = {file, line, func, type, msg};

	for(Handler& handler : m_handlers)
	{
		handler.m_callback(handler.m_data, inf);
	}

	m_mutex.unlock();
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
	std::ostream* out = NULL;
	const char* x = NULL;
	const char* terminalColor = nullptr;

	switch(info.m_type)
	{
	case Logger::MessageType::NORMAL:
		out = &std::cout;
		x = "Info";
		terminalColor = "\033[0;32m";
		break;
	case Logger::MessageType::ERROR:
		out = &std::cerr;
		x = "Error";
		terminalColor = "\033[0;31m";
		break;
	case Logger::MessageType::WARNING:
		out = &std::cerr;
		x = "Warn";
		terminalColor = "\033[0;33m";
		break;
	}

	(*out) << terminalColor << "(" << info.m_file << ":" << info.m_line << " "
		<< info.m_func << ") " << x << ": " << info.m_msg 
		<< "\033[0m" << std::endl;
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
	}

	std::stringstream ss;

	__android_log_print(andMsgType, "AnKi", "(%s:%d %s) %s", info.m_file,
		info.m_line, info.m_func, info.m_msg);
#else
	std::ostream* out = NULL;
	const char* x = NULL;

	switch(info.m_type)
	{
	case Logger::MessageType::NORMAL:
		out = &std::cout;
		x = "Info";
		break;
	case Logger::MessageType::ERROR:
		out = &std::cerr;
		x = "Error";
		break;
	case Logger::MessageType::WARNING:
		out = &std::cerr;
		x = "Warn";
		break;
	}

	(*out) << "(" << info.m_file << ":" << info.m_line << " "
		<< info.m_func << ") " << x << ": " << info.m_msg << std::endl;
#endif
}

//==============================================================================
void Logger::logfileMessageHandler(void* vlogger, const Info& info)
{
	Logger* logger = (Logger*)vlogger;

	// Init the file
	if(!logger->m_logfile.isOpen())
	{
		const String& filename = AppSingleton::get().getSettingsPath();

		if(!filename.empty())
		{
			logger->m_logfile.open((filename + "/anki.log").c_str(), 
				File::OpenFlag::WRITE);
		}
		else
		{
			return;
		}
	}

	const char* x = nullptr;
	switch(info.m_type)
	{
	case Logger::MessageType::NORMAL:
		x = "Info";
		break;
	case Logger::MessageType::ERROR:
		x = "Error";
		break;
	case Logger::MessageType::WARNING:
		x = "Warn";
		break;
	}

	logger->m_logfile.writeText("(%s:%d %s) %s: %s\n", 
		info.m_file, info.m_line, info.m_func, x, info.m_msg);

	logger->m_logfile.flush();
}

} // end namespace anki
