// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ResourceManager.h"
#include "anki/core/Logger.h"
#include "anki/core/App.h"
#include "anki/misc/ConfigSet.h"
#include <cstring>

namespace anki {

//==============================================================================
ResourceManager::ResourceManager(const ConfigSet& config)
{
	if(getenv("ANKI_DATA_PATH"))
	{
		m_dataPath = getenv("ANKI_DATA_PATH");
#if ANKI_POSIX
		m_dataPath += "/";
#else
		m_dataPath = "\\";
#endif
		ANKI_LOGI("Data path: %s", m_dataPath.c_str());
	}
	else
	{
		// Assume working directory
#if ANKI_OS == ANKI_OS_ANDROID
		m_dataPath = "$";
#else
#	if ANKI_POSIX
		m_dataPath = "./";
#	else
		m_dataPath = ".\\";
#	endif
#endif
	}

	m_maxTextureSize = config.get("maxTextureSize");
	m_textureAnisotropy = config.get("textureAnisotropy");
}

//==============================================================================
String ResourceManager::fixResourcePath(const char* filename) const
{
	String newFname;

	// If the filename is in cache then dont append the data path
	const char* cachePath = AppSingleton::get().getCachePath().c_str();
	if(strstr(filename, cachePath) != nullptr)
	{
		newFname = filename;
	}
	else
	{
		newFname = m_dataPath + filename;
	}

	return newFname;
}

} // end namespace anki
