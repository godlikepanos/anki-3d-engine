// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/File.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Assert.h>
#include <cstring>
#include <cstdarg>
#if ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#	include <android/asset_manager.h>
#endif

namespace anki
{

#if ANKI_OS_ANDROID
#	define ANKI_AFILE static_cast<AAsset*>(m_file)
#endif

#define ANKI_CFILE reinterpret_cast<FILE*>(m_file)

File& File::operator=(File&& b)
{
	close();

	if(b.m_file != nullptr)
	{
		m_file = b.m_file;
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
	ANKI_ASSERT(m_file == nullptr && m_flags == FileOpenFlag::NONE);

	// Only these flags are accepted
	ANKI_ASSERT((flags
				 & (FileOpenFlag::READ | FileOpenFlag::WRITE | FileOpenFlag::APPEND | FileOpenFlag::BINARY
					| FileOpenFlag::ENDIAN_LITTLE | FileOpenFlag::ENDIAN_BIG | FileOpenFlag::SPECIAL))
				!= FileOpenFlag::NONE);

	// Cannot be both
	ANKI_ASSERT((flags & FileOpenFlag::READ) != (flags & FileOpenFlag::WRITE));

	// Open it
#if ANKI_OS_ANDROID
	if(!!(flags & FileOpenFlag::SPECIAL))
	{
		ANKI_CHECK(openAndroidFile(filename, flags));
	}
	else
#endif
	{
		ANKI_CHECK(openCFile(filename, flags));
	}

	// Set endianess
	if((flags & (FileOpenFlag::ENDIAN_BIG | FileOpenFlag::ENDIAN_LITTLE)) == FileOpenFlag::NONE)
	{
		// If the open() DIDN'T provided us the file endianess Set the machine's endianness
		m_flags = m_flags | getMachineEndianness();
	}
	else
	{
		// Else just make sure that only one of the flags is set
		ANKI_ASSERT((flags & FileOpenFlag::ENDIAN_BIG) != (flags & FileOpenFlag::ENDIAN_LITTLE));
	}

	return Error::NONE;
}

Error File::openCFile(const CString& filename, FileOpenFlag flags)
{
	Error err = Error::NONE;
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
	m_file = fopen(filename.cstr(), openMode);
	if(m_file == nullptr)
	{
		ANKI_UTIL_LOGE("Failed to open file \"%s\", open mode \"%s\"", &filename[0], openMode);
		err = Error::FILE_ACCESS;
	}
	else
	{
		m_flags = flags;
	}

	// Get file size
	if(((flags & FileOpenFlag::READ) != FileOpenFlag::NONE) && !err)
	{
		fseek(ANKI_CFILE, 0, SEEK_END);
		I64 size = ftell(ANKI_CFILE);
		if(size < 1)
		{
			ANKI_UTIL_LOGE("ftell() failed");
			err = Error::FUNCTION_FAILED;
		}
		else
		{
			m_size = U32(size);
			rewind(ANKI_CFILE);
		}
	}

	return err;
}

#if ANKI_OS_ANDROID
Error File::openAndroidFile(const CString& filename, FileOpenFlag flags)
{
	ANKI_ASSERT(!!(flags & FileOpenFlag::SPECIAL));

	if(!!(flags & FileOpenFlag::WRITE))
	{
		ANKI_UTIL_LOGE("Cannot write inside archives");
		return Error::FILE_ACCESS;
	}

	if(!(flags & FileOpenFlag::READ))
	{
		ANKI_UTIL_LOGE("Missing FileOpenFlag::READ flag");
		return Error::FILE_ACCESS;
	}

	// Open file
	ANKI_ASSERT(g_androidApp != nullptr && g_androidApp->activity && g_androidApp->activity->assetManager);

	m_file = AAssetManager_open(g_androidApp->activity->assetManager, filename.cstr(), AASSET_MODE_STREAMING);

	if(m_file == nullptr)
	{
		ANKI_UTIL_LOGE("AAssetManager_open() failed");
		return Error::FILE_ACCESS;
	}

	m_flags = flags;

	return Error::NONE;
}
#endif

void File::close()
{
	if(m_file)
	{
#if ANKI_OS_ANDROID
		if(!!(m_flags & FileOpenFlag::SPECIAL))
		{
			AAsset_close(ANKI_AFILE);
		}
		else
#endif
		{
			fclose(ANKI_CFILE);
		}
	}

	zero();
}

Error File::flush()
{
	ANKI_ASSERT(m_file);
	Error err = Error::NONE;

	if((m_flags & FileOpenFlag::WRITE) != FileOpenFlag::NONE)
	{
#if ANKI_OS_ANDROID
		if(!!(m_flags & FileOpenFlag::SPECIAL))
		{
			ANKI_ASSERT(0 && "Cannot have write these file types");
		}
		else
#endif
		{
			const I ierr = fflush(ANKI_CFILE);
			if(ierr)
			{
				ANKI_UTIL_LOGE("fflush() failed");
				err = Error::FUNCTION_FAILED;
			}
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

#if ANKI_OS_ANDROID
	if(!!(m_flags & FileOpenFlag::SPECIAL))
	{
		readSize = AAsset_read(ANKI_AFILE, buff, size);
	}
	else
#endif
	{
		readSize = fread(buff, 1, size, ANKI_CFILE);
	}

	Error err = Error::NONE;
	if(static_cast<I64>(size) != readSize)
	{
		ANKI_UTIL_LOGE("File read failed");
		err = Error::FILE_ACCESS;
	}

	return err;
}

PtrSize File::getSize() const
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::NONE);
	PtrSize out = 0;

#if ANKI_OS_ANDROID
	if(!!(m_flags & FileOpenFlag::SPECIAL))
	{
		out = AAsset_getLength(ANKI_AFILE);
	}
	else
#endif
	{
		ANKI_ASSERT(m_size != 0);
		out = m_size;
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
		memcpy(&out, &integer, sizeof(F32));
	}

	return err;
}

Error File::write(const void* buff, PtrSize size)
{
	ANKI_ASSERT(buff);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(m_file);
	ANKI_ASSERT((m_flags & FileOpenFlag::WRITE) != FileOpenFlag::NONE);

	Error err = Error::NONE;

#if ANKI_OS_ANDROID
	if(!!(m_flags & FileOpenFlag::SPECIAL))
	{
		ANKI_UTIL_LOGE("Writting to special files is not supported");
		err = Error::FILE_ACCESS;
	}
	else
#endif
	{
		PtrSize writeSize = 0;
		writeSize = std::fwrite(buff, 1, size, ANKI_CFILE);

		if(writeSize != size)
		{
			ANKI_UTIL_LOGE("std::fwrite() failed");
			err = Error::FILE_ACCESS;
		}
	}

	return err;
}

Error File::writeText(CString format, ...)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::NONE);
	ANKI_ASSERT((m_flags & FileOpenFlag::WRITE) != FileOpenFlag::NONE);
	ANKI_ASSERT((m_flags & FileOpenFlag::BINARY) == FileOpenFlag::NONE);

	Error err = Error::NONE;
	va_list args;
	va_start(args, format);

#if ANKI_OS_ANDROID
	if(!!(m_flags & FileOpenFlag::SPECIAL))
	{
		ANKI_UTIL_LOGE("Writting to special files is not supported");
		err = Error::FILE_ACCESS;
	}
	else
#endif
	{
		std::vfprintf(ANKI_CFILE, &format[0], args);
	}

	va_end(args);
	return err;
}

Error File::seek(PtrSize offset, FileSeekOrigin origin)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::NONE);
	Error err = Error::NONE;

#if ANKI_OS_ANDROID
	if(!!(m_flags & FileOpenFlag::SPECIAL))
	{
		if(AAsset_seek(ANKI_AFILE, offset, I32(origin)) == off_t(-1))
		{
			ANKI_UTIL_LOGE("AAsset_seek() failed");
			err = Error::FUNCTION_FAILED;
		}
	}
	else
#endif
	{
		if(fseek(ANKI_CFILE, offset, I32(origin)) != 0)
		{
			ANKI_UTIL_LOGE("fseek() failed");
			err = Error::FUNCTION_FAILED;
		}
	}

	return err;
}

PtrSize File::tell()
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::NONE);

#if ANKI_OS_ANDROID
	if(!!(m_flags & FileOpenFlag::SPECIAL))
	{
		ANKI_ASSERT(0);
	}
	else
#endif
	{
		return ftell(ANKI_CFILE);
	}

	return 0;
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
	Error err = Error::NONE;
	PtrSize size = getSize();

	if(size != 0)
	{
		out.create(alloc, '?', size);
		err = read(&out[0], size);
	}
	else
	{
		err = Error::FUNCTION_FAILED;
	}

	return err;
}

Error File::readAllText(StringAuto& out)
{
	Error err = Error::NONE;
	PtrSize size = getSize();

	if(size != 0)
	{
		out.create('?', size);
		err = read(&out[0], size);
	}
	else
	{
		err = Error::FUNCTION_FAILED;
	}

	return err;
}

} // end namespace anki
