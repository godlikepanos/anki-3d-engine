// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/File.h"
#include "anki/util/Filesystem.h"
#include "anki/util/Logger.h"
#include "anki/util/Assert.h"
#include <cstring>
#include <cstdarg>
#include <contrib/minizip/unzip.h>

namespace anki {

//==============================================================================
// Misc

#if ANKI_OS == ANKI_OS_ANDROID
extern android_app* gAndroidApp;

#define ANKI_AFILE reinterpret_cast<AAsset*>(m_file)
#endif

#define ANKI_CFILE reinterpret_cast<FILE*>(m_file)
#define ANKI_ZFILE reinterpret_cast<void*>(m_file)

//==============================================================================
File& File::operator=(File&& b)
{
	close();

	if(b.m_file != nullptr)
	{
		m_file = b.m_file;
		m_type = b.m_type;
		m_flags = b.m_flags;
		m_size = b.m_size;
	}

	b.zero();

	return *this;
}

//==============================================================================
File::~File()
{
	close();
}

//==============================================================================
Error File::open(const CString& filename, OpenFlag flags)
{
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

	Array<char, 512> archive;
	CString filenameInArchive;
	Type ft;
	Error err = identifyFile(
		filename, &archive[0], archive.getSize(), filenameInArchive, ft);

	if(!err)
	{
		switch(ft)
		{
#if ANKI_OS == ANKI_OS_ANDROID
		case Type::SPECIAL:
			err = openAndroidFile(filename.get(), flags);
			break;
#endif
		case Type::C:
			err = openCFile(filename, flags);
			break;
		case Type::ZIP:
			err = openZipFile(CString(&archive[0]), filenameInArchive, flags);
			break;
		default:
			ANKI_ASSERT(0);
		}
	}

	//
	// Set endianess
	//

	if(!err)
	{
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

	return err;
}

//==============================================================================
Error File::openCFile(const CString& filename, OpenFlag flags)
{
	Error err = ErrorCode::NONE;
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
	m_file = reinterpret_cast<FILE*>(fopen(filename.get(), openMode));
	if(m_file == nullptr)
	{
		ANKI_LOGE("Failed to open file %s", &filename[0]);
		err = ErrorCode::FILE_ACCESS;
	}
	else
	{
		m_flags = flags;
		m_type = Type::C;
	}

	return err;
}

//==============================================================================
Error File::openZipFile(
	const CString& archive, const CString& archived, OpenFlag flags)
{
	if((flags & OpenFlag::WRITE) != OpenFlag::NONE)
	{
		// Cannot write inside archives
		return ErrorCode::FILE_ACCESS;
	}

	if((flags & OpenFlag::READ) == OpenFlag::NONE)
	{
		// Missing OpenFlag::READ flag
		return ErrorCode::FILE_ACCESS;
	}

	// Open archive
	unzFile zfile = unzOpen(&archive[0]);
	if(zfile == nullptr)
	{
		// Failed to open archive
		return ErrorCode::FILE_ACCESS;
	}

	// Locate archived
	const int caseSensitive = 1;
	if(unzLocateFile(zfile, &archived[0], caseSensitive) != UNZ_OK)
	{
		unzClose(zfile);
		// Failed to locate file in archive
		return ErrorCode::FILE_ACCESS;
	}

	// Open file
	if(unzOpenCurrentFile(zfile) != UNZ_OK)
	{
		unzClose(zfile);
		// unzOpenCurrentFile failed
		return ErrorCode::FILE_ACCESS;
	}

	static_assert(sizeof(m_file) == sizeof(zfile), "See file");
	m_file = reinterpret_cast<void*>(zfile);
	m_flags = flags;
	m_type = Type::ZIP;

	// Get size just in case
	unz_file_info zinfo;
	zinfo.uncompressed_size = 0;
	unzGetCurrentFileInfo(zfile, &zinfo, nullptr, 0, nullptr, 0, nullptr, 0);
	m_size = zinfo.uncompressed_size;
	ANKI_ASSERT(m_size != 0);

	return ErrorCode::NONE;
}

//==============================================================================
#if ANKI_OS == ANKI_OS_ANDROID
Error File::openAndroidFile(const CString& filename, OpenFlag flags)
{
	if((flags & OpenFlag::WRITE) != OpenFlag::NONE)
	{
		// Cannot write inside archives
		return ErrorCode::FILE_ACCESS;
	}

	if((flags & OpenFlag::READ) != OpenFlag::NONE)
	{
		// Missing OpenFlag::READ flag"
		return ErrorCode::FILE_ACCESS;
	}

	// Open file
	ANKI_ASSERT(gAndroidApp != nullptr && gAndroidApp->activity 
		&& gAndroidApp->activity->assetManager);

	m_file = AAssetManager_open(
		gAndroidApp->activity->assetManager, 
		&filename[0] + 1, 
		AASSET_MODE_STREAMING);

	if(m_file == nullptr)
	{
		return ErrorCode::FILE_ACCESS;
	}

	m_flags = flags;
	m_type = Type::SPECIAL;
	
	return ErrorCode::NONE;
}
#endif

//==============================================================================
void File::close()
{
	if(m_file)
	{
		if(m_type == Type::C)
		{
			fclose(ANKI_CFILE);
		}
		else if(m_type == Type::ZIP)
		{
			unzClose(ANKI_ZFILE);
		}
#if ANKI_OS == ANKI_OS_ANDROID
		else if(m_type == Type::SPECIAL)
		{
			AAsset_close(ANKI_AFILE);
		}
#endif
		else
		{
			ANKI_ASSERT(0);
		}
	}

	zero();
}

//==============================================================================
Error File::flush()
{
	ANKI_ASSERT(m_file);
	Error err = ErrorCode::NONE;

	if((m_flags & OpenFlag::WRITE) != OpenFlag::NONE)
	{
		if(m_type == Type::C)
		{
			I ierr = fflush(ANKI_CFILE);
			if(ierr)
			{
				ANKI_LOGE("fflush() failed");
				err = ErrorCode::FUNCTION_FAILED;
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

	return err;
}

//==============================================================================
Error File::read(void* buff, PtrSize size)
{
	ANKI_ASSERT(buff);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(m_file);
	ANKI_ASSERT((m_flags & OpenFlag::READ) != OpenFlag::NONE);

	I64 readSize = 0;

	if(m_type == Type::C)
	{
		readSize = fread(buff, 1, size, ANKI_CFILE);
	}
	else if(m_type == Type::ZIP)
	{
		readSize = unzReadCurrentFile(m_file, buff, size);
	}
#if ANKI_OS == ANKI_OS_ANDROID
	else if(m_type == Type::SPECIAL)
	{
		readSize = AAsset_read(ANKI_AFILE, buff, size);
	}
#endif
	else
	{
		ANKI_ASSERT(0);
	}

	Error err = ErrorCode::NONE;
	if(static_cast<I64>(size) != readSize)
	{
		ANKI_LOGE("File read failed");
		err = ErrorCode::FILE_ACCESS;
	}

	return err;
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
		fseek(ANKI_CFILE, 0, SEEK_END);
		I64 size = ftell(ANKI_CFILE);
		if(size < 1)
		{
			ANKI_LOGE("ftell() failed");
		}
		else
		{
			out = size;
		}

		rewind(ANKI_CFILE);
	}
	else if(m_type == Type::ZIP)
	{
		ANKI_ASSERT(m_size != 0);
		out = m_size;
	}
#if ANKI_OS == ANKI_OS_ANDROID
	else if(m_type == Type::SPECIAL)
	{
		out = AAsset_getLength(ANKI_AFILE);
	}
#endif
	else
	{
		ANKI_ASSERT(0);
	}

	return out;
}

//==============================================================================
Error File::readU32(U32& out)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != OpenFlag::NONE);
	ANKI_ASSERT((m_flags & OpenFlag::READ) != OpenFlag::NONE);
	ANKI_ASSERT((m_flags & OpenFlag::BINARY) != OpenFlag::NONE 
		&& "Should be binary file");
	ANKI_ASSERT(
		(m_flags & OpenFlag::BIG_ENDIAN) != (m_flags & OpenFlag::LITTLE_ENDIAN) 
		&& "One of those 2 should be active");

	Error err = read(&out, sizeof(out));
	if(!err)
	{
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
			U8* c = reinterpret_cast<U8*>(&out);
			out = (c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24));
		}
		else
		{
			U8* c = reinterpret_cast<U8*>(&out);
			out = ((c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3]);
		}
	}

	return err;
}

//==============================================================================
Error File::readF32(F32& out)
{
	U32 integer = MAX_U32;
	Error err = readU32(integer);
	if(!err)
	{
		std::memcpy(&out, &integer, sizeof(F32));
	}

	return err;
}

//==============================================================================
Error File::write(void* buff, PtrSize size)
{
	ANKI_ASSERT(buff);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(m_file);
	ANKI_ASSERT((m_flags & OpenFlag::WRITE) != OpenFlag::NONE);

	Error err = ErrorCode::NONE;

	if(m_type == Type::C)
	{
		PtrSize writeSize = 0;
		writeSize = std::fwrite(buff, 1, size, ANKI_CFILE);

		if(writeSize != size)
		{
			// Failed to write on file
			err = ErrorCode::FILE_ACCESS;
		}
	}
	else if(m_type == Type::ZIP
#if ANKI_OS == ANKI_OS_ANDROID
		|| m_type == Type::SPECIAL
#endif
		)
	{
		ANKI_LOGE("Writting to archives is not supported");
		err = ErrorCode::FILE_ACCESS;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	return err;
}

//==============================================================================
Error File::writeText(CString format, ...)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != OpenFlag::NONE);
	ANKI_ASSERT((m_flags & OpenFlag::WRITE) != OpenFlag::NONE);
	ANKI_ASSERT((m_flags & OpenFlag::BINARY) == OpenFlag::NONE);

	Error err = ErrorCode::NONE;
	va_list args;
	va_start(args, format);

	if(m_type == Type::C)
	{
		std::vfprintf(ANKI_CFILE, &format[0], args);
	}
	else if(m_type == Type::ZIP
#if ANKI_OS == ANKI_OS_ANDROID
		|| m_type == Type::SPECIAL
#endif
		)
	{
		ANKI_LOGE("Writting to archives is not supported");
		err = ErrorCode::FILE_ACCESS;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	va_end(args);
	return err;
}

//==============================================================================
Error File::seek(PtrSize offset, SeekOrigin origin)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != OpenFlag::NONE);
	Error err = ErrorCode::NONE;

	if(m_type == Type::C)
	{
		if(fseek(ANKI_CFILE, offset, (I)origin) != 0)
		{
			ANKI_LOGE("fseek() failed");
			err = ErrorCode::FUNCTION_FAILED;
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
				ANKI_LOGE("Rewind failed");
				err = ErrorCode::FUNCTION_FAILED;
			}
		}

		if(!err)
		{
			// Move forward by reading dummy data
			char buff[128];
			while(!err && offset != 0)
			{
				PtrSize toRead = std::min(offset, sizeof(buff));
				err = read(buff, toRead);
				offset -= toRead;
			}
		}
	}
#if ANKI_OS == ANKI_OS_ANDROID
	else if(m_type == Type::SPECIAL)
	{
		if(AAsset_seek(ANKI_AFILE, offset, origin) == (off_t)-1)
		{
			ANKI_LOGE("AAsset_seek() failed");
			err = ErrorCode::FUNCTION_FAILED;
		}
	}
#endif
	else
	{
		ANKI_ASSERT(0);
	}

	return err;
}

//==============================================================================
Error File::identifyFile(const CString& filename, 
	char* archiveFilename, PtrSize archiveFilenameLength,
	CString& filenameInArchive, Type& type)
{
	Error err = ErrorCode::NONE;

#if ANKI_OS == ANKI_OS_ANDROID
	if(filename[0] == '$')
	{
		type = Type::SPECIAL;
	}
	else
#endif
	{
		static const char aext[] = {".ankizip"};
		const PtrSize aextLen = sizeof(aext) - 1;
		const char* ptrToArchiveExt = std::strstr(&filename[0], aext);

		if(ptrToArchiveExt == nullptr)
		{
			// It's a C file
			type = Type::C;
		}
		else
		{
			// Maybe it's a file in a zipped archive

			auto fnameLen = filename.getLength();
			auto archLen = (ptrToArchiveExt - filename.get()) + aextLen;

			if(archLen + 1 >= fnameLen)
			{
				ANKI_LOGE("Too sort archived filename");
				err = ErrorCode::FILE_NOT_FOUND;
			}

			if(archiveFilenameLength > archLen)
			{
				ANKI_LOGE("Using too long paths");
				err = ErrorCode::FILE_NOT_FOUND;
			}

			if(!err)
			{
				std::memcpy(archiveFilename, &filename[0], archLen);
				archiveFilename[archLen] = '\0';

				if(directoryExists(CString(archiveFilename)))
				{
					// It's a directory so failback to C file
					type = Type::C;
				}
				else
				{
					// It's a ziped file

					filenameInArchive = CString(&filename[0] + archLen + 1);

					type = Type::ZIP;
				}
			}
		}
	}

	return err;
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

} // end namespace anki
