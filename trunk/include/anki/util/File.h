// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_FILE_H
#define ANKI_UTIL_FILE_H

#include "anki/util/StringList.h"
#include <string>

namespace anki {

// Undefine some flags because someone else is polluting the global namespace
#if defined(LITTLE_ENDIAN)
#	undef LITTLE_ENDIAN
#endif

#if defined(BIG_ENDIAN)
#	undef BIG_ENDIAN
#endif

/// @addtogroup util_file
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
public:
	/// Open mode
	enum class OpenFlag: U8
	{
		NONE = 0,
		READ = 1 << 0,
		WRITE = 1 << 1,
		APPEND = WRITE | (1 << 3),
		BINARY = 1 << 4,
		LITTLE_ENDIAN = 1 << 5, ///< The default
		BIG_ENDIAN = 1 << 6
	};

	friend OpenFlag operator|(OpenFlag a, OpenFlag b)
	{
		typedef std::underlying_type<OpenFlag>::type Int;
		return static_cast<OpenFlag>(static_cast<Int>(a) | static_cast<Int>(b));
	}

	friend OpenFlag operator&(OpenFlag a, OpenFlag b)
	{
		typedef std::underlying_type<OpenFlag>::type Int;
		return static_cast<OpenFlag>(static_cast<Int>(a) & static_cast<Int>(b));
	}

	/// Passed to seek function
	enum class SeekOrigin
	{
		BEGINNING = SEEK_SET,
		CURRENT = SEEK_CUR,
		END = SEEK_END
	};

	/// Default constructor
	File() = default;

	/// Open file
	File(const char* filename, OpenFlag openMask)
	{
		open(filename, openMask);
	}

	/// Closes the file if it's open
	~File();

	/// Open a file
	/// @param[in] filename The file to open
	/// @param[in] openMask The open flags. It's a combination of OpenFlag enum
	void open(const char* filename, OpenFlag openMask);

	/// Return true if the file is oppen
	Bool isOpen() const
	{
		return m_file != nullptr;
	}

	/// Close the file
	void close();

	/// Flush pending operations
	void flush();

	/// @name Read methods
	/// @{

	/// Read data from the file
	void read(void* buff, PtrSize size);

	/// Read all the contents of a text file
	/// If the file is not rewined it will probably fail
	template<typename TContainer>
	void readAllText(TContainer& out)
	{
		PtrSize size = getSize();
		out.resize(size + 1);
		read(&out[0], size);
		out[size] = '\0';
	}

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

	/// Write formated text
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
	/// Internal filetype
	enum class Type: U8
	{
		NONE = 0,
		C, ///< C file
		ZIP, ///< Ziped file
		SPECIAL ///< For example file is located in the android apk 
	};

	void* m_file = nullptr; ///< A native file type
	Type m_type = Type::NONE;
	OpenFlag m_flags = OpenFlag::NONE; ///< All the flags. Set on open
	U16 m_size = 0;

	/// Get the current machine's endianness
	static OpenFlag getMachineEndianness();

	/// Get the type of the file
	Type identifyFile(const char* filename,
		String& archive, String& filenameInArchive);

	/// Open a C file
	void openCFile(const char* filename, OpenFlag flags);

	/// Open an archive and the file inside
	/// @param[in] archive The filename of the archive
	/// @param[in] archived The filename of the file inside the archive
	void openZipFile(const char* archive, const char* archived, 
		OpenFlag flags);

#if ANKI_OS == ANKI_OS_ANDROID
	/// Open an Android file
	void openAndroidFile(const char* filename, OpenFlag flags);
#endif

	/// The file should be open
	PtrSize getSize();
};

/// Return true if directory exists?
extern Bool directoryExists(const char* dir);

/// Equivalent to: rm -rf dir
extern void removeDirectory(const char* dir);

/// Equivalent to: mkdir dir
extern void createDirectory(const char* dir);

/// Get the home directory
/// Write the home directory to @a buff. The @a buffSize is the size of the 
/// @a buff. If the @buffSize is not enough the function will throw exception
extern void getHomeDirectory(U32 buffSize, char* buff);

/// @}

} // end namespace anki

#endif
