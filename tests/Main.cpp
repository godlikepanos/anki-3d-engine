// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/util/Filesystem.h"

using namespace anki;

int main(int argc, char** argv)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Call a few singletons to avoid memory leak confusion
	LoggerSingleton::get();

	int exitcode = getTesterSingleton().run(argc, argv);

	LoggerSingleton::destroy();

	deleteTesterSingleton();

	return exitcode;
}
