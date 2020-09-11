// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/String.h>
#include <anki/util/Enum.h>
#include <anki/util/NonCopyable.h>
#include <cstdio>

namespace anki
{

/// @addtogroup util_file
/// @{

/// Open mode
/// @memberof File
enum class FileOpenFlag : U8
{
	NONE = 0,
	READ = 1 << 0,
	WRITE = 1 << 1,
	APPEND = WRITE | (1 << 3),
	BINARY = 1 << 4,
	ENDIAN_LITTLE = 1 << 5, ///< The default
	ENDIAN_BIG = 1 << 6
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(FileOpenFlag)

/// Passed to seek function
/// @memberof File
enum class FileSeekOrigin
{
	BEGINNING = SEEK_SET,
	CURRENT = SEEK_CUR,
	END = SEEK_END
};

/// An abstraction over typical files and files in ziped archives. This class can read from regular C files, zip files
/// and on Android from the packed asset files.
/// To identify the file:
/// - If the filename starts with '$' it will try to load a system specific file. For Android this is a file in the .apk
/// - If the above are false then try to load a regular C file
class File : public NonCopyable
{
public:
	/// Default constructor
	File() = default;

	/// Move
	File(File&& b)
	{
		*this = std::move(b);
	}

	/// Closes the file if it's open
	~File();

	/// Move
	File& operator=(File&& b);

	/// Open a file.
	/// @param[in] filename The file to open
	/// @param[in] openMask The open flags. It's a combination of FileOpenFlag enum
	ANKI_USE_RESULT Error open(const CString& filename, FileOpenFlag openMask);

	/// Return true if the file is oppen
	Bool isOpen() const
	{
		return m_file != nullptr;
	}

	/// Close the file
	void close();

	/// Flush pending operations
	ANKI_USE_RESULT Error flush();

	/// Read data from the file
	ANKI_USE_RESULT Error read(void* buff, PtrSize size);

	/// Read all the contents of a text file
	/// If the file is not rewined it will probably fail
	ANKI_USE_RESULT Error readAllText(GenericMemoryPoolAllocator<U8> alloc, String& out);

	/// Read all the contents of a text file. If the file is not rewined it will probably fail.
	ANKI_USE_RESULT Error readAllText(StringAuto& out);

	/// Read 32bit unsigned integer. Set the endianness if the file's endianness is different from the machine's.
	ANKI_USE_RESULT Error readU32(U32& u);

	/// Read 32bit float. Set the endianness if the file's endianness is different from the machine's.
	ANKI_USE_RESULT Error readF32(F32& f);

	/// Write data to the file
	ANKI_USE_RESULT Error write(const void* buff, PtrSize size);

	/// Write formated text
	ANKI_USE_RESULT Error writeText(CString format, ...);

	/// Set the position indicator to a new position.
	/// @param offset Number of bytes to offset from origin
	/// @param origin Position used as reference for the offset
	ANKI_USE_RESULT Error seek(PtrSize offset, FileSeekOrigin origin);

	/// Return the position indicator inside the file.
	PtrSize tell();

	/// The the size of the file.
	PtrSize getSize() const;

private:
	/// Internal filetype
	enum class Type : U8
	{
		NONE = 0,
		C, ///< C file
		SPECIAL ///< For example file is located in the android apk
	};

	void* m_file = nullptr; ///< A native file type
	Type m_type = Type::NONE;
	FileOpenFlag m_flags = FileOpenFlag::NONE; ///< All the flags. Set on open
	U32 m_size = 0;

	/// Get the current machine's endianness
	static FileOpenFlag getMachineEndianness();

	/// Get the type of the file
	ANKI_USE_RESULT Error identifyFile(const CString& filename, char* archiveFilename, PtrSize archiveFilenameSize,
									   CString& filenameInArchive, Type& type);

	/// Open a C file
	ANKI_USE_RESULT Error openCFile(const CString& filename, FileOpenFlag flags);

#if ANKI_OS_ANDROID
	/// Open an Android file
	ANKI_USE_RESULT Error openAndroidFile(const CString& filename, FileOpenFlag flags);
#endif

	void zero()
	{
		m_file = nullptr;
		m_type = Type::NONE;
		m_flags = FileOpenFlag::NONE;
		m_size = 0;
	}
};
/// @}

} // end namespace anki
