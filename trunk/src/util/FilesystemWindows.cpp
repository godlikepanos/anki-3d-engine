// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Filesystem.h"
#include "anki/util/Exception.h"
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
void removeDirectory(const CString& dirname)
{
	SHFILEOPSTRUCTA fileOperation;
	fileOperation.wFunc = FO_DELETE;
	fileOperation.pFrom = dirname.get();
	fileOperation.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION;

	I result = SHFileOperationA(&fileOperation);
	if(result != 0) 
	{
		throw ANKI_EXCEPTION("Could not delete directory %s", dirname.get());
	}
}

//==============================================================================
void createDirectory(const CString& dir)
{
	if(CreateDirectory(dir.get(), NULL) == 0)
	{
		throw ANKI_EXCEPTION("Failed to create directory %s", dir.get());
	}
}

//==============================================================================
String getHomeDirectory(HeapAllocator<U8>& alloc)
{
	const char* homed = getenv("HOMEDRIVE");
	const char* homep = getenv("HOMEPATH");

	if(homed == nullptr || homep == nullptr)
	{
		throw ANKI_EXCEPTION("HOME environment variables not set");
	}

	String out(alloc);
	out.reserve(std::strlen(homed) + std::strlen(homep));

	out += CString(homed);
	out += CString(homep);

	// Convert to Unix path
	for(char& c : out)
	{
		if(c == '\\')
		{
			c = '/';
		}
	}

	return out;
}

} // end namespace anki
