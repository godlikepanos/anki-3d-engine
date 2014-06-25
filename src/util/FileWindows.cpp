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

	SHFILEOPSTRUCTA fileOperation;
	fileOperation.wFunc = FO_DELETE;
	fileOperation.pFrom = dirname;
	fileOperation.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION;

	I result = SHFileOperationA(&fileOperation);
	if(result != 0) 
	{
		throw ANKI_EXCEPTION("Could not delete directory %s", dirname);
	}
}

//==============================================================================
void createDirectory(const char* dir)
{
	ANKI_ASSERT(dir);

	if(CreateDirectory(dir, NULL) == 0)
	{
		throw ANKI_EXCEPTION("Failed to create directory %s", dir);
	}
}

//==============================================================================
void getHomeDirectory(U32 buffSize, char* buff)
{
	const char* homed = getenv("HOMEDRIVE");
	const char* homep = getenv("HOMEPATH");

	if(homed == nullptr || homep == nullptr)
	{
		throw ANKI_EXCEPTION("HOME environment variables not set");
	}

	U len0 = strlen(homed);
	U len1 = strlen(homep);
	if(len0 + len1 + 1 > buffSize)
	{
		throw ANKI_EXCEPTION("buffSize too small");
	}

	memcpy(buff, homed, len0);
	memcpy(buff + len0, homep, len1 + 1);

	// Convert to Unix path
	while(*buff != '\0')
	{
		if(*buff == '\\')
		{
			*buff = '/';
		}
		++buff;
	}
}

} // end namespace anki
