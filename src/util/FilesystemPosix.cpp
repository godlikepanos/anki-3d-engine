// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Filesystem.h>
#include <anki/util/Assert.h>
#include <anki/util/Thread.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cerrno>
#include <ftw.h> // For walkDirectoryTree

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
static Mutex walkDirMtx;
static WalkDirectoryTreeCallback walkDirCallback = nullptr;
static void* walkDirUserData = nullptr;
static Array<char, PATH_MAX> walkDirDir;

static int ftwCallback(
	const char* fpath,
	const struct stat* /*sb*/,
	int typeflag)
{
	Error err = ErrorCode::NONE;

	if(typeflag == FTW_F || typeflag == FTW_D)
	{
		CString path(fpath);

		// First item is the given directory
		if(walkDirDir[0] == '\0')
		{
			strncpy(&walkDirDir[0], fpath, PATH_MAX);
		}
		else
		{
			// Remove the base dir from the fpath
			ANKI_ASSERT(path.find(&walkDirDir[0]) == 0);
			fpath += strlen(&walkDirDir[0]) + 1;

			// Call the callback
			err = walkDirCallback(fpath, walkDirUserData, typeflag != FTW_F);
		}
	}
	else
	{
		ANKI_LOGW("Permission denied %s\n", fpath);
	}

	return err._getCodeInt();
}

Error walkDirectoryTree(
	const CString& dir,
	void* userData,
	WalkDirectoryTreeCallback callback)
{
	ANKI_ASSERT(callback != nullptr);

	LockGuard<Mutex> lock(walkDirMtx);

	walkDirCallback = callback;
	walkDirUserData = userData;
	walkDirDir[0] = '\0';
	const int MAX_OPEN_FILE_DESCRS = 1;

	int ierr = ftw(&dir[0], &ftwCallback, MAX_OPEN_FILE_DESCRS);
	Error err = static_cast<ErrorCode>(ierr);

	if(ierr < 0)
	{
		ANKI_LOGE("%s\n", strerror(errno));
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
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
Error getHomeDirectory(GenericMemoryPoolAllocator<U8> alloc, String& out)
{
	const char* home = getenv("HOME");
	if(home == nullptr)
	{
		ANKI_LOGE("HOME environment variable not set");
		return ErrorCode::FUNCTION_FAILED;
	}

	out.create(alloc, home);
	return ErrorCode::NONE;
}

} // end namespace anki
