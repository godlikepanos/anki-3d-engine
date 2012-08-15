#include "tests/framework/Framework.h"

using namespace anki;

int main(int argc, char** argv)
{
	return TesterSingleton::get().runAll();
}
