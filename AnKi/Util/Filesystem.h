// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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
void getFilepathExtension(const CString& filename, StringAuto& out);

/// Get path filename.
/// On path/to/file.ext return file.ext
void getFilepathFilename(const CString& filename, StringAuto& out);

/// Get base path.
/// On path/to/file.ext return path/to
void getParentFilepath(const CString& filename, StringAuto& out);

/// Return true if directory exists?
Bool directoryExists(const CString& dir);

/// @internal
ANKI_USE_RESULT Error walkDirectoryTreeInternal(const CString& dir,
												const Function<Error(const CString&, Bool)>& callback);

/// Walk a directory tree.
/// @param dir The dir to walk.
/// @param alloc An allocator for temp allocations.
/// @param func A lambda. See code example on how to use it.
/// Example:
/// @code
/// walkDirectoryTree("./path/to", alloc, [&, this](CString path, Bool isDir) {
/// 	...
/// 	return Error::NONE;
/// });
/// @endcode
template<typename TFunc>
ANKI_USE_RESULT Error walkDirectoryTree(const CString& dir, GenericMemoryPoolAllocator<U8> alloc, TFunc func)
{
	Function<Error(const CString&, Bool)> f(alloc, func);
	const Error err = walkDirectoryTreeInternal(dir, f);
	f.destroy(alloc);
	return err;
}

/// Equivalent to: rm -rf dir
/// @param dir The directory to remove.
/// @param alloc A temp allocator that this function requires.
ANKI_USE_RESULT Error removeDirectory(const CString& dir, GenericMemoryPoolAllocator<U8> alloc);

/// Equivalent to: mkdir dir
ANKI_USE_RESULT Error createDirectory(const CString& dir);

/// Get the home directory.
ANKI_USE_RESULT Error getHomeDirectory(StringAuto& out);

/// Get the temp directory.
ANKI_USE_RESULT Error getTempDirectory(StringAuto& out);

/// Get the time the file was last modified.
ANKI_USE_RESULT Error getFileModificationTime(CString filename, U32& year, U32& month, U32& day, U32& hour, U32& min,
											  U32& second);
/// @}

} // end namespace anki
