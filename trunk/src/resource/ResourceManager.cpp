// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ResourceManager.h"
#include "anki/resource/Animation.h"
#include "anki/resource/Material.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/Model.h"
#include "anki/resource/ProgramResource.h"
#include "anki/resource/ParticleEmitterResource.h"
#include "anki/resource/TextureResource.h"
#include "anki/core/Logger.h"
#include "anki/misc/ConfigSet.h"
#include <cstring>

namespace anki {

//==============================================================================
ResourceManager::ResourceManager(App* app, const ConfigSet& config)
:	m_app(app),
	m_alloc(HeapMemoryPool(
		m_app->getAllocationCallback(), m_app->getAllocationCallbackData())),
	m_tmpAlloc(StackMemoryPool(m_app->getAllocationCallback(), 
		m_app->getAllocationCallbackData(), 1024 * 1024))
{
	// Init the data path
	//
	if(getenv("ANKI_DATA_PATH"))
	{
		m_dataDir = ResourceString(getenv("ANKI_DATA_PATH"), m_alloc);
		m_dataDir += "/";
		ANKI_LOGI("Data path: %s", m_dataDir.c_str());
	}
	else
	{
		// Assume working directory
#if ANKI_OS == ANKI_OS_ANDROID
		m_dataDir = ResourceString("$", m_alloc);
#else
		m_dataDir = ResourceString("./", m_alloc);
#endif
	}

	// Init cache dir
	//
	m_cacheDir = ResourceString(cacheDir, m_alloc);

	// Init some constants
	//
	m_maxTextureSize = config.get("maxTextureSize");
	m_textureAnisotropy = config.get("textureAnisotropy");

	// Init type resource managers
	//
	TypeResourceManager<Animation, ResourceManager>::init(m_alloc);
	TypeResourceManager<Material, ResourceManager>::init(m_alloc);
	TypeResourceManager<Mesh, ResourceManager>::init(m_alloc);
	TypeResourceManager<Model, ResourceManager>::init(m_alloc);
	TypeResourceManager<BucketMesh, ResourceManager>::init(m_alloc);
	TypeResourceManager<ProgramResource, ResourceManager>::init(m_alloc);
	TypeResourceManager<ParticleEmitterResource, ResourceManager>::init(
		m_alloc);
}

//==============================================================================
TempResourceString ResourceManager::fixResourceFilename(
	const char* filename) const
{
	TempResourceString newFname(m_tmpAlloc);

	// If the filename is in cache then dont append the data path
	if(std::strstr(filename, m_cacheDir.c_str()) != nullptr)
	{
		newFname = filename;
	}
	else
	{
		newFname = TempResourceString(m_dataDir.c_str(), m_tmpAlloc) + filename;
	}

	return newFname;
}

} // end namespace anki
