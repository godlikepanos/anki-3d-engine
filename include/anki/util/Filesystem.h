#ifndef ANKI_UTIL_FILESYSTEM_H
#define ANKI_UTIL_FILESYSTEM_H

#include "anki/util/StringList.h"
#include <string>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup filesystem
/// @{

/// Get file extension
/// @param[in] filename The file to open
/// @return nullptr on failure and if the dot is the last character
extern const char* getFileExtension(const char* filename);

/// Open a text file and return its contents into a string
extern std::string readFile(const char* filename);

/// Open a text file and return its lines into a string vector
extern StringList readFileLines(const char* filename);

/// File exists?
extern bool fileExists(const char* filename);

/// Directory exists?
extern bool directoryExists(const char* filename);

/// rm -rf
extern void removeDirectory(const char* dir);

/// mkdir
extern void createDirectory(const char* dir);

/// Convert a POSIX path to a platform native path. It actually replaces /
/// with \ in windows
extern void toNativePath(const char* path);

/// @}
/// @}

} // end namespace anki

#endif
