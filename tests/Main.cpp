#include "tests/framework/Framework.h"
#include "anki/core/Logger.h"

using namespace anki;

int main(int argc, char** argv)
{
	// Call a few singletons to avoid memory leak confusion
	LoggerSingleton::get();
	LoggerSingleton::get().init(Logger::INIT_SYSTEM_MESSAGE_HANDLER);

	int exitcode = getTesterSingleton().run(argc, argv);

	deleteTesterSingleton();

	return exitcode;
}
