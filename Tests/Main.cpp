// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/Filesystem.h>

using namespace anki;

ANKI_MAIN_FUNCTION(myMain)
int myMain(int argc, char** argv)
{
	int exitcode = getTesterSingleton().run(argc, argv);

	deleteTesterSingleton();

	return exitcode;
}
