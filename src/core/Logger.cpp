#include "anki/core/Logger.h"
#include "anki/core/App.h"
#include <cstring>
#include <cstdarg>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android/log.h>
#endif

namespace anki {

//==============================================================================
void Logger::init(U32 flags)
{
	if(flags & INIT_SYSTEM_MESSAGE_HANDLER)
	{
		addMessageHandler(this, &defaultSystemMessageHandler);
	}

	if(flags & INIT_LOG_FILE_MESSAGE_HANDLER)
	{
		addMessageHandler(this, &logfileMessageHandler);
	}
}

//==============================================================================
void Logger::addMessageHandler(void* data, MessageHandlerCallback callback)
{
	handlers.push_back(Handler{data, callback});
}

//==============================================================================
void Logger::write(const char* file, int line, const char* func,
	LoggerMessageType type, const char* msg)
{
	mutex.lock();

	Info inf = {file, line, func, type, msg};

	for(Handler& handler : handlers)
	{
		handler.callback(handler.data, inf);
	}

	mutex.unlock();
}

//==============================================================================
void Logger::writeFormated(const char* file, int line, const char* func,
	LoggerMessageType type, const char* fmt, ...)
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

//==============================================================================
void Logger::logfileMessageHandler(void* vlogger, const Info& info)
{
	Logger* logger = (Logger*)vlogger;

	// Init the file
	if(!logger->logfile.isOpen())
	{
		const std::string& filename = AppSingleton::get().getSettingsPath();

		if(!filename.empty())
		{
			logger->logfile.open((filename + "/anki.log").c_str(), 
				File::OF_WRITE);
		}
		else
		{
			return;
		}
	}

	const char* x = nullptr;
	switch(info.type)
	{
	case Logger::LMT_NORMAL:
		x = "Info";
		break;
	case Logger::LMT_ERROR:
		x = "Error";
		break;
	case Logger::LMT_WARNING:
		x = "Warn";
		break;
	}

	logger->logfile.writeText("(%s:%d %s) %s: %s\n", 
		info.file, info.line, info.func, x, info.msg);

	logger->logfile.flush();
}

} // end namespace anki
