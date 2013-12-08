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
// File                                                                        =
//==============================================================================

//==============================================================================
Bool File::fileExists(const char* filename)
{
	ANKI_ASSERT(filename);
	struct stat s;
	if(stat(filename, &s) == 0)
	{
		return S_ISREG(s.st_mode);
	}
	else
	{
		return false;
	}
}

//==============================================================================
// Functions                                                                   =
//==============================================================================

//==============================================================================
Bool directoryExists(const char* filename)
{
	ANKI_ASSERT(filename);
	struct stat s;
	if(stat(filename, &s) == 0)
	{
		return S_ISDIR(s.st_mode);
	}
	else
	{
		return false;
	}
}

//==============================================================================
void removeDirectory(const char* dirname)
{
	DIR* dir;
	struct dirent* entry;
	char path[PATH_MAX];

	if(path == nullptr) 
	{
		throw ANKI_EXCEPTION("Out of memory error");
	}

	dir = opendir(dirname);
	if(dir == nullptr) 
	{
		throw ANKI_EXCEPTION("opendir() failed");
	}

	while((entry = readdir(dir)) != nullptr) 
	{
		if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) 
		{
			snprintf(path, (size_t)PATH_MAX, "%s/%s", dirname, entry->d_name);
			if(entry->d_type == DT_DIR) 
			{
				removeDirectory(path);
			}
			else
			{
				remove(path);
			}
		}

	}

	closedir(dir);
	remove(dirname);
}

//==============================================================================
void createDirectory(const char* dir)
{
	if(directoryExists(dir))
	{
		return;
	}

	if(mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
	{
		throw ANKI_EXCEPTION("%s : %s", strerror(errno), dir);
	}
}

} // end namespace anki
