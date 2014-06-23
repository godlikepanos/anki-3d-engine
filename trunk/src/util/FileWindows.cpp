// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/File.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include <windows.h>

namespace anki {

//==============================================================================
// File                                                                        =
//==============================================================================

//==============================================================================
Bool File::fileExists(const char* filename)
{
	ANKI_ASSERT(filename);

	DWORD dwAttrib = GetFileAttributes(filename);

	return dwAttrib != INVALID_FILE_ATTRIBUTES 
		&& !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

//==============================================================================
// Functions                                                                   =
//==============================================================================

//==============================================================================
Bool directoryExists(const char* filename)
{
	ANKI_ASSERT(filename);

	DWORD dwAttrib = GetFileAttributes(filename);

	return dwAttrib != INVALID_FILE_ATTRIBUTES 
		&& (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

//==============================================================================
void removeDirectory(const char* dirname)
{
	ANKI_ASSERT(dirname);

	SHFILEOPSTRUCTW fileOperation;
	fileOperation.wFunc = FO_DELETE;
	fileOperation.pFrom = dirname;
	fileOperation.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION;

	I result = SHFileOperationW(&fileOperation);
	if(result != 0) 
	{
		throw ANKI_EXCEPTION("Could not delete directory %s", dirname);
	}
}

//==============================================================================
void createDirectory(const char* dir)
{
	ANKI_ASSERT(dir);

	if(CreateDirectory(dir, NULL) 
		|| GetLastError() == ERROR_ALREADY_EXISTS)
	{
		throw ANKI_EXCEPTION("Failed to create directory %s", dir);
	}
}

} // end namespace anki
