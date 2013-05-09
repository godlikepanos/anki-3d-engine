#include "anki/util/Filesystem.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include <fstream>
#include <cstring>
#include <cstdarg>

namespace anki {

//==============================================================================
// File                                                                        =
//==============================================================================

//==============================================================================
File::~File()
{
	if(file)
	{
		switch(fileType)
		{
		case FT_C:
			fclose((FILE*)file);
			break;
		case FT_ZIP:
			ANKI_ASSERT(0 && "Not implemented");
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
}

//==============================================================================
void File::open(const char* filename, U8 flags)
{
	const char* openMode;

	if(flags & OF_READ)
	{
		openMode = "r";
	}
	else if(flags & OF_WRITE)
	{
		openMode = "w";
	}
	else if(flags & OF_APPEND)
	{
		openMode = "a+";
	}
	else
	{
		ANKI_ASSERT(0);
		openMode = nullptr;
	}
	openFlags = flags;

	// Open
	file = (FILE*)fopen(filename, openMode);
	if(!file)
	{
		throw ANKI_EXCEPTION("Failed to open file");
	}
	fileType = FT_C;
}

//==============================================================================
void File::close()
{
	ANKI_ASSERT(file);

	switch(fileType)
	{
	case FT_C:
		fclose((FILE*)file);
		break;
	case FT_ZIP:
		ANKI_ASSERT(0 && "Not implemented");
		break;
	default:
		ANKI_ASSERT(0);
	}

	file = nullptr;
}

//==============================================================================
PtrSize File::read(void* buff, PtrSize size)
{
	ANKI_ASSERT(file);
	ANKI_ASSERT(buff);
	ANKI_ASSERT(size > 0);

	PtrSize readSize = 0;

	switch(fileType)
	{
	case FT_C:
		readSize = fread(buff, 1, size, (FILE*)file);
		break;
	case FT_ZIP:
		ANKI_ASSERT(0 && "Not implemented");
		break;
	default:
		ANKI_ASSERT(0);
	}

	return readSize;
}

//==============================================================================
PtrSize File::write(void* buff, PtrSize size)
{
	ANKI_ASSERT(file);
	ANKI_ASSERT(buff);
	ANKI_ASSERT(size > 0);

	PtrSize writeSize = 0;

	switch(fileType)
	{
	case FT_C:
		writeSize = fwrite(buff, 1, size, (FILE*)file);
		break;
	case FT_ZIP:
		ANKI_ASSERT(0 && "Not implemented");
		break;
	default:
		ANKI_ASSERT(0);
	}

	return writeSize;
}

//==============================================================================
void File::writeString(const char* format, ...)
{
	ANKI_ASSERT(file);
	ANKI_ASSERT(format);

	va_list args;
	va_start(args, format);

	switch(fileType)
	{
	case FT_C:
		vfprintf((FILE*)file, format, args);
		break;
	case FT_ZIP:
		ANKI_ASSERT(0 && "Not implemented");
		break;
	default:
		ANKI_ASSERT(0);
	}

	va_end(args);
}

//==============================================================================
// Functions                                                                   =
//==============================================================================

//==============================================================================
const char* getFileExtension(const char* filename)
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

//==============================================================================
std::string readFile(const char* filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		throw ANKI_EXCEPTION("Cannot open file: " + filename);
	}

	return std::string((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
}

//==============================================================================
StringList readFileLines(const char* filename)
{
	ANKI_ASSERT(filename);
	std::ifstream ifs(filename);
	if(!ifs.is_open())
	{
		throw ANKI_EXCEPTION("Cannot open file: " + filename);
	}

	StringList lines;
	std::string temp;
	while(getline(ifs, temp))
	{
		lines.push_back(temp);
	}
	return lines;
}

} // end namespace anki
