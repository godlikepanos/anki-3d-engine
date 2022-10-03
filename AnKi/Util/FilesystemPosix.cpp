// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Some defines for extra posix features
#define _XOPEN_SOURCE 700
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/Assert.h>
#include <AnKi/Util/Thread.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cerrno>
#include <ftw.h> // For walkDirectoryTree
#include <cstdlib>
#include <time.h>
#include <unistd.h>
#if ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#endif

#ifndef USE_FDS
#	define USE_FDS 15
#endif

// Define PATH_MAX if needed
#ifndef PATH_MAX
#	define PATH_MAX 4096
#endif

namespace anki {

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
	const Function<Error(const CString&, Bool)>* m_callback = nullptr;
	U32 m_prefixLen;
	Error m_err = {Error::kNone};
};

static thread_local WalkDirectoryTreeCallbackContext g_walkDirectoryTreeContext;

static int walkDirectoryTreeCallback(const char* filepath, [[maybe_unused]] const struct stat* info, const int typeflag,
									 [[maybe_unused]] struct FTW* pathinfo)
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

		ctx.m_err = (*ctx.m_callback)(filepath + ctx.m_prefixLen, isDir);
	}

	return 0;
}

Error walkDirectoryTreeInternal(const CString& dir, const Function<Error(const CString&, Bool)>& callback)
{
	ANKI_ASSERT(dir.getLength() > 0);
	Error err = Error::kNone;

	// Compute how long it will be the prefix fts_read will add
	U32 prefixLen = dir.getLength();
	if(dir[prefixLen - 1] != '/')
	{
		++prefixLen;
	}

	WalkDirectoryTreeCallbackContext& ctx = g_walkDirectoryTreeContext;
	ctx.m_callback = &callback;
	ctx.m_prefixLen = prefixLen;
	ctx.m_err = Error::kNone;

	const int result = nftw(dir.cstr(), walkDirectoryTreeCallback, USE_FDS, FTW_PHYS);
	if(result != 0)
	{
		ANKI_UTIL_LOGE("nftw() failed");
		err = Error::kFunctionFailed;
	}
	else
	{
		err = ctx.m_err;
	}

	return err;
}

static Error removeDirectoryInternal(const CString& dirname, BaseMemoryPool& pool)
{
	DIR* dir;
	struct dirent* entry;

	dir = opendir(dirname.cstr());
	if(dir == nullptr)
	{
		ANKI_UTIL_LOGE("opendir() failed");
		return Error::kFunctionFailed;
	}

	while((entry = readdir(dir)) != nullptr)
	{
		if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
		{
			StringRaii path(&pool);
			path.sprintf("%s/%s", dirname.cstr(), entry->d_name);

			if(entry->d_type == DT_DIR)
			{
				Error err = removeDirectoryInternal(path.toCString(), pool);
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

	return Error::kNone;
}

Error removeDirectory(const CString& dirname, BaseMemoryPool& pool)
{
	return removeDirectoryInternal(dirname, pool);
}

Error createDirectory(const CString& dir)
{
	if(directoryExists(dir))
	{
		return Error::kNone;
	}

	Error err = Error::kNone;
	if(mkdir(dir.cstr(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
	{
		ANKI_UTIL_LOGE("%s : %s", strerror(errno), dir.cstr());
		err = Error::kFunctionFailed;
	}

	return err;
}

Error getHomeDirectory(StringRaii& out)
{
#if ANKI_OS_LINUX
	const char* home = getenv("HOME");
	if(home == nullptr)
	{
		ANKI_UTIL_LOGE("HOME environment variable not set");
		return Error::kFunctionFailed;
	}

	out.create(home);
#else
	out.create(g_androidApp->activity->internalDataPath);
#endif

	return Error::kNone;
}

Error getTempDirectory(StringRaii& out)
{
#if ANKI_OS_LINUX
	out.create("/tmp/");
#else
	out.create(g_androidApp->activity->internalDataPath);
#endif
	return Error::kNone;
}

Error getFileModificationTime(CString filename, U32& year, U32& month, U32& day, U32& hour, U32& min, U32& second)
{
	struct stat buff;
	if(lstat(filename.cstr(), &buff))
	{
		ANKI_UTIL_LOGE("stat() failed");
		return Error::kNone;
	}

	struct tm t;
	localtime_r(&buff.st_mtim.tv_sec, &t);
	year = 1900 + t.tm_year;
	month = t.tm_mon + 1;
	day = t.tm_mday;
	hour = t.tm_hour;
	min = t.tm_min;
	second = t.tm_sec;

	return Error::kNone;
}

Error getApplicationPath(StringRaii& out)
{
#if ANKI_OS_ANDROID
	ANKI_ASSERT(0 && "getApplicationPath() doesn't work on Android");
	(void)out;
#else
	DynamicArrayRaii<Char> buff(&out.getMemoryPool(), 1024);

	const ssize_t result = readlink("/proc/self/exe", &buff[0], buff.getSize());
	if(result < 0)
	{
		ANKI_UTIL_LOGE("readlink() failed");
		return Error::kFunctionFailed;
	}

	out.destroy();
	out.create('0', result);

	memcpy(&out[0], &buff[0], result);
#endif

	return Error::kNone;
}

} // end namespace anki
