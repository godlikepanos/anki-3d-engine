// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/String.h>
#include <AnKi/Util/Function.h>

namespace anki {

/// @addtogroup util_file
/// @{

/// Return true if a file exists
Bool fileExists(const CString& filename);

/// Get path extension.
void getFilepathExtension(const CString& filename, StringRaii& out);

/// Get path filename.
/// On path/to/file.ext return file.ext
void getFilepathFilename(const CString& filename, StringRaii& out);

/// Get base path.
/// On path/to/file.ext return path/to
void getParentFilepath(const CString& filename, StringRaii& out);

/// Return true if directory exists?
Bool directoryExists(const CString& dir);

/// @internal
Error walkDirectoryTreeInternal(const CString& dir, const Function<Error(const CString&, Bool)>& callback);

/// Walk a directory tree.
/// @param dir The dir to walk.
/// @param pool A mem pool for temp allocations.
/// @param func A lambda. See code example on how to use it.
/// Example:
/// @code
/// walkDirectoryTree("./path/to", pool, [&, this](CString path, Bool isDir) {
/// 	...
/// 	return Error::kNone;
/// });
/// @endcode
template<typename TMemPool, typename TFunc>
Error walkDirectoryTree(const CString& dir, TMemPool& pool, TFunc func)
{
	Function<Error(const CString&, Bool)> f(pool, func);
	const Error err = walkDirectoryTreeInternal(dir, f);
	f.destroy(pool);
	return err;
}

/// Equivalent to: rm -rf dir
/// @param dir The directory to remove.
/// @param pool A temp mem pool that this function requires.
Error removeDirectory(const CString& dir, BaseMemoryPool& pool);

/// Remove a file.
Error removeFile(const CString& filename);

/// Equivalent to: mkdir dir
Error createDirectory(const CString& dir);

/// Get the home directory.
Error getHomeDirectory(StringRaii& out);

/// Get the temp directory.
Error getTempDirectory(StringRaii& out);

/// Get the time the file was last modified.
Error getFileModificationTime(CString filename, U32& year, U32& month, U32& day, U32& hour, U32& min, U32& second);

/// Get the path+filename of the currently running executable.
Error getApplicationPath(StringRaii& path);
/// @}

} // end namespace anki
