// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Filesystem.h"
#include "anki/util/Assert.h"
#include <windows.h>

namespace anki {

//==============================================================================
Bool fileExists(const CString& filename)
{
	DWORD dwAttrib = GetFileAttributes(filename.get());

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
	Error err = ErrorCode::NONE;
	SHFILEOPSTRUCTA fileOperation;
	fileOperation.wFunc = FO_DELETE;
	fileOperation.pFrom = dirname.get();
	fileOperation.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION;

	I result = SHFileOperationA(&fileOperation);
	if(result != 0) 
	{
		ANKI_LOGE("Could not delete directory %s", dirname.get());
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
Error getHomeDirectory(HeapAllocator<U8>& alloc, String& out);
{
	const char* homed = getenv("HOMEDRIVE");
	const char* homep = getenv("HOMEPATH");

	if(homed == nullptr || homep == nullptr)
	{
		ANKI_LOGE("HOME environment variables not set");
		return ErrorCode::FUNCTION_FAILED;
	}

	String out;
	Error err = out.sprintf("%s/%s", homed, homep);
	if(!err)
	{
		// Convert to Unix path
		for(char& c : out)
		{
			if(c == '\\')
			{
				c = '/';
			}
		}
	}

	return err;
}

} // end namespace anki
