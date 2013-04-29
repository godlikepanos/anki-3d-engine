#include "anki/util/Filesystem.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include <fstream>
#include <cstring>

namespace anki {

//==============================================================================
// File                                                                        =
//==============================================================================

//==============================================================================
void File::open(const char* filename, U8 flags)
{
	const char* openMode;

	if(flags & READ)
	{
		openMode = "r";
	}
	else if(flags & WRITE)
	{
		openMode = "w";
	}
	else if(flags & APPEND)
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
	fileType = C_FILE;
}

//==============================================================================
PtrSize File::read(void* buff, PtrSize size)
{
	ANKI_ASSERT(file);
	ANKI_ASSERT(buff);
	PtrSize readSize = 0;

	switch(fileType)
	{
	case C_FILE:
		readSize = fread(buff, 1, size, (FILE*)file);
		break;
	case ZIP_FILE:
		break;
	}

	return readSize;
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
