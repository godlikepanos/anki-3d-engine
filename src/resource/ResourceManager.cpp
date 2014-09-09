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

namespace anki {

//==============================================================================
ResourceManager::ResourceManager(Initializer& init)
:	m_gl(init.m_gl),
	m_alloc(HeapMemoryPool(init.m_allocCallback, init.m_allocCallbackData)),
	m_tmpAlloc(StackMemoryPool(
		init.m_allocCallback, init.m_allocCallbackData, 
		init.m_tempAllocatorMemorySize)),
	m_cacheDir(m_alloc),
	m_dataDir(init.m_cacheDir, m_alloc)
{
	// Init the data path
	//
	if(getenv("ANKI_DATA_PATH"))
	{
		m_dataDir = getenv("ANKI_DATA_PATH");
		m_dataDir += "/";
		ANKI_LOGI("Data path: %s", &m_dataDir[0]);
	}
	else
	{
		// Assume working directory
#if ANKI_OS == ANKI_OS_ANDROID
		m_dataDir = "$";
#else
		m_dataDir = "./";
#endif
	}

	// Init some constants
	//
	m_maxTextureSize = init.m_config->get("maxTextureSize");
	m_textureAnisotropy = init.m_config->get("textureAnisotropy");

	// Init type resource managers
	//
#define ANKI_RESOURCE(type_) \
	TypeResourceManager<type_, ResourceManager>::init(m_alloc);

	ANKI_RESOURCE(Animation)
	ANKI_RESOURCE(TextureResource)
	ANKI_RESOURCE(ProgramResource)
	ANKI_RESOURCE(Material)
	ANKI_RESOURCE(Mesh)
	ANKI_RESOURCE(BucketMesh)
	ANKI_RESOURCE(Skeleton)
	ANKI_RESOURCE(ParticleEmitterResource)
	ANKI_RESOURCE(Model)

#undef ANKI_RESOURCE
}

//==============================================================================
TempResourceString ResourceManager::fixResourceFilename(
	const CString& filename) const
{
	TempResourceString newFname(m_tmpAlloc);

	// If the filename is in cache then dont append the data path
	if(filename.find(m_cacheDir.toCString()) != TempResourceString::NPOS)
	{
		newFname = filename;
	}
	else
	{
		newFname = TempResourceString(m_dataDir.toCString(), m_tmpAlloc) 
			+ filename;
	}

	return newFname;
}

} // end namespace anki
