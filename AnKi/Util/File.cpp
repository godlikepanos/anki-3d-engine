// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
#if ANKI_POSIX
#	include <sys/stat.h>
#endif

namespace anki {

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
	ANKI_ASSERT(m_file == nullptr && m_flags == FileOpenFlag::kNone);

	// Only these flags are accepted
	ANKI_ASSERT((flags
				 & (FileOpenFlag::kRead | FileOpenFlag::kWrite | FileOpenFlag::kAppend | FileOpenFlag::kBinary
					| FileOpenFlag::kLittleEndian | FileOpenFlag::kBigEndian | FileOpenFlag::kSpecial))
				!= FileOpenFlag::kNone);

	// Cannot be both
	ANKI_ASSERT((flags & FileOpenFlag::kRead) != (flags & FileOpenFlag::kWrite));

	// Open it
#if ANKI_OS_ANDROID
	if(!!(flags & FileOpenFlag::kSpecial))
	{
		ANKI_CHECK(openAndroidFile(filename, flags));
	}
	else
#endif
	{
		ANKI_CHECK(openCFile(filename, flags));
	}

	// Set endianess
	if((flags & (FileOpenFlag::kBigEndian | FileOpenFlag::kLittleEndian)) == FileOpenFlag::kNone)
	{
		// If the open() DIDN'T provided us the file endianess Set the machine's endianness
		m_flags = m_flags | getMachineEndianness();
	}
	else
	{
		// Else just make sure that only one of the flags is set
		ANKI_ASSERT((flags & FileOpenFlag::kBigEndian) != (flags & FileOpenFlag::kLittleEndian));
	}

	return Error::kNone;
}

Error File::openCFile(const CString& filename, FileOpenFlag flags)
{
	Error err = Error::kNone;
	const char* openMode;

	if((flags & FileOpenFlag::kRead) != FileOpenFlag::kNone)
	{
		openMode = "rb";
	}
	else if((flags & FileOpenFlag::kAppend) == FileOpenFlag::kAppend)
	{
		openMode = "a+b";
	}
	else if((flags & FileOpenFlag::kWrite) != FileOpenFlag::kNone)
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
		err = Error::kFileAccess;
	}
	else
	{
		m_flags = flags;
	}

	// Get file size
	if(!!(flags & FileOpenFlag::kRead) && !err)
	{
#if ANKI_POSIX
		const int fd = fileno(ANKI_CFILE);
		struct stat stbuf;

		if((fstat(fd, &stbuf) != 0) || (!S_ISREG(stbuf.st_mode)))
		{
			ANKI_UTIL_LOGE("fstat() failed");
			err = Error::kFunctionFailed;
		}
		else
		{
			m_size = stbuf.st_size;
		}
#else
		fseek(ANKI_CFILE, 0, SEEK_END);
		I64 size = ftell(ANKI_CFILE);
		if(size < 1)
		{
			ANKI_UTIL_LOGE("ftell() failed");
			err = Error::kFunctionFailed;
		}
		else
		{
			m_size = PtrSize(size);
			rewind(ANKI_CFILE);
		}
#endif
	}

	return err;
}

#if ANKI_OS_ANDROID
Error File::openAndroidFile(const CString& filename, FileOpenFlag flags)
{
	ANKI_ASSERT(!!(flags & FileOpenFlag::kSpecial));

	if(!!(flags & FileOpenFlag::kWrite))
	{
		ANKI_UTIL_LOGE("Cannot write inside archives");
		return Error::kFileAccess;
	}

	if(!(flags & FileOpenFlag::kRead))
	{
		ANKI_UTIL_LOGE("Missing FileOpenFlag::kRead flag");
		return Error::kFileAccess;
	}

	// Open file
	ANKI_ASSERT(g_androidApp != nullptr && g_androidApp->activity && g_androidApp->activity->assetManager);

	m_file = AAssetManager_open(g_androidApp->activity->assetManager, filename.cstr(), AASSET_MODE_STREAMING);

	if(m_file == nullptr)
	{
		ANKI_UTIL_LOGE("AAssetManager_open() failed");
		return Error::kFileAccess;
	}

	m_flags = flags;

	// Get size
	m_size = AAsset_getLength(ANKI_AFILE);

	return Error::kNone;
}
#endif

void File::close()
{
	if(m_file)
	{
#if ANKI_OS_ANDROID
		if(!!(m_flags & FileOpenFlag::kSpecial))
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
	Error err = Error::kNone;

	if((m_flags & FileOpenFlag::kWrite) != FileOpenFlag::kNone)
	{
#if ANKI_OS_ANDROID
		if(!!(m_flags & FileOpenFlag::kSpecial))
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
				err = Error::kFunctionFailed;
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
	ANKI_ASSERT((m_flags & FileOpenFlag::kRead) != FileOpenFlag::kNone);

	I64 readSize = 0;

#if ANKI_OS_ANDROID
	if(!!(m_flags & FileOpenFlag::kSpecial))
	{
		readSize = AAsset_read(ANKI_AFILE, buff, size);
	}
	else
#endif
	{
		readSize = fread(buff, 1, size, ANKI_CFILE);
	}

	Error err = Error::kNone;
	if(static_cast<I64>(size) != readSize)
	{
		ANKI_UTIL_LOGE("File read failed");
		err = Error::kFileAccess;
	}

	return err;
}

Error File::readU32(U32& out)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::kNone);
	ANKI_ASSERT((m_flags & FileOpenFlag::kRead) != FileOpenFlag::kNone);
	ANKI_ASSERT((m_flags & FileOpenFlag::kBinary) != FileOpenFlag::kNone && "Should be binary file");
	ANKI_ASSERT((m_flags & FileOpenFlag::kBigEndian) != (m_flags & FileOpenFlag::kLittleEndian)
				&& "One of those 2 should be active");

	Error err = read(&out, sizeof(out));
	if(!err)
	{
		// Copy it
		FileOpenFlag machineEndianness = getMachineEndianness();
		FileOpenFlag fileEndianness = ((m_flags & FileOpenFlag::kBigEndian) != FileOpenFlag::kNone)
										  ? FileOpenFlag::kBigEndian
										  : FileOpenFlag::kLittleEndian;

		if(machineEndianness == fileEndianness)
		{
			// Same endianness between the file and the machine. Do nothing
		}
		else if(machineEndianness == FileOpenFlag::kBigEndian && fileEndianness == FileOpenFlag::kLittleEndian)
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
	U32 integer = kMaxU32;
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
	ANKI_ASSERT((m_flags & FileOpenFlag::kWrite) != FileOpenFlag::kNone);

	Error err = Error::kNone;

#if ANKI_OS_ANDROID
	if(!!(m_flags & FileOpenFlag::kSpecial))
	{
		ANKI_UTIL_LOGE("Writting to special files is not supported");
		err = Error::kFileAccess;
	}
	else
#endif
	{
		const PtrSize writeSize = fwrite(buff, 1, size, ANKI_CFILE);

		if(writeSize != size)
		{
			ANKI_UTIL_LOGE("fwrite() failed");
			err = Error::kFileAccess;
		}
	}

	return err;
}

Error File::writeTextf(const Char* format, ...)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::kNone);
	ANKI_ASSERT((m_flags & FileOpenFlag::kWrite) != FileOpenFlag::kNone);
	ANKI_ASSERT((m_flags & FileOpenFlag::kBinary) == FileOpenFlag::kNone);

	Error err = Error::kNone;
	va_list args;
	va_start(args, format);

#if ANKI_OS_ANDROID
	if(!!(m_flags & FileOpenFlag::kSpecial))
	{
		ANKI_UTIL_LOGE("Writting to special files is not supported");
		err = Error::kFileAccess;
	}
	else
#endif
	{
		std::vfprintf(ANKI_CFILE, format, args);
	}

	va_end(args);
	return err;
}

Error File::writeText(CString text)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::kNone);
	ANKI_ASSERT((m_flags & FileOpenFlag::kWrite) != FileOpenFlag::kNone);
	ANKI_ASSERT((m_flags & FileOpenFlag::kBinary) == FileOpenFlag::kNone);

	const PtrSize writeSize = text.getLength();
	if(ANKI_UNLIKELY(writeSize == 0))
	{
		return Error::kNone;
	}

	const PtrSize writenSize = fwrite(text.cstr(), 1, writeSize, ANKI_CFILE);

	if(writeSize != writenSize)
	{
		ANKI_UTIL_LOGE("fwrite() failed");
		return Error::kFileAccess;
	}
	else
	{
		return Error::kNone;
	}
}

Error File::seek(PtrSize offset, FileSeekOrigin origin)
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::kNone);
	Error err = Error::kNone;

#if ANKI_OS_ANDROID
	if(!!(m_flags & FileOpenFlag::kSpecial))
	{
		if(AAsset_seek(ANKI_AFILE, offset, I32(origin)) == off_t(-1))
		{
			ANKI_UTIL_LOGE("AAsset_seek() failed");
			err = Error::kFunctionFailed;
		}
	}
	else
#endif
	{
		if(fseek(ANKI_CFILE, (long int)(offset), I32(origin)) != 0)
		{
			ANKI_UTIL_LOGE("fseek() failed");
			err = Error::kFunctionFailed;
		}
	}

	return err;
}

PtrSize File::tell()
{
	ANKI_ASSERT(m_file);
	ANKI_ASSERT(m_flags != FileOpenFlag::kNone);

#if ANKI_OS_ANDROID
	if(!!(m_flags & FileOpenFlag::kSpecial))
	{
		ANKI_ASSERT(0);
		return 0;
	}
	else
#endif
	{
		return ftell(ANKI_CFILE);
	}
}

FileOpenFlag File::getMachineEndianness()
{
	I32 num = 1;
	if(*(char*)&num == 1)
	{
		return FileOpenFlag::kLittleEndian;
	}
	else
	{
		return FileOpenFlag::kBigEndian;
	}
}

Error File::readAllText(BaseMemoryPool& pool, String& out)
{
	Error err = Error::kNone;
	PtrSize size = getSize();

	if(size != 0)
	{
		out.create(pool, '?', size);
		err = read(&out[0], size);
	}
	else
	{
		err = Error::kFunctionFailed;
	}

	return err;
}

Error File::readAllText(StringRaii& out)
{
	Error err = Error::kNone;
	PtrSize size = getSize();

	if(size != 0)
	{
		out.create('?', size);
		err = read(&out[0], size);
	}
	else
	{
		err = Error::kFunctionFailed;
	}

	return err;
}

} // end namespace anki
