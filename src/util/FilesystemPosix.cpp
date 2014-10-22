// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/File.h"
#include "anki/util/Assert.h"
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cerrno>

// Define PATH_MAX if needed
#ifndef PATH_MAX
#	define PATH_MAX 4096
#endif

namespace anki {

//==============================================================================
Bool fileExists(const CString& filename)
{
	struct stat s;
	if(stat(filename.get(), &s) == 0)
	{
		return S_ISREG(s.st_mode);
	}
	else
	{
		return false;
	}
}

//==============================================================================
Bool directoryExists(const CString& filename)
{
	struct stat s;
	if(stat(filename.get(), &s) == 0)
	{
		return S_ISDIR(s.st_mode);
	}
	else
	{
		return false;
	}
}

//==============================================================================
Error removeDirectory(const CString& dirname)
{
	DIR* dir;
	struct dirent* entry;
	char path[PATH_MAX];

	if(path == nullptr) 
	{
		ANKI_LOGE("Out of memory error");
		return ErrorCode::FUNCTION_FAILED;
	}

	dir = opendir(dirname.get());
	if(dir == nullptr) 
	{
		ANKI_LOGE("opendir() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	while((entry = readdir(dir)) != nullptr) 
	{
		if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) 
		{
			std::snprintf(
				path, (size_t)PATH_MAX, "%s/%s", dirname.get(), entry->d_name);

			if(entry->d_type == DT_DIR) 
			{
				Error err = removeDirectory(CString(path));
				if(err)
				{
					return err;
				}
			}
			else
			{
				remove(path);
			}
		}

	}

	closedir(dir);
	remove(dirname.get());

	return ErrorCode::NONE;
}

//==============================================================================
Error createDirectory(const CString& dir)
{
	if(directoryExists(dir))
	{
		return ErrorCode::NONE;
	}

	Error err = ErrorCode::NONE;
	if(mkdir(dir.get(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
	{
		ANKI_LOGE("%s : %s", strerror(errno), dir.get());
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

//==============================================================================
Error getHomeDirectory(HeapAllocator<U8>& alloc, String& out)
{
	const char* home = getenv("HOME");
	if(home == nullptr)
	{
		ANKI_LOGE("HOME environment variable not set");
		return ErrorCode::FUNCTION_FAILED;
	}

	return out.create(alloc, home);
}

} // end namespace anki
