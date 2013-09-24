#ifndef ANKI_UTIL_FILE_H
#define ANKI_UTIL_FILE_H

#include "anki/util/StringList.h"
#include <string>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup filesystem
/// @{

/// An abstraction over typical files and files in ziped archives. This class
/// can read from regular C files, zip files and on Android from the packed
/// asset files.
/// To identify the file:
/// - If the path contains ".ankizip" (eg /path/to/arch.ankizip/path/file.ext) 
///   it tries to open the archive and read the file from there.
/// - If the filename starts with '$' it will try to load a system specific 
///   file. For Android this is a file in the .apk
/// - If the above are false then try to load a regular C file
class File
{
private:
	/// Internal filetype
	enum FileType
	{
		FT_C = 1 << 0, ///< C file
		FT_ZIP = 1 << 1, ///< Ziped file
		FT_SPECIAL = 1 << 2 ///< For example file is located in the android apk 
	};

public:
	/// Open mode
	enum OpenFlag
	{
		OF_READ = 1 << 3,
		OF_WRITE = 1 << 4,
		OF_APPEND = OF_WRITE | (1 << 5),
		OF_BINARY = 1 << 6
	};

	/// The 2 available byte orders. Used in binary files.
	enum Endianness
	{
		E_LITTLE_ENDIAN = 1 << 7, ///< The default
		E_BIG_ENDIAN = 1 << 8
	};

	/// Passed to seek function
	enum SeekOrigin
	{
		SO_BEGINNING = SEEK_SET,
		SO_CURRENT = SEEK_CUR,
		SO_END = SEEK_END
	};

	/// Default constructor
	File()
		: file(nullptr), flags(0)
	{}

	/// Open file
	File(const char* filename, U16 openMask)
		: file(nullptr), flags(0)
	{
		open(filename, openMask);
	}

	/// Closes the file if it's open
	~File();

	/// Open a file
	/// @param[in] filename The file to open
	/// @param[in] openMask The open flags. It's a combination of OpenFlag and 
	///                     Endianness enums
	void open(const char* filename, U16 openMask);

	/// Return true if the file is oppen
	Bool isOpen() const
	{
		return file != nullptr;
	}

	/// Close the file
	void close();

	/// @name Read methods
	/// @{

	/// Read data from the file
	void read(void* buff, PtrSize size);

	/// Read all the contents of a text file
	void readAllText(std::string& out);

	/// Read all the contents of a text file and return the result in lines
	void readAllTextLines(StringList& lines);

	/// Read 32bit unsigned integer. Set the endianness if the file's 
	/// endianness is different from the machine's
	U32 readU32();

	/// Read 32bit float. Set the endianness if the file's endianness is 
	/// different from the machine's
	F32 readF32();
	/// @}

	/// @name Write methods
	/// @{

	/// Write data to the file
	void write(void* buff, PtrSize size);

	/// Write text
	void writeText(const char* format, ...);
	/// @}

	/// Set the position indicator to a new position
	/// @param offset Number of bytes to offset from origin
	/// @param origin Position used as reference for the offset
	void seek(PtrSize offset, SeekOrigin origin);

	/// @name Public statics
	/// @{

	/// Get file extension
	/// @param[in] filename The file to open
	/// @return nullptr on failure and if the dot is the last character
	static const char* getFileExtension(const char* filename);

	/// File exists?
	static Bool fileExists(const char* filename);

	/// @}

private:
	void* file; ///< A native type
	U16 flags; ///< All the flags. Initialy zero and set on open

	/// Get the current machine's endianness
	static Endianness getMachineEndianness();

	/// Get the type of the file
	FileType identifyFile(const char* filename,
		std::string* archive, std::string* filenameInArchive);

	/// Open a C file
	void openCFile(const char* filename, U16 flags);

	/// Open an archive and the file inside
	/// @param[in] archive The filename of the archive
	/// @param[in] archived The filename of the file inside the archive
	void openZipFile(const char* archive, const char* archived, U16 flags);

#if ANKI_OS == ANKI_OS_ANDROID
	/// Open an Android file
	void openAndFile(const char* filename, U16 flags);
#endif
};

/// Return true if directory exists?
extern Bool directoryExists(const char* dir);

/// Equivalent to: rm -rf dir
extern void removeDirectory(const char* dir);

/// Equivalent to: mkdir dir
extern void createDirectory(const char* dir);

/// @}
/// @}

} // end namespace anki

#endif
