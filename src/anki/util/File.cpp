// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/File.h>
#include <anki/util/Filesystem.h>
#include <anki/util/Logger.h>
#include <anki/util/Assert.h>
#include <cstring>
#include <cstdarg>

namespace anki
{

#if ANKI_OS == ANKI_OS_ANDROID
extern android_app* gAndroidApp;

#define ANKI_AFILE reinterpret_cast<AAsset*>(m_file)
#endif

#define ANKI_CFILE reinterpret_cast<FILE*>(m_file)

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

File::~File()
{
	close();
}

Error File::open(const CString& filename, FileOpenFlag flags)
{
	ANKI_ASSERT(m_file == nullptr && m_flags == FileOpenFlag::NONE && m_type == Type::NONE);

	// Only these flags are accepted
	ANKI_ASSERT((flags & (FileOpenFlag::READ | FileOpenFlag::WRITE | FileOpenFlag::APPEND | FileOpenFlag::BINARY
							 | FileOpenFlag::ENDIAN_LITTLE
							 | FileOpenFlag::ENDIAN_BIG))
		!= FileOpenFlag::NONE);

	// Cannot be both
	ANKI_ASSERT((flags & FileOpenFlag::READ) != (flags & FileOpenFlag::WRITE));

	//
	// Determine the file type and open it
	//

	Array<char, 512> archive;
	CString filenameInArchive;
	Type ft;
	Error err = identifyFile(filename, &archive[0], archive.getSize(), filenameInArchive, ft);

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
		if((flags & (FileOpenFlag::ENDIAN_BIG | FileOpenFlag::ENDIAN_LITTLE)) == FileOpenFlag::NONE)
		{
			// Set the machine's endianness
			m_flags = m_flags | getMachineEndianness();
		}
		else
		{
			// Else just make sure that only one of the flags is set
			ANKI_ASSERT((flags & FileOpenFlag::ENDIAN_BIG) != (flags & FileOpenFlag::ENDIAN_LITTLE));
		}
	}

	return err;
}

Error File::openCFile(const CString& filename, FileOpenFlag flags)
{
	Error err = ErrorCode::NONE;
	const char* openMode;

	if((flags & FileOpenFlag::READ) != FileOpenFlag::NONE)
	{
		openMode = "rb";
	}
	else if((flags & FileOpenFlag::APPEND) == FileOpenFlag::APPEND)
	{
		openMode = "a+b";
	}
	else if((flags & FileOpenFlag::WRITE) != FileOpenFlag::NONE)
	{
		openMode = "wb";
	}
	else
	{
		ANKI_ASSERT(0);
		openMode = nullptr;
	}

	// Open
	m_file = fopen(filename.get(), openMode);
	if(m_file == nullptr)
	{
		ANKI_UTIL_LOGE("Failed to open file %s", &filename[0]);
		err = ErrorCode::FILE_ACCESS;
	}
	else
	{
		m_flags = flags;
		m_type = Type::C;
	}

	// Get file size
	if(((flags & FileOpenFlag::READ) != FileOpenFlag::NONE) && !err)
	{
		fseek(ANKI_CFILE, 0, SEEK_END);
		I64 size = ftell(ANKI_CFILE);
		if(size < 1)
		{
			ANKI_UTIL_LOGE("ftell() failed");
			err = ErrorCode::FUNCTION_FAILED;
		}
		else
		{
			m_size = size;
			rewind(ANKI_CFILE);
		}
	}

	return err;
}

#if ANKI_OS == ANKI_OS_ANDROID
Error File::openAndroidFile(const CString& filename, FileOpenFlag flags)
{
	if((flags & FileOpenFlag::WRITE) != FileOpenFlag::NONE)
	{
		ANKI_UTIL_LOGE("Cannot write inside archives");
		return ErrorCode::FILE_ACCESS;
	}

	if((flags & FileOpenFlag::READ) != FileOpenFlag::NONE)
	{
		ANKI_UTIL_LOGE("Missing FileOpenFlag::READ flag");
		return ErrorCode::FILE_ACCESS;
	}

	// Open file
	ANKI_ASSERT(gAndroidApp != nullptr && gAndroidApp->activity && gAndroidApp->activity->assetManager);

	m_file = AAssetManager_open(gAndroidApp->activity->assetManager, &filename[0] + 1, AASSET_MODE_STREAMING);

	if(m_file == nullptr)
	{
		ANKI_UTIL_LOGE("AAssetManager_open() failed");
		return ErrorCode::FILE_ACCESS;
	}

	m_flags = flags;
	m_type = Type::SPECIAL;

	return ErrorCode::NONE;
}
#endif

void File::close()
{
	if(m_file)
	{
		if(m_type == Type::C)
		{
			fclose(ANKI_CFILE);
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

Error File::flush()
{
	ANKI_ASSERT(m_file);
	Error err = ErrorCode::NONE;

	if((m_flags & FileOpenFlag::WRITE) != FileOpenFlag::NONE)
	{
		if(m_type == Type::C)
		{
			I ierr = fflush(ANKI_CFILE);
			if(ierr)
			{
				ANKI_UTIL_LOGE("fflush() failed");
				err = ErrorCode::FUNCTION_FAILED;
			}
		}
#if ANKI_OS == ANKI_OS_ANDROID
		else if(m_type == Type::SPECIAL)
		{
			ANKI_ASSERT(0 && "Cannot have write these file types");
		}
#endif
		else
		{
			ANKI_ASSERT(0);
		}
	}

	return err;
}

Error File::read(void* buff, PtrSize size)
{
	ANKI_ASSERT(buff);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(m_file);
	ANKI_ASSERT((m_flags & FileOpenFlag::READ) != FileOpenFlag::NONE);

	I64 readSize = 0;

	if(m_type == Type::C)
	{
		readSize = fread(buff, 1, size, ANKI_CFILE);
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
		ANKI_UTIL_LOGE("File read failed");
		err = ErrorCode::FILE_ACCESS;
	}

	return err;
}

PtrSize File::getSize() const
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::NONE);
	PtrSize out = 0;

	if(m_type == Type::C)
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

Error File::readU32(U32& out)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::NONE);
	ANKI_ASSERT((m_flags & FileOpenFlag::READ) != FileOpenFlag::NONE);
	ANKI_ASSERT((m_flags & FileOpenFlag::BINARY) != FileOpenFlag::NONE && "Should be binary file");
	ANKI_ASSERT((m_flags & FileOpenFlag::ENDIAN_BIG) != (m_flags & FileOpenFlag::ENDIAN_LITTLE)
		&& "One of those 2 should be active");

	Error err = read(&out, sizeof(out));
	if(!err)
	{
		// Copy it
		FileOpenFlag machineEndianness = getMachineEndianness();
		FileOpenFlag fileEndianness = ((m_flags & FileOpenFlag::ENDIAN_BIG) != FileOpenFlag::NONE)
			? FileOpenFlag::ENDIAN_BIG
			: FileOpenFlag::ENDIAN_LITTLE;

		if(machineEndianness == fileEndianness)
		{
			// Same endianness between the file and the machine. Do nothing
		}
		else if(machineEndianness == FileOpenFlag::ENDIAN_BIG && fileEndianness == FileOpenFlag::ENDIAN_LITTLE)
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

Error File::write(void* buff, PtrSize size)
{
	ANKI_ASSERT(buff);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(m_file);
	ANKI_ASSERT((m_flags & FileOpenFlag::WRITE) != FileOpenFlag::NONE);

	Error err = ErrorCode::NONE;

	if(m_type == Type::C)
	{
		PtrSize writeSize = 0;
		writeSize = std::fwrite(buff, 1, size, ANKI_CFILE);

		if(writeSize != size)
		{
			ANKI_UTIL_LOGE("std::fwrite() failed");
			err = ErrorCode::FILE_ACCESS;
		}
	}
#if ANKI_OS == ANKI_OS_ANDROID
	else if(m_type == Type::SPECIAL)
	{
		ANKI_UTIL_LOGE("Writting to special files is not supported");
		err = ErrorCode::FILE_ACCESS;
	}
#endif
	else
	{
		ANKI_ASSERT(0);
	}

	return err;
}

Error File::writeText(CString format, ...)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::NONE);
	ANKI_ASSERT((m_flags & FileOpenFlag::WRITE) != FileOpenFlag::NONE);
	ANKI_ASSERT((m_flags & FileOpenFlag::BINARY) == FileOpenFlag::NONE);

	Error err = ErrorCode::NONE;
	va_list args;
	va_start(args, format);

	if(m_type == Type::C)
	{
		std::vfprintf(ANKI_CFILE, &format[0], args);
	}
#if ANKI_OS == ANKI_OS_ANDROID
	else if(m_type == Type::SPECIAL)
	{
		ANKI_UTIL_LOGE("Writting to special files is not supported");
		err = ErrorCode::FILE_ACCESS;
	}
#endif
	else
	{
		ANKI_ASSERT(0);
	}

	va_end(args);
	return err;
}

Error File::seek(PtrSize offset, SeekOrigin origin)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::NONE);
	Error err = ErrorCode::NONE;

	if(m_type == Type::C)
	{
		if(fseek(ANKI_CFILE, offset, (I)origin) != 0)
		{
			ANKI_UTIL_LOGE("fseek() failed");
			err = ErrorCode::FUNCTION_FAILED;
		}
	}
#if ANKI_OS == ANKI_OS_ANDROID
	else if(m_type == Type::SPECIAL)
	{
		if(AAsset_seek(ANKI_AFILE, offset, origin) == (off_t)-1)
		{
			ANKI_UTIL_LOGE("AAsset_seek() failed");
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

Error File::identifyFile(const CString& filename,
	char* archiveFilename,
	PtrSize archiveFilenameLength,
	CString& filenameInArchive,
	Type& type)
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
		// It's a C file
		type = Type::C;
	}

	return err;
}

FileOpenFlag File::getMachineEndianness()
{
	I32 num = 1;
	if(*(char*)&num == 1)
	{
		return FileOpenFlag::ENDIAN_LITTLE;
	}
	else
	{
		return FileOpenFlag::ENDIAN_BIG;
	}
}

Error File::readAllText(GenericMemoryPoolAllocator<U8> alloc, String& out)
{
	Error err = ErrorCode::NONE;
	PtrSize size = getSize();

	if(size != 0)
	{
		out.create(alloc, '?', size);
		err = read(&out[0], size);
	}
	else
	{
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

Error File::readAllText(StringAuto& out)
{
	Error err = ErrorCode::NONE;
	PtrSize size = getSize();

	if(size != 0)
	{
		out.create('?', size);
		err = read(&out[0], size);
	}
	else
	{
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

} // end namespace anki
