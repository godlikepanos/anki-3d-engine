#include "tests/framework/Framework.h"
#include "anki/core/Logger.h"

using namespace anki;

int main(int argc, char** argv)
{
	// Call a few singletons to avoid memory leak confusion
	LoggerSingleton::get();

	return TesterSingleton::get().run(argc, argv);
}
