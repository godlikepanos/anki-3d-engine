#include "tests/framework/Framework.h"
#include "anki/core/Logger.h"

using namespace anki;

struct LoggerMessageHandler
{
	ANKI_HAS_SLOTS(LoggerMessageHandler)

	void handleLoggerMessages(const Logger::Info& info)
	{
		std::ostream* out = NULL;
		const char* x = NULL;

		switch(info.type)
		{
		case Logger::LMT_NORMAL:
			out = &std::cout;
			x = "Info";
			break;

		case Logger::LMT_ERROR:
			out = &std::cerr;
			x = "Error";
			break;

		case Logger::LMT_WARNING:
			out = &std::cerr;
			x = "Warn";
			break;
		}

		(*out) << "(" << info.file << ":" << info.line << " "<< info.func 
			<< ") " << x << ": " << info.msg << std::endl;
	}

	ANKI_SLOT(handleLoggerMessages, const Logger::Info&)
};

static LoggerMessageHandler msgh;

int main(int argc, char** argv)
{
	// Call a few singletons to avoid memory leak confusion
	LoggerSingleton::get();

	ANKI_CONNECT(&LoggerSingleton::get(), messageRecieved, 
		&msgh, handleLoggerMessages);

	int exitcode = getTesterSingleton().run(argc, argv);

	deleteTesterSingleton();

	return exitcode;
}
