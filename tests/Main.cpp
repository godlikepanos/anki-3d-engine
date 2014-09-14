// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/core/Logger.h"
#include "anki/util/Filesystem.h"

using namespace anki;

int main(int argc, char** argv)
{
	HeapAllocator<U8> alloc(HeapMemoryPool(allocAligned, nullptr));

	// Call a few singletons to avoid memory leak confusion
	LoggerSingleton::init(
		Logger::InitFlags::WITH_SYSTEM_MESSAGE_HANDLER,
		alloc, &(getHomeDirectory(alloc) + ".anki/tests.log")[0]);

	int exitcode = getTesterSingleton().run(argc, argv);

	LoggerSingleton::destroy();

	deleteTesterSingleton();

	return exitcode;
}
