// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Filesystem.h"
#include "anki/util/Assert.h"
#include "anki/util/Logger.h"
#include <windows.h>

// Someone pollutes the global namespace
#undef ERROR

namespace anki {

//==============================================================================
Bool fileExists(const CString& filename)
{
	DWORD dwAttrib = GetFileAttributes(&filename[0]);

	return dwAttrib != INVALID_FILE_ATTRIBUTES
		&& !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

//==============================================================================
Bool directoryExists(const CString& filename)
{
	DWORD dwAttrib = GetFileAttributes(filename.get());

	return dwAttrib != INVALID_FILE_ATTRIBUTES
		&& (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

//==============================================================================
Error removeDirectory(const CString& dirname)
{
	// For some reason dirname should be double null terminated
	const U PATH_SIZE = 1024 * 2;
	if(dirname.getLength() > PATH_SIZE - 2)
	{
		ANKI_LOGE("Path too big");
		return ErrorCode::FUNCTION_FAILED;
	}

	Array<char, PATH_SIZE> dirname2;
	memcpy(&dirname2[0], &dirname[0], dirname.getLength() + 1);
	dirname2[dirname.getLength() + 1] = '\0';

	Error err = ErrorCode::NONE;
	SHFILEOPSTRUCTA fileOperation = {
		NULL,
		FO_DELETE,
		&dirname2[0],
		"",
		FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT,
		false,
		0,
		""};

	I result = SHFileOperationA(&fileOperation);
	if(result != 0)
	{
		ANKI_LOGE("Could not delete directory %s", &dirname[0]);
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

//==============================================================================
Error createDirectory(const CString& dir)
{
	Error err = ErrorCode::NONE;
	if(CreateDirectory(dir.get(), NULL) == 0)
	{
		ANKI_LOGE("Failed to create directory %s", dir.get());
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

//==============================================================================
Error getHomeDirectory(GenericMemoryPoolAllocator<U8> alloc, String& out)
{
	const char* homed = getenv("HOMEDRIVE");
	const char* homep = getenv("HOMEPATH");

	if(homed == nullptr || homep == nullptr)
	{
		ANKI_LOGE("HOME environment variables not set");
		return ErrorCode::FUNCTION_FAILED;
	}

	out.sprintf(alloc, "%s/%s", homed, homep);

	// Convert to Unix path
	for(char& c : out)
	{
		if(c == '\\')
		{
			c = '/';
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki
