// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/File.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
// File                                                                        =
//==============================================================================

//==============================================================================
Bool File::fileExists(const char* filename)
{
	ANKI_ASSERT(filename);
	// TODO
}

//==============================================================================
// Functions                                                                   =
//==============================================================================

//==============================================================================
Bool directoryExists(const char* filename)
{
	ANKI_ASSERT(filename);
	// TODO
}

//==============================================================================
void removeDirectory(const char* dirname)
{
	// TODO
}

//==============================================================================
void createDirectory(const char* dir)
{
	// TODO
}

} // end namespace anki
