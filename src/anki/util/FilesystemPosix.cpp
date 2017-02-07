// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
#include <fts.h> // For walkDirectoryTree
#include <cstdlib>

// Define PATH_MAX if needed
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

namespace anki
{

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

Error walkDirectoryTree(const CString& dir, void* userData, WalkDirectoryTreeCallback callback)
{
	ANKI_ASSERT(callback != nullptr);
	ANKI_ASSERT(dir.getLength() > 0);

	char* const dirs[] = {const_cast<char*>(&dir[0]), nullptr};

	// Compute how long it will be the prefix fts_read will add
	U prefixLen = dir.getLength();
	if(dir[prefixLen - 1] != '/')
	{
		++prefixLen;
	}

	// FTS_NOCHDIR and FTS_NOSTAT are opts. FTS_LOGICAL will follow symlics
	FTS* tree = fts_open(&dirs[0], FTS_NOCHDIR | FTS_LOGICAL | FTS_NOSTAT, nullptr);

	if(!tree)
	{
		ANKI_UTIL_LOGE("fts_open() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	Error err = ErrorCode::NONE;
	FTSENT* node;
	while((node = fts_read(tree)) && !err)
	{
		if(node->fts_info & FTS_DP)
		{
			// Skip if already visited
			continue;
		}

		const char* fname = node->fts_path;
		U len = strlen(fname);
		if(len <= prefixLen)
		{
			continue;
		}

		const char* newPath = fname + prefixLen;

		Bool isFile = (node->fts_info & FTS_F) == FTS_F;
		err = callback(newPath, userData, !isFile);
	}

	if(errno)
	{
		ANKI_UTIL_LOGE("fts_read() failed: %s", strerror(errno));
		err = ErrorCode::FUNCTION_FAILED;
	}

	if(fts_close(tree))
	{
		ANKI_UTIL_LOGE("fts_close() failed");
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

// To avoid having the g_removeDirectoryPath in stack or having to allocate it, make it global.
static char g_removeDirectoryPath[PATH_MAX];
Mutex g_removeDirectoryLock;

Error removeDirectory(const CString& dirname)
{
	DIR* dir;
	struct dirent* entry;

	dir = opendir(dirname.get());
	if(dir == nullptr)
	{
		ANKI_UTIL_LOGE("opendir() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	LockGuard<Mutex> lock(g_removeDirectoryLock);
	while((entry = readdir(dir)) != nullptr)
	{
		if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
		{
			std::snprintf(g_removeDirectoryPath, size_t(PATH_MAX), "%s/%s", dirname.get(), entry->d_name);

			if(entry->d_type == DT_DIR)
			{
				Error err = removeDirectory(CString(g_removeDirectoryPath));
				if(err)
				{
					return err;
				}
			}
			else
			{
				remove(g_removeDirectoryPath);
			}
		}
	}

	closedir(dir);
	remove(dirname.get());

	return ErrorCode::NONE;
}

Error createDirectory(const CString& dir)
{
	if(directoryExists(dir))
	{
		return ErrorCode::NONE;
	}

	Error err = ErrorCode::NONE;
	if(mkdir(dir.get(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
	{
		ANKI_UTIL_LOGE("%s : %s", strerror(errno), dir.get());
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

Error getHomeDirectory(GenericMemoryPoolAllocator<U8> alloc, String& out)
{
	const char* home = getenv("HOME");
	if(home == nullptr)
	{
		ANKI_UTIL_LOGE("HOME environment variable not set");
		return ErrorCode::FUNCTION_FAILED;
	}

	out.create(alloc, home);
	return ErrorCode::NONE;
}

} // end namespace anki
