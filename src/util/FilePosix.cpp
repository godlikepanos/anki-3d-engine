#include "anki/util/File.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cerrno>

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

	if(path == NULL) 
	{
		throw ANKI_EXCEPTION("Out of memory error");
	}

	dir = opendir(dirname);
	if(dir == NULL) 
	{
		throw ANKI_EXCEPTION("opendir() failed");
	}

	while((entry = readdir(dir)) != NULL) 
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
		throw ANKI_EXCEPTION(strerror(errno) + ": " + dir);
	}
}

} // end namespace anki
