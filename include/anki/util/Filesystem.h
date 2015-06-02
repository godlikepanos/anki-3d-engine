// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_DIRECTORY_H
#define ANKI_UTIL_DIRECTORY_H

#include "anki/util/String.h"

namespace anki {

/// @addtogroup util_file
/// @{

/// Return true if a file exists
Bool fileExists(const CString& filename);

/// Get file extension.
void getFileExtension(
	const CString& filename, GenericMemoryPoolAllocator<U8> alloc, String& out);

/// Return true if directory exists?
Bool directoryExists(const CString& dir);

/// Equivalent to: rm -rf dir
ANKI_USE_RESULT Error removeDirectory(const CString& dir);

/// Equivalent to: mkdir dir
ANKI_USE_RESULT Error createDirectory(const CString& dir);

/// Get the home directory.
/// Write the home directory to @a buff. The @a buffSize is the size of the
/// @a buff. If the @buffSize is not enough the function will throw
/// an exception.
ANKI_USE_RESULT Error getHomeDirectory(
	GenericMemoryPoolAllocator<U8> alloc, String& out);
/// @}

} // end namespace anki

#endif

