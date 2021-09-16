// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/Filesystem.h>

using namespace anki;

ANKI_MAIN_FUNCTION(myMain)
int myMain(int argc, char** argv)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Call a few singletons to avoid memory leak confusion
	LoggerSingleton::get();

	int exitcode = getTesterSingleton().run(argc, argv);

	LoggerSingleton::destroy();

	deleteTesterSingleton();

	return exitcode;
}
