// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
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

/// Get file extension
/// @param[in] filename The file to open
/// @return nullptr on failure and if the dot is the last character
CString getFileExtension(const CString& filename);

/// Return true if directory exists?
Bool directoryExists(const CString& dir);

/// Equivalent to: rm -rf dir
void removeDirectory(const CString& dir);

/// Equivalent to: mkdir dir
void createDirectory(const CString& dir);

/// Get the home directory.
/// Write the home directory to @a buff. The @a buffSize is the size of the 
/// @a buff. If the @buffSize is not enough the function will throw 
/// an exception.
String getHomeDirectory(HeapAllocator<U8>& alloc);

/// @}

} // end namespace anki

#endif

