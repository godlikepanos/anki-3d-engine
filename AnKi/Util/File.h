// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/String.h>
#include <AnKi/Util/Enum.h>
#include <cstdio>

namespace anki {

/// @addtogroup util_file
/// @{

/// Open mode
/// @memberof File
enum class FileOpenFlag : U8
{
	kNone = 0,
	kRead = 1 << 0,
	kWrite = 1 << 1,
	kAppend = kWrite | (1 << 3),
	kBinary = 1 << 4,
	kLittleEndian = 1 << 5, ///< The default
	kBigEndian = 1 << 6,
	kSpecial = 1 << 7, ///< Android package file.
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(FileOpenFlag)

/// Passed to seek function
/// @memberof File
enum class FileSeekOrigin
{
	kBeginning = SEEK_SET,
	kCurrent = SEEK_CUR,
	kEnd = SEEK_END
};

/// An abstraction over typical files and files in ziped archives. This class can read from regular C files, zip files
/// and on Android from the packed asset files.
/// To identify the file:
/// - If the filename starts with '$' it will try to load a system specific file. For Android this is a file in the .apk
/// - If the above are false then try to load a regular C file
class File
{
public:
	/// Default constructor
	File() = default;

	File(const File&) = delete; // Non-copyable

	File& operator=(const File&) = delete; // Non-copyable

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
	Error open(const CString& filename, FileOpenFlag openMask);

	/// Return true if the file is oppen
	Bool isOpen() const
	{
		return m_file != nullptr;
	}

	/// Close the file
	void close();

	/// Flush pending operations
	Error flush();

	/// Read data from the file
	Error read(void* buff, PtrSize size);

	/// Read all the contents of a text file
	/// If the file is not rewined it will probably fail
	Error readAllText(BaseMemoryPool& pool, String& out);

	/// Read all the contents of a text file. If the file is not rewined it will probably fail.
	Error readAllText(StringRaii& out);

	/// Read 32bit unsigned integer. Set the endianness if the file's endianness is different from the machine's.
	Error readU32(U32& u);

	/// Read 32bit float. Set the endianness if the file's endianness is different from the machine's.
	Error readF32(F32& f);

	/// Write data to the file
	Error write(const void* buff, PtrSize size);

	/// Write formated text
	ANKI_CHECK_FORMAT(1, 2)
	Error writeTextf(const Char* format, ...);

	/// Write plain text.
	Error writeText(CString text);

	/// Set the position indicator to a new position.
	/// @param offset Number of bytes to offset from origin
	/// @param origin Position used as reference for the offset
	Error seek(PtrSize offset, FileSeekOrigin origin);

	/// Return the position indicator inside the file.
	PtrSize tell();

	/// The the size of the file.
	PtrSize getSize() const
	{
		ANKI_ASSERT(!(m_flags & FileOpenFlag::kWrite));
		return m_size;
	}

private:
	void* m_file = nullptr; ///< A native file type
	FileOpenFlag m_flags = FileOpenFlag::kNone; ///< All the flags. Set on open
	PtrSize m_size = 0;

	/// Get the current machine's endianness
	static FileOpenFlag getMachineEndianness();

	/// Open a C file
	Error openCFile(const CString& filename, FileOpenFlag flags);

#if ANKI_OS_ANDROID
	/// Open an Android file
	Error openAndroidFile(const CString& filename, FileOpenFlag flags);
#endif

	void zero()
	{
		m_file = nullptr;
		m_flags = FileOpenFlag::kNone;
		m_size = 0;
	}
};
/// @}

} // end namespace anki
