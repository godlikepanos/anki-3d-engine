#include "anki/util/File.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
//#include <ftw.h>
#include <cerrno>

namespace anki {

//==============================================================================
// File                                                                        =
//==============================================================================

//==============================================================================
Bool File::fileExists(const char* filename)
{
	ANKI_ASSERT(filename);
	struct stat s;
	if(stat(filename, &s) == 0)
	{
		return S_ISREG(s.st_mode);
	}
	else
	{
		return false;
	}
}

//==============================================================================
// Functions                                                                   =
//==============================================================================

//==============================================================================
Bool directoryExists(const char* filename)
{
	ANKI_ASSERT(filename);
	struct stat s;
	if(stat(filename, &s) == 0)
	{
		return S_ISDIR(s.st_mode);
	}
	else
	{
		return false;
	}
}

//==============================================================================
static int rmDirCallback(const char* fpath, const struct stat* /*sb*/, 
	int /*typeflag*/, struct FTW* /*ftwbuf*/)
{
	int rv = remove(fpath);

	if(rv)
	{
		throw ANKI_EXCEPTION(strerror(errno) + ": " + fpath);
	}

	return rv;
}

void removeDirectory(const char* dir)
{
	if(nftw(dir, rmDirCallback, 64, FTW_DEPTH | FTW_PHYS))
	{
		throw ANKI_EXCEPTION(strerror(errno) + ": " + dir);
	}
}

//==============================================================================
void createDirectory(const char* dir)
{
	if(directoryExists(dir))
	{
		return;
	}

	if(mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
	{
		throw ANKI_EXCEPTION(strerror(errno) + ": " + dir);
	}
}

} // end namespace anki
