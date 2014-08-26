// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/File.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include <fstream>
#include <cstring>
#include <cstdarg>
#include <contrib/minizip/unzip.h>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#	include <android/asset_manager.h>
#endif
#if ANKI_POSIX
#	include <sys/stat.h>
#	include <sys/types.h>
#	include <dirent.h>
#	include <cerrno>
#endif

namespace anki {

//==============================================================================
// File                                                                        =
//==============================================================================

#if ANKI_OS == ANKI_OS_ANDROID
extern android_app* gAndroidApp;
#endif

//==============================================================================
File::~File()
{
	close();
}

//==============================================================================
void File::open(const char* filename, OpenFlag flags)
{
	ANKI_ASSERT(filename);
	ANKI_ASSERT(strlen(filename) > 0);
	ANKI_ASSERT(m_file == nullptr && m_flags == OpenFlag::NONE 
		&& m_type == Type::NONE);

	// Only these flags are accepted
	ANKI_ASSERT((flags & (OpenFlag::READ | OpenFlag::WRITE | OpenFlag::APPEND 
		| OpenFlag::BINARY | OpenFlag::LITTLE_ENDIAN 
		| OpenFlag::BIG_ENDIAN)) != OpenFlag::NONE);

	// Cannot be both
	ANKI_ASSERT((flags & OpenFlag::READ) != (flags & OpenFlag::WRITE));

	//
	// Determine the file type and open it
	//

	String archive, filenameInArchive;
	Type ft = identifyFile(filename, archive, filenameInArchive);

	switch(ft)
	{
#if ANKI_OS == ANKI_OS_ANDROID
	case Type::SPECIAL:
		openAndroidFile(filename, flags);
		break;
#endif
	case Type::C:
		openCFile(filename, flags);
		break;
	case Type::ZIP:
		openZipFile(archive.c_str(), filenameInArchive.c_str(), flags);
		break;
	default:
		ANKI_ASSERT(0);
	}

	//
	// Set endianess
	//

	// If the open() DIDN'T provided us the file endianess 
	if((flags & (OpenFlag::BIG_ENDIAN | OpenFlag::LITTLE_ENDIAN)) 
		== OpenFlag::NONE)
	{
		// Set the machine's endianness
		m_flags = m_flags | getMachineEndianness();
	}
	else
	{
		// Else just make sure that only one of the flags is set
		ANKI_ASSERT((flags & OpenFlag::BIG_ENDIAN) 
			!= (flags & OpenFlag::LITTLE_ENDIAN));
	}
}

//==============================================================================
void File::openCFile(const char* filename, OpenFlag flags)
{
	const char* openMode;

	if((flags & OpenFlag::READ) != OpenFlag::NONE)
	{
		openMode = "r";
	}
	else if((flags & OpenFlag::APPEND) == OpenFlag::APPEND)
	{
		openMode = "a+";
	}
	else if((flags & OpenFlag::WRITE) != OpenFlag::NONE)
	{
		openMode = "w";
	}
	else
	{
		ANKI_ASSERT(0);
		openMode = nullptr;
	}

	// Open
	m_file = (FILE*)fopen(filename, openMode);
	if(m_file == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to open file: %s", filename);
	}

	m_flags = flags;
	m_type = Type::C;
}

//==============================================================================
void File::openZipFile(
	const char* archive, const char* archived, OpenFlag flags)
{
	if((flags & OpenFlag::WRITE) != OpenFlag::NONE)
	{
		throw ANKI_EXCEPTION("Cannot write inside archives");
	}

	if((flags & OpenFlag::READ) == OpenFlag::NONE)
	{
		throw ANKI_EXCEPTION("Missing OpenFlag::READ flag");
	}

	// Open archive
	unzFile zfile = unzOpen(archive);
	if(zfile == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to open archive: %s", archive);
	}

	// Locate archived
	const int caseSensitive = 1;
	if(unzLocateFile(zfile, archived, caseSensitive) != UNZ_OK)
	{
		unzClose(zfile);
		throw ANKI_EXCEPTION("Failed to locate file in archive: %s", archived);
	}

	// Open file
	if(unzOpenCurrentFile(zfile) != UNZ_OK)
	{
		unzClose(zfile);
		throw ANKI_EXCEPTION("unzOpenCurrentFile failed: %s", archived);
	}

	static_assert(sizeof(m_file) == sizeof(zfile), "See file");
	m_file = (void*)zfile;
	m_flags = flags;
	m_type = Type::ZIP;

	// Get size just in case
	unz_file_info zinfo;
	zinfo.uncompressed_size = 0;
	unzGetCurrentFileInfo(zfile, &zinfo, nullptr, 0, nullptr, 0, nullptr, 0);
	m_size = zinfo.uncompressed_size;
	ANKI_ASSERT(m_size != 0);
}

//==============================================================================
#if ANKI_OS == ANKI_OS_ANDROID
void File::openAndroidFile(const char* filename, OpenFlag flags)
{
	if((flags & OpenFlag::WRITE) != OpenFlag::NONE)
	{
		throw ANKI_EXCEPTION("Cannot write inside archives");
	}

	if((flags & OpenFlag::READ) != OpenFlag::NONE)
	{
		throw ANKI_EXCEPTION("Missing OpenFlag::READ flag");
	}

	// Open file
	ANKI_ASSERT(gAndroidApp != nullptr && gAndroidApp->activity 
		&& gAndroidApp->activity->assetManager);

	m_file = AAssetManager_open(
		gAndroidApp->activity->assetManager, 
		filename + 1, 
		AASSET_MODE_STREAMING);

	if(m_file == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to open file: %s", filename);
	}

	m_flags = flags;
	m_type = Type::SPECIAL;
}
#endif

//==============================================================================
void File::close()
{
	if(m_file)
	{
		if(m_type == Type::C)
		{
			fclose((FILE*)m_file);
		}
		else if(m_type == Type::ZIP)
		{
			unzClose(m_file);
		}
#if ANKI_OS == ANKI_OS_ANDROID
		else if(m_type == Type::SPECIAL)
		{
			AAsset_close((AAsset*)m_file);
		}
#endif
		else
		{
			ANKI_ASSERT(0);
		}
	}

	m_file = nullptr;
	m_flags = OpenFlag::NONE;
	m_type = Type::NONE;
}

//==============================================================================
void File::flush()
{
	ANKI_ASSERT(m_file);

	if((m_flags & OpenFlag::WRITE) != OpenFlag::NONE)
	{
		if(m_type == Type::C)
		{
			I err = fflush((FILE*)m_file);
			if(err)
			{
				throw ANKI_EXCEPTION("fflush() failed");
			}
		}
		else if(m_type == Type::ZIP
#if ANKI_OS == ANKI_OS_ANDROID
			|| m_type == Type::SPECIAL
#endif
			)
		{
			ANKI_ASSERT(0 && "Cannot have write these file types");
		}
		else
		{
			ANKI_ASSERT(0);
		}
	}
}

//==============================================================================
void File::read(void* buff, PtrSize size)
{
	ANKI_ASSERT(buff);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(m_file);
	ANKI_ASSERT((m_flags & OpenFlag::READ) != OpenFlag::NONE);

	I64 readSize = 0;

	if(m_type == Type::C)
	{
		readSize = fread(buff, 1, size, (FILE*)m_file);
	}
	else if(m_type == Type::ZIP)
	{
		readSize = unzReadCurrentFile(m_file, buff, size);
	}
#if ANKI_OS == ANKI_OS_ANDROID
	else if(m_type == Type::SPECIAL)
	{
		readSize = AAsset_read((AAsset*)m_file, buff, size);
	}
#endif
	else
	{
		ANKI_ASSERT(0);
	}

	if((I64)size != readSize)
	{
		throw ANKI_EXCEPTION("File read failed");
	}
}

//==============================================================================
PtrSize File::getSize()
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != OpenFlag::NONE);
	PtrSize out = 0;

	if(m_type == Type::C)
	{
		// Get file size
		fseek((FILE*)m_file, 0, SEEK_END);
		I64 size = ftell((FILE*)m_file);
		if(size < 1)
		{
			throw ANKI_EXCEPTION("ftell() failed");
		}
		rewind((FILE*)m_file);

		out = size;
	}
	else if(m_type == Type::ZIP)
	{
		ANKI_ASSERT(m_size != 0);
		out = m_size;
	}
#if ANKI_OS == ANKI_OS_ANDROID
	else if(m_type == Type::SPECIAL)
	{
		out = AAsset_getLength((AAsset*)m_file);
	}
#endif
	else
	{
		ANKI_ASSERT(0);
	}

	ANKI_ASSERT(out != 0);
	return out;
}

//==============================================================================
U32 File::readU32()
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != OpenFlag::NONE);
	ANKI_ASSERT((m_flags & OpenFlag::READ) != OpenFlag::NONE);
	ANKI_ASSERT((m_flags & OpenFlag::BINARY) != OpenFlag::NONE 
		&& "Should be binary file");
	ANKI_ASSERT(
		(m_flags & OpenFlag::BIG_ENDIAN) != (m_flags & OpenFlag::LITTLE_ENDIAN) 
		&& "One of those 2 should be active");

	U32 out;
	read(&out, sizeof(out));
	
	// Copy it
	OpenFlag machineEndianness = getMachineEndianness();
	OpenFlag fileEndianness = 
		((m_flags & OpenFlag::BIG_ENDIAN) != OpenFlag::NONE)
		? OpenFlag::BIG_ENDIAN 
		: OpenFlag::LITTLE_ENDIAN;

	if(machineEndianness == fileEndianness)
	{
		// Same endianness between the file and the machine. Do nothing
	}
	else if(machineEndianness == OpenFlag::BIG_ENDIAN 
		&& fileEndianness == OpenFlag::LITTLE_ENDIAN)
	{
		U16* c = (U16*)&out;
		out = (c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24));
	}
	else
	{
		U16* c = (U16*)&out;
		out = ((c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3]);
	}

	return out;
}

//==============================================================================
F32 File::readF32()
{
	F32 out;
	U32 integer = readU32();

	memcpy(&out, &integer, sizeof(F32));

	return out;
}

//==============================================================================
void File::write(void* buff, PtrSize size)
{
	ANKI_ASSERT(buff);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(m_file);
	ANKI_ASSERT((m_flags & OpenFlag::WRITE) != OpenFlag::NONE);

	if(m_type == Type::C)
	{
		PtrSize writeSize = 0;
		writeSize = fwrite(buff, 1, size, (FILE*)m_file);

		if(writeSize != size)
		{
			throw ANKI_EXCEPTION("Failed to write on file");
		}
	}
	else if(m_type == Type::ZIP
#if ANKI_OS == ANKI_OS_ANDROID
		|| m_type == Type::SPECIAL
#endif
		)
	{
		throw ANKI_EXCEPTION("Writting to archives is not supported");
	}
	else
	{
		ANKI_ASSERT(0);
	}
}

//==============================================================================
void File::writeText(const char* format, ...)
{
	ANKI_ASSERT(format);
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != OpenFlag::NONE);
	ANKI_ASSERT((m_flags & OpenFlag::WRITE) != OpenFlag::NONE);
	ANKI_ASSERT((m_flags & OpenFlag::BINARY) == OpenFlag::NONE);

	va_list args;
	va_start(args, format);

	if(m_type == Type::C)
	{
		vfprintf((FILE*)m_file, format, args);
	}
	else if(m_type == Type::ZIP
#if ANKI_OS == ANKI_OS_ANDROID
		|| m_type == Type::SPECIAL
#endif
		)
	{
		throw ANKI_EXCEPTION("Writting to archives is not supported");
	}
	else
	{
		ANKI_ASSERT(0);
	}

	va_end(args);
}

//==============================================================================
void File::seek(PtrSize offset, SeekOrigin origin)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != OpenFlag::NONE);

	if(m_type == Type::C)
	{
		if(fseek((FILE*)m_file, offset, (I)origin) != 0)
		{
			throw ANKI_EXCEPTION("fseek() failed");
		}
	}
	else if(m_type == Type::ZIP)
	{
		// Rewind if needed
		if(origin == SeekOrigin::BEGINNING)
		{
			if(unzCloseCurrentFile(m_file)
				|| unzOpenCurrentFile(m_file))
			{
				throw ANKI_EXCEPTION("Rewind failed");
			}
		}

		// Move forward by reading dummy data
		char buff[256];
		while(offset != 0)
		{
			PtrSize toRead = std::min(offset, sizeof(buff));
			read(buff, toRead);
			offset -= toRead;
		}
	}
#if ANKI_OS == ANKI_OS_ANDROID
	else if(m_type == Type::SPECIAL)
	{
		if(AAsset_seek((AAsset*)file, offset, origin) == (off_t)-1)
		{
			throw ANKI_EXCEPTION("AAsset_seek() failed");
		}
	}
#endif
	else
	{
		ANKI_ASSERT(0);
	}
}

//==============================================================================
File::Type File::identifyFile(const char* filename, 
	String& archive, String& filenameInArchive)
{
	ANKI_ASSERT(filename);
	ANKI_ASSERT(strlen(filename) > 1);

#if ANKI_OS == ANKI_OS_ANDROID
	if(filename[0] == '$')
	{
		return Type::SPECIAL;
	}
	else
#endif
	{
		static const char aext[] = {".ankizip"};
		const char* ptrToArchiveExt = strstr(filename, aext);

		if(ptrToArchiveExt == nullptr)
		{
			// It's a C file
			return Type::C;
		}
		else
		{
			// Maybe it's a file in a zipped archive

			PtrSize fnameLen = strlen(filename);
			const PtrSize extLen = sizeof(aext) - 1;
			PtrSize archLen = (PtrSize)(ptrToArchiveExt - filename) + extLen;

			if(archLen + 1 >= fnameLen)
			{
				throw ANKI_EXCEPTION("Too sort archived filename");
			}

			archive = String(filename, archLen);

			if(directoryExists(archive.c_str()))
			{
				// It's a directory so failback to C file
				return Type::C;
			}
			else
			{
				// It's a ziped file

				filenameInArchive = String(
					filename + archLen + 1, fnameLen - archLen);

				return Type::ZIP;
			}
		}
	}

	ANKI_ASSERT(0);
	return Type::C;
}

//==============================================================================
File::OpenFlag File::getMachineEndianness()
{
	I32 num = 1;
	if(*(char*)&num == 1)
	{
		return OpenFlag::LITTLE_ENDIAN;
	}
	else
	{
		return OpenFlag::BIG_ENDIAN;
	}
}

//==============================================================================
const char* File::getFileExtension(const char* filename)
{
	ANKI_ASSERT(filename);
	const char* pc = strrchr(filename, '.');

	if(pc == nullptr)
	{
		return nullptr;
	}

	++pc;
	return (*pc == '\0') ? nullptr : pc;
}

} // end namespace anki
