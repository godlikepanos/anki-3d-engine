// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/File.h"
#include "anki/util/Exception.h"
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
void removeDirectory(const CString& dirname)
{
	DIR* dir;
	struct dirent* entry;
	char path[PATH_MAX];

	if(path == nullptr) 
	{
		throw ANKI_EXCEPTION("Out of memory error");
	}

	dir = opendir(dirname.get());
	if(dir == nullptr) 
	{
		throw ANKI_EXCEPTION("opendir() failed");
	}

	while((entry = readdir(dir)) != nullptr) 
	{
		if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) 
		{
			std::snprintf(
				path, (size_t)PATH_MAX, "%s/%s", dirname.get(), entry->d_name);

			if(entry->d_type == DT_DIR) 
			{
				removeDirectory(CString(path));
			}
			else
			{
				remove(path);
			}
		}

	}

	closedir(dir);
	remove(dirname.get());
}

//==============================================================================
void createDirectory(const CString& dir)
{
	if(directoryExists(dir))
	{
		return;
	}

	if(mkdir(dir.get(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
	{
		throw ANKI_EXCEPTION("%s : %s", strerror(errno), dir.get());
	}
}

//==============================================================================
String getHomeDirectory(HeapAllocator<U8>& alloc)
{
	const char* home = getenv("HOME");
	if(home == nullptr)
	{
		throw ANKI_EXCEPTION("HOME environment variable not set");
	}

	return String(CString(home), alloc);
}

} // end namespace anki
