#ifndef ANKI_UTIL_FILESYSTEM_H
#define ANKI_UTIL_FILESYSTEM_H

#include "anki/util/StringList.h"
#include <string>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup filesystem
/// @{

/// An abstraction over typical files and files in ziped archives.
class File
{
private:
	/// Internal filetype
	enum FileType
	{
		FT_NONE,
		FT_C, ///< C file
		FT_ZIP ///< Ziped file
	};

public:
	/// Open mode
	enum OpenFlag
	{
		OF_READ = 1 << 0,
		OF_WRITE = 1 << 1,
		OF_APPEND = 1 << 2,
		OF_BINARY = 1 << 3
	};

	/// Default constructor
	File()
		: openFlags(0), file(nullptr), fileType(FT_NONE)
	{}

	/// Closes the file if it's open
	~File();

	/// Open a file
	void open(const char* filename, U8 openMask);

	/// Read data from the file
	PtrSize read(void* buff, PtrSize size);

	/// Write data to the file
	PtrSize write(void* buff, PtrSize size);

public:
	U8 openFlags; ///< Mainly for assertions
	void* file; ///< A native type
	U8 fileType;
};

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
