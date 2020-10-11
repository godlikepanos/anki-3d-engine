// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/String.h>

namespace anki
{

/// @addtogroup util_file
/// @{

/// Return true if a file exists
Bool fileExists(const CString& filename);

/// Get path extension.
void getFilepathExtension(const CString& filename, StringAuto& out);

/// Get path filename.
/// On path/to/file.ext return file.ext
void getFilepathFilename(const CString& filename, StringAuto& out);

/// Return true if directory exists?
Bool directoryExists(const CString& dir);

/// Callback for the @ref walkDirectoryTree.
/// @param filename The file or directory name.
/// @param userData User data passed to walkDirectoryTree.
/// @param isDirectory True if it's directory, false if it's regular file.
using WalkDirectoryTreeCallback = Error (*)(const CString& filename, void* userData, Bool isDirectory);

/// Walk a directory and it's subdirectories. Will walk and list all directories and files of a directory.
ANKI_USE_RESULT Error walkDirectoryTree(const CString& dir, void* userData, WalkDirectoryTreeCallback callback);

/// Equivalent to: rm -rf dir
/// @param dir The directory to remove.
/// @param alloc A temp allocator that this function requires.
ANKI_USE_RESULT Error removeDirectory(const CString& dir, GenericMemoryPoolAllocator<U8> alloc);

/// Equivalent to: mkdir dir
ANKI_USE_RESULT Error createDirectory(const CString& dir);

/// Get the home directory.
/// Write the home directory to @a buff. The @a buffSize is the size of the @a buff. If the @buffSize is not enough the
/// function will throw an exception.
ANKI_USE_RESULT Error getHomeDirectory(StringAuto& out);

/// Get the time the file was last modified.
ANKI_USE_RESULT Error getFileModificationTime(CString filename, U32& year, U32& month, U32& day, U32& hour, U32& min,
											  U32& second);
/// @}

} // end namespace anki
