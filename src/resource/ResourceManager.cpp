// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ResourceManager.h"
#include "anki/resource/AsyncLoader.h"
#include "anki/resource/Animation.h"
#include "anki/resource/Material.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/Model.h"
#include "anki/resource/Script.h"
#include "anki/resource/DummyRsrc.h"
#include "anki/resource/ShaderResource.h"
#include "anki/resource/ParticleEmitterResource.h"
#include "anki/resource/TextureResource.h"
#include "anki/util/Logger.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
ResourceManager::ResourceManager()
{}

//==============================================================================
ResourceManager::~ResourceManager()
{
	m_cacheDir.destroy(m_alloc);
	m_dataDir.destroy(m_alloc);
	m_alloc.deleteInstance(m_asyncLoader);
}

//==============================================================================
Error ResourceManager::create(Initializer& init)
{
	Error err = ErrorCode::NONE;

	m_gr = init.m_gr;
	m_physics = init.m_physics;
	m_alloc = ResourceAllocator<U8>(
		init.m_allocCallback, init.m_allocCallbackData);

	m_tmpAlloc = TempResourceAllocator<U8>(
		init.m_allocCallback, init.m_allocCallbackData, 
		init.m_tempAllocatorMemorySize);

	m_cacheDir.create(m_alloc, init.m_cacheDir);

	// Init the data path
	//
	if(getenv("ANKI_DATA_PATH"))
	{
		m_dataDir.sprintf(m_alloc, "%s/", getenv("ANKI_DATA_PATH"));
		ANKI_LOGI("Data path: %s", &m_dataDir[0]);
	}
	else
	{
		// Assume working directory
#if ANKI_OS == ANKI_OS_ANDROID
		m_dataDir.create(m_alloc, "$");
#else
		m_dataDir.create(m_alloc, "./");
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
	ANKI_RESOURCE(ShaderResource)
	ANKI_RESOURCE(Material)
	ANKI_RESOURCE(Mesh)
	ANKI_RESOURCE(BucketMesh)
	ANKI_RESOURCE(Skeleton)
	ANKI_RESOURCE(ParticleEmitterResource)
	ANKI_RESOURCE(Model)
	ANKI_RESOURCE(Script)
	ANKI_RESOURCE(DummyRsrc)

#undef ANKI_RESOURCE

	// Init the thread
	m_asyncLoader = m_alloc.newInstance<AsyncLoader>();
	if(m_asyncLoader == nullptr)
	{
		return ErrorCode::OUT_OF_MEMORY;
	}

	err = m_asyncLoader->create(m_alloc);

	return err;
}

//==============================================================================
void ResourceManager::fixResourceFilename(
	const CString& filename,
	StringAuto& out) const
{
	// If the filename is in cache then dont append the data path
	if(filename.find(m_cacheDir.toCString()) != String::NPOS)
	{
		out.create(filename);
	}
	else
	{
		out.sprintf("%s%s", &m_dataDir[0], &filename[0]);
	}
}

} // end namespace anki
