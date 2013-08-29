#include "anki/util/File.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include "anki/core/Logger.h" // XXX
#include <fstream>
#include <cstring>
#include <cstdarg>
#include <contrib/minizip/unzip.h>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android/asset_manager.h>
#endif

namespace anki {

//==============================================================================
// File                                                                        =
//==============================================================================

#if ANKI_OS == ANKI_OS_ANDROID
AAssetManager* File::andAssetManager = nullptr;
#endif

//==============================================================================
File::~File()
{
	close();
}

//==============================================================================
void File::open(const char* filename, U16 flags_)
{
	ANKI_ASSERT(filename);
	ANKI_ASSERT(strlen(filename) > 0);
	ANKI_ASSERT(file == nullptr && flags == 0);

	// Only these flags are accepted
	ANKI_ASSERT((flags_ & (OF_READ | OF_WRITE | OF_APPEND | OF_BINARY 
		| E_LITTLE_ENDIAN | E_BIG_ENDIAN)) != 0);

	// Cannot be both
	ANKI_ASSERT((flags_ & OF_READ) != (flags_ & OF_WRITE));

	//
	// In the following lines determine the file type and open it
	//

#if ANKI_OS == ANKI_OS_ANDROID
	if(filename[0] == '$')
	{
		openAndFile(filename, flags_);
	}
	else
#endif
	{
		const char* aext = ".ankizip";
		const char* ptrToArchiveExt = strstr(filename, aext);

		if(ptrToArchiveExt == nullptr)
		{
			// It's a C file
			openCFile(filename, flags_);
		}
		else
		{
			// Maybe it's a file in a zipped archive

			PtrSize fnameLen = strlen(filename);
			PtrSize extLen = strlen(aext);
			PtrSize archLen = (PtrSize)(ptrToArchiveExt - filename) + extLen;

			if(archLen + 1 >= fnameLen)
			{
				throw ANKI_EXCEPTION("Wrong file inside the archive");
			}

			std::string archive(filename, archLen);

			if(directoryExists(archive.c_str()))
			{
				// It's a directory so failback to C file
				openCFile(filename, flags_);
			}
			else
			{
				// It's a ziped file

				std::string filenameInArchive(
					filename + archLen + 1, fnameLen - archLen);

				openZipFile(archive.c_str(), filenameInArchive.c_str(), flags_);
			}
		}
	}

	//
	// Set endianess
	//

	// If the open() DIDN'T provided us the file endianess 
	if((flags_ & (E_BIG_ENDIAN | E_LITTLE_ENDIAN)) == 0)
	{
		// Set the machine's endianness
		flags |= getMachineEndianness();
	}
	else
	{
		// Else just make sure that only one of the flags is set
		ANKI_ASSERT((flags_ & E_BIG_ENDIAN) != (flags_ & E_LITTLE_ENDIAN));
	}
}

//==============================================================================
void File::openCFile(const char* filename, U16 flags_)
{
	const char* openMode;

	if(flags_ & OF_READ)
	{
		openMode = "r";
	}
	else if(flags_ & OF_WRITE)
	{
		openMode = "w";
	}
	else if(flags_ & OF_APPEND)
	{
		openMode = "a+";
	}
	else
	{
		ANKI_ASSERT(0);
		openMode = nullptr;
	}

	// Open
	file = (FILE*)fopen(filename, openMode);
	if(file == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to open file");
	}

	flags = flags_ | FT_C;
}

//==============================================================================
void File::openZipFile(const char* archive, const char* archived, U16 flags_)
{
	if(flags_ & OF_WRITE)
	{
		throw ANKI_EXCEPTION("Cannot write inside archives");
	}

	if(!(flags_ & OF_READ))
	{
		throw ANKI_EXCEPTION("Missing OF_READ flag");
	}

	// Open archive
	unzFile zfile = unzOpen(archive);
	if(zfile == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to open file");
	}

	// Locate archived
	const int caseSensitive = 1;
	if(unzLocateFile(zfile, archived, caseSensitive) != UNZ_OK)
	{
		unzClose(zfile);
		throw ANKI_EXCEPTION("Failed to locate file in archive");
	}

	// Open file
	if(unzOpenCurrentFile(zfile) != UNZ_OK )
	{
		unzClose(zfile);
		throw ANKI_EXCEPTION("unzOpenCurrentFile failed");
	}

	static_assert(sizeof(file) == sizeof(zfile), "See file");
	file = (void*)zfile;
	flags = flags_ | FT_ZIP;
}

//==============================================================================
#if ANKI_OS == ANKI_OS_ANDROID
void File::openAndFile(const char* filename, U16 flags_)
{
	if(flags_ & OF_WRITE)
	{
		throw ANKI_EXCEPTION("Cannot write inside archives");
	}

	if(!(flags_ & OF_READ))
	{
		throw ANKI_EXCEPTION("Missing OF_READ flag");
	}

	// Open file
	ANKI_ASSERT(andAssetManager != nullptr 
		&& "You should call setAndroidAssetManager() on engine initialization");
	file = AAssetManager_open(
		andAssetManager, filename + 1, AASSET_MODE_STREAMING);

	if(file == nullptr)
	{
		throw ANKI_EXCEPTION("Failed to open file");
	}

	flags = flags_ | FT_SPECIAL;
}
#endif

//==============================================================================
void File::close()
{
	if(file)
	{
		if(flags & FT_C)
		{
			fclose((FILE*)file);
		}
		else if(flags & FT_ZIP)
		{
			unzClose(file);
		}
#if ANKI_OS == ANKI_OS_ANDROID
		else if(flags & FT_SPECIAL)
		{
			AAsset_close((AAsset*)file);
		}
#endif
		else
		{
			ANKI_ASSERT(0);
		}
	}

	file = nullptr;
	flags = 0;
}

//==============================================================================
void File::read(void* buff, PtrSize size)
{
	ANKI_ASSERT(buff);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(file);
	ANKI_ASSERT(flags & OF_READ);

	I64 readSize = 0;

	if(flags & FT_C)
	{
		readSize = fread(buff, 1, size, (FILE*)file);
	}
	else if(flags & FT_ZIP)
	{
		readSize = unzReadCurrentFile(file, buff, size);
	}
#if ANKI_OS == ANKI_OS_ANDROID
	else if(flags & FT_SPECIAL)
	{
		readSize = AAsset_read((AAsset*)file, buff, size);
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
void File::readAllText(std::string& txt)
{
	ANKI_ASSERT(file);
	ANKI_ASSERT(flags);
	ANKI_ASSERT((flags & OF_BINARY) == 0 && "Should not be binary file");
	ANKI_ASSERT(flags & OF_READ);

	if(flags & FT_C)
	{
		// Get file size
		fseek((FILE*)file, 0, SEEK_END);
		I64 size = ftell((FILE*)file);
		if(size < 1)
		{
			throw ANKI_EXCEPTION("ftell() failed");
		}
		rewind((FILE*)file);

		// Read and write
		txt.resize(size + 1);
		PtrSize readSize = fread(&txt[0], 1, size, (FILE*)file);
		ANKI_ASSERT(readSize == (PtrSize)size);
		txt[readSize] = '\0';
	}
	else if(flags & FT_ZIP)
	{
		char buff[256];
		I64 readSize;

		while(true)
		{
			readSize = unzReadCurrentFile(file, buff, sizeof(buff) - 1);

			if(readSize > 0)
			{
				buff[readSize] = '\0';
				txt += buff;
			}
			else
			{
				break;
			}
		}

		if(readSize < 0)
		{
			throw ANKI_EXCEPTION("unzReadCurrentFile() failed");
		}
	}
#if ANKI_OS == ANKI_OS_ANDROID
	else if(flags & FT_SPECIAL)
	{
		PtrSize size = AAsset_getLength((AAsset*)file);
		AAsset_seek((AAsset*)file, 0, SEEK_SET);

		txt.resize(size + 1);
		I readSize = AAsset_read((AAsset*)file, &txt[0], size);
		ANKI_ASSERT((PtrSize)readSize == size);

		txt[readSize] = '\0';
	}
#endif
	else
	{
		ANKI_ASSERT(0);
	}
}

//==============================================================================
void File::readAllTextLines(StringList& lines)
{
	ANKI_ASSERT(file);
	ANKI_ASSERT(flags);
	ANKI_ASSERT((flags & OF_BINARY) == 0 && "Should not be binary file");
	ANKI_ASSERT(flags & OF_READ);

	if(flags & FT_C)
	{
		char line[1024];

		while(fgets(line, sizeof(line), (FILE*)file))
		{
			I len = strlen(line);

			if(len != sizeof(line) - 1)
			{
				// line is big enough

				line[len - 1] = '\0';
				lines.push_back(line);
			}
			else
			{
				throw ANKI_EXCEPTION("Line bigger than temp buffer");
			}			
		}
	}
	else if(flags & FT_ZIP
#if ANKI_OS == ANKI_OS_ANDROID
		|| flags & FT_SPECIAL
#endif
		)
	{
		std::string txt;
		readAllText(txt);
		lines = StringList::splitString(txt.c_str(), '\n');
	}
	else
	{
		ANKI_ASSERT(0);
	}
}

//==============================================================================
U32 File::readU32()
{
	ANKI_ASSERT(file);
	ANKI_ASSERT(flags);
	ANKI_ASSERT(flags & OF_READ);
	ANKI_ASSERT((flags & OF_BINARY) && "Should be binary file");
	ANKI_ASSERT((flags & E_BIG_ENDIAN) != (flags & E_LITTLE_ENDIAN) 
		&& "One of those 2 should be active");

	U32 out;
	read(&out, sizeof(out));
	
	// Copy it
	Endianness machineEndianness = getMachineEndianness();
	Endianness fileEndianness = (flags & E_BIG_ENDIAN) 
		? E_BIG_ENDIAN : E_LITTLE_ENDIAN;

	if(machineEndianness == fileEndianness)
	{
		// Same endianness between the file and the machine. Do nothing
	}
	else if(machineEndianness == E_BIG_ENDIAN 
		&& fileEndianness == E_LITTLE_ENDIAN)
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
	ANKI_ASSERT(file);
	ANKI_ASSERT(flags & OF_WRITE);

	if(flags & FT_C)
	{
		PtrSize writeSize = 0;
		writeSize = fwrite(buff, 1, size, (FILE*)file);

		if(writeSize != size)
		{
			throw ANKI_EXCEPTION("Failed to write on file");
		}
	}
	else if(flags & FT_ZIP
#if ANKI_OS == ANKI_OS_ANDROID
		|| flags & FT_SPECIAL
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
	ANKI_ASSERT(file);
	ANKI_ASSERT(flags);
	ANKI_ASSERT(flags & OF_WRITE);
	ANKI_ASSERT((flags & OF_BINARY) == 0);

	va_list args;
	va_start(args, format);

	if(flags & FT_C)
	{
		vfprintf((FILE*)file, format, args);
	}
	else if(flags & FT_ZIP
#if ANKI_OS == ANKI_OS_ANDROID
		|| flags & FT_SPECIAL
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
	ANKI_ASSERT(file);
	ANKI_ASSERT(flags);

	if(flags & FT_C)
	{
		if(fseek((FILE*)file, offset, origin) != 0)
		{
			throw ANKI_EXCEPTION("fseek() failed");
		}
	}
	else if(flags & FT_ZIP)
	{
		// Rewind if needed
		if(origin == SO_BEGINNING)
		{
			if(unzCloseCurrentFile(file)
				|| unzOpenCurrentFile(file))
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
	else if(flags & FT_SPECIAL)
	{
		if(AAsset_seek((AAsset*)file, offset, origin) != 0)
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
File::Endianness File::getMachineEndianness()
{
	I32 num = 1;
	if(*(char*)&num == 1)
	{
		return E_LITTLE_ENDIAN;
	}
	else
	{
		return E_BIG_ENDIAN;
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
