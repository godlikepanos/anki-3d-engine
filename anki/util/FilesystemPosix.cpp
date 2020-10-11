// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Some defines for extra posix features
#define _XOPEN_SOURCE 700
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <anki/util/Filesystem.h>
#include <anki/util/Assert.h>
#include <anki/util/Thread.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cerrno>
#include <ftw.h> // For walkDirectoryTree
#include <cstdlib>
#include <time.h>

#ifndef USE_FDS
#	define USE_FDS 15
#endif

// Define PATH_MAX if needed
#ifndef PATH_MAX
#	define PATH_MAX 4096
#endif

namespace anki
{

Bool fileExists(const CString& filename)
{
	struct stat s;
	if(stat(filename.cstr(), &s) == 0)
	{
		return S_ISREG(s.st_mode);
	}
	else
	{
		return false;
	}
}

Bool directoryExists(const CString& filename)
{
	struct stat s;
	if(stat(filename.cstr(), &s) == 0)
	{
		return S_ISDIR(s.st_mode);
	}
	else
	{
		return false;
	}
}

class WalkDirectoryTreeCallbackContext
{
public:
	WalkDirectoryTreeCallback m_callback = nullptr;
	void* m_userData = nullptr;
	U32 m_prefixLen;
	Error m_err = {Error::NONE};
};

static thread_local WalkDirectoryTreeCallbackContext g_walkDirectoryTreeContext;

static int walkDirectoryTreeCallback(const char* filepath, const struct stat* info, const int typeflag,
									 struct FTW* pathinfo)
{
	Bool isDir;
	Bool ignored = true;
	if(typeflag == FTW_F)
	{
		isDir = false;
		ignored = false;
	}
	else if(typeflag == FTW_D || typeflag == FTW_DP)
	{
		isDir = true;
		ignored = false;
	}

	if(!ignored)
	{
		WalkDirectoryTreeCallbackContext& ctx = g_walkDirectoryTreeContext;
		ANKI_ASSERT(ctx.m_callback);

		if(ctx.m_err || strlen(filepath) <= ctx.m_prefixLen)
		{
			return 0;
		}

		ctx.m_err = ctx.m_callback(filepath + ctx.m_prefixLen, ctx.m_userData, isDir);
	}

	return 0;
}

Error walkDirectoryTree(const CString& dir, void* userData, WalkDirectoryTreeCallback callback)
{
	ANKI_ASSERT(callback != nullptr);
	ANKI_ASSERT(dir.getLength() > 0);
	Error err = Error::NONE;

	// Compute how long it will be the prefix fts_read will add
	U32 prefixLen = dir.getLength();
	if(dir[prefixLen - 1] != '/')
	{
		++prefixLen;
	}

	WalkDirectoryTreeCallbackContext& ctx = g_walkDirectoryTreeContext;
	ctx.m_callback = callback;
	ctx.m_userData = userData;
	ctx.m_prefixLen = prefixLen;
	ctx.m_err = Error::NONE;

	const int result = nftw(dir.cstr(), walkDirectoryTreeCallback, USE_FDS, FTW_PHYS);
	if(result != 0)
	{
		ANKI_UTIL_LOGE("nftw() failed");
		err = Error::FUNCTION_FAILED;
	}
	else
	{
		err = ctx.m_err;
	}

	return err;
}

static Error removeDirectoryInternal(const CString& dirname, GenericMemoryPoolAllocator<U8>& alloc)
{
	DIR* dir;
	struct dirent* entry;

	dir = opendir(dirname.cstr());
	if(dir == nullptr)
	{
		ANKI_UTIL_LOGE("opendir() failed");
		return Error::FUNCTION_FAILED;
	}

	while((entry = readdir(dir)) != nullptr)
	{
		if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
		{
			StringAuto path(alloc);
			path.sprintf("%s/%s", dirname.cstr(), entry->d_name);

			if(entry->d_type == DT_DIR)
			{
				Error err = removeDirectoryInternal(path.toCString(), alloc);
				if(err)
				{
					return err;
				}
			}
			else
			{
				remove(path.cstr());
			}
		}
	}

	closedir(dir);
	remove(dirname.cstr());

	return Error::NONE;
}

Error removeDirectory(const CString& dirname, GenericMemoryPoolAllocator<U8> alloc)
{
	return removeDirectoryInternal(dirname, alloc);
}

Error createDirectory(const CString& dir)
{
	if(directoryExists(dir))
	{
		return Error::NONE;
	}

	Error err = Error::NONE;
	if(mkdir(dir.cstr(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
	{
		ANKI_UTIL_LOGE("%s : %s", strerror(errno), dir.cstr());
		err = Error::FUNCTION_FAILED;
	}

	return err;
}

Error getHomeDirectory(StringAuto& out)
{
	const char* home = getenv("HOME");
	if(home == nullptr)
	{
		ANKI_UTIL_LOGE("HOME environment variable not set");
		return Error::FUNCTION_FAILED;
	}

	out.create(home);
	return Error::NONE;
}

Error getFileModificationTime(CString filename, U32& year, U32& month, U32& day, U32& hour, U32& min, U32& second)
{
	struct stat buff;
	if(lstat(filename.cstr(), &buff))
	{
		ANKI_UTIL_LOGE("stat() failed");
		return Error::NONE;
	}

	const struct tm& t = *localtime(&buff.st_mtim.tv_sec);
	year = 1900 + t.tm_year;
	month = t.tm_mon + 1;
	day = t.tm_mday;
	hour = t.tm_hour;
	second = t.tm_sec;

	return Error::NONE;
}

} // end namespace anki
