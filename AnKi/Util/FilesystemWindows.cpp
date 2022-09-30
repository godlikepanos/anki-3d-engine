// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/Assert.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Win32Minimal.h>

namespace anki {

static constexpr U kMaxPathLen = MAX_PATH - 1;

Bool fileExists(const CString& filename)
{
	DWORD dwAttrib = GetFileAttributesA(&filename[0]);

	return dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

Bool directoryExists(const CString& filename)
{
	DWORD dwAttrib = GetFileAttributesA(filename.cstr());

	return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

Error removeDirectory(const CString& dirname, [[maybe_unused]] BaseMemoryPool& pool)
{
	// For some reason dirname should be double null terminated
	if(dirname.getLength() > MAX_PATH - 2)
	{
		ANKI_UTIL_LOGE("Path too big");
		return Error::kFunctionFailed;
	}

	Array<char, MAX_PATH> dirname2;
	memcpy(&dirname2[0], &dirname[0], dirname.getLength() + 1);
	dirname2[dirname.getLength() + 1] = '\0';

	Error err = Error::kNone;
	SHFILEOPSTRUCTA fileOperation = {
		NULL, FO_DELETE, &dirname2[0], "", FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT, false, 0, ""};

	I result = SHFileOperationA(&fileOperation);
	if(result != 0)
	{
		ANKI_UTIL_LOGE("Could not delete directory %s", &dirname[0]);
		err = Error::kFunctionFailed;
	}

	return err;
}

Error createDirectory(const CString& dir)
{
	Error err = Error::kNone;
	if(CreateDirectoryA(dir.cstr(), NULL) == 0)
	{
		ANKI_UTIL_LOGE("Failed to create directory %s", dir.cstr());
		err = Error::kFunctionFailed;
	}

	return err;
}

Error getHomeDirectory(StringRaii& out)
{
	char path[MAX_PATH];
	if(SHGetFolderPathA(NULL, CSIDL_PROFILE, nullptr, 0, path) != S_OK)
	{
		ANKI_UTIL_LOGE("SHGetFolderPath() failed");
		return Error::kFunctionFailed;
	}

	out.create(path);
	return Error::kNone;
}

Error getTempDirectory(StringRaii& out)
{
	char path[MAX_PATH + 1];

	const DWORD len = GetTempPathA(sizeof(path), path);
	if(len == 0)
	{
		ANKI_UTIL_LOGE("GetTempPathA() failed");
		return Error::kFunctionFailed;
	}

	out.create(path);
	return Error::kNone;
}

static Error walkDirectoryTreeRecursive(const CString& dir, const Function<Error(const CString&, Bool)>& callback,
										U baseDirLen)
{
	// Append something to the path
	if(dir.getLength() > kMaxPathLen - 2)
	{
		ANKI_UTIL_LOGE("Path too long");
		return Error::kFunctionFailed;
	}

	Array<char, MAX_PATH> dir2;
	memcpy(&dir2[0], &dir[0], dir.getLength());
	if(dir[dir.getLength() - 1] != '/')
	{
		dir2[dir.getLength() + 0] = '/';
		dir2[dir.getLength() + 1] = '*';
		dir2[dir.getLength() + 2] = '\0';
	}
	else
	{
		dir2[dir.getLength() + 0] = '*';
		dir2[dir.getLength() + 1] = '\0';
	}

	// Find files
	HANDLE handle = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA find;

	handle = FindFirstFileA(&dir2[0], &find);
	if(handle == INVALID_HANDLE_VALUE)
	{
		ANKI_UTIL_LOGE("FindFirstFile() failed");
		return Error::kFunctionFailed;
	}

	// Remove '*' from dir2
	dir2[strlen(&dir2[0]) - 1] = '\0';

	do
	{
		CString filename(find.cFileName);

		if(filename != "." && filename != "..")
		{
			Bool isDir = find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

			// Compute new path
			const PtrSize oldLen = strlen(&dir2[0]);
			if(oldLen + filename.getLength() > kMaxPathLen)
			{
				ANKI_UTIL_LOGE("Path too long");
				return Error::kFunctionFailed;
			}

			strcat(&dir2[0], &filename[0]);
			const Error err = callback(&dir2[0] + baseDirLen, isDir);
			if(err)
			{
				FindClose(handle);
				return err;
			}

			// Move to next dir
			if(isDir)
			{
				const Error err = walkDirectoryTreeRecursive(&dir2[0], callback, baseDirLen);
				if(err)
				{
					FindClose(handle);
					return err;
				}
			}

			dir2[oldLen] = '\0';
		}
	} while(FindNextFileA(handle, &find) != 0);

	if(GetLastError() != ERROR_NO_MORE_FILES)
	{
		ANKI_UTIL_LOGE("Unknown error");
		FindClose(handle);
		return Error::kFunctionFailed;
	}

	FindClose(handle);
	return Error::kNone;
}

Error walkDirectoryTreeInternal(const CString& dir, const Function<Error(const CString&, Bool)>& callback)
{
	U baseDirLen = 0;
	const U len = dir.getLength();
	if(dir[len - 1] == '/')
	{
		baseDirLen = len;
	}
	else
	{
		baseDirLen = len + 1;
	}

	return walkDirectoryTreeRecursive(dir, callback, baseDirLen);
}

Error getApplicationPath(StringRaii& out)
{
	DynamicArrayRaii<Char> buff(&out.getMemoryPool(), 1024);

	const DWORD result = GetModuleFileNameA(nullptr, &buff[0], buff.getSize());
	DWORD lastError = GetLastError();
	if(result == 0
	   || (result == buff.getSize() && (lastError == ERROR_INSUFFICIENT_BUFFER || lastError == ERROR_SUCCESS)))
	{
		ANKI_UTIL_LOGE("GetModuleFileNameA() failed");
		return Error::kFunctionFailed;
	}

	out.destroy();
	out.create('0', result);

	memcpy(&out[0], &buff[0], result);
	return Error::kNone;
}

} // end namespace anki
