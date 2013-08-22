#include "anki/resource/ResourceManager.h"
#include "anki/core/Logger.h"
#include "anki/core/App.h"
#include <cstring>

namespace anki {

//==============================================================================
ResourceManager::ResourceManager()
{
	if(getenv("ANKI_DATA_PATH"))
	{
		dataPath = getenv("ANKI_DATA_PATH");
#if ANKI_POSIX
		dataPath += "/";
#else
		dataPath = "\\";
#endif
		ANKI_LOGI("Data path: %s", dataPath.c_str());
	}
	else
	{
		// Assume working directory
#if ANKI_POSIX
		dataPath = "./";
#else
		dataPath = ".\\";
#endif
	}
}

//==============================================================================
std::string ResourceManager::fixResourcePath(const char* filename) const
{
	std::string newFname;

	// If the filename is in cache then dont append the data path
	const char* cachePath = AppSingleton::get().getCachePath().c_str();
	if(strstr(filename, cachePath) != nullptr)
	{
		newFname = filename;
	}
	else
	{
		newFname = ResourceManagerSingleton::get().getDataPath() + filename;
	}

	return newFname;
}

} // end namespace anki
