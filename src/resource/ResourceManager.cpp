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
#include "anki/resource/GenericResource.h"
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
	m_shadersPrependedSource.destroy(m_alloc);
	m_alloc.deleteInstance(m_asyncLoader);
}

//==============================================================================
Error ResourceManager::create(Initializer& init)
{
	Error err = ErrorCode::NONE;

	m_gr = init.m_gr;
	m_physics = init.m_physics;
	m_fs = init.m_resourceFs;
	m_alloc = ResourceAllocator<U8>(
		init.m_allocCallback, init.m_allocCallbackData);

	m_tmpAlloc = TempResourceAllocator<U8>(
		init.m_allocCallback, init.m_allocCallbackData,
		init.m_tempAllocatorMemorySize);

	m_cacheDir.create(m_alloc, init.m_cacheDir);

	// Init some constants
	//
	m_maxTextureSize = init.m_config->getNumber("maxTextureSize");
	m_textureAnisotropy = init.m_config->getNumber("textureAnisotropy");

	// Init type resource managers
	//
#define ANKI_RESOURCE(type_) \
	TypeResourceManager<type_>::init(m_alloc);

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
	ANKI_RESOURCE(CollisionResource)
	ANKI_RESOURCE(GenericResource)

#undef ANKI_RESOURCE

	// Init the thread
	m_asyncLoader = m_alloc.newInstance<AsyncLoader>();
	err = m_asyncLoader->create(m_alloc);

	return err;
}

} // end namespace anki
