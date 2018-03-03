// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ResourceManager.h>
#include <anki/resource/AsyncLoader.h>
#include <anki/resource/AnimationResource.h>
#include <anki/resource/MaterialResource.h>
#include <anki/resource/MeshResource.h>
#include <anki/resource/ModelResource.h>
#include <anki/resource/ScriptResource.h>
#include <anki/resource/DummyResource.h>
#include <anki/resource/ParticleEmitterResource.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/GenericResource.h>
#include <anki/resource/TextureAtlasResource.h>
#include <anki/resource/ShaderProgramResource.h>
#include <anki/util/Logger.h>
#include <anki/misc/ConfigSet.h>
#include <anki/gr/ShaderCompiler.h>

namespace anki
{

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	m_cacheDir.destroy(m_alloc);
	m_alloc.deleteInstance(m_asyncLoader);
	m_alloc.deleteInstance(m_transferGpuAlloc);
	m_alloc.deleteInstance(m_shaderCompiler);
}

Error ResourceManager::init(ResourceManagerInitInfo& init)
{
	m_gr = init.m_gr;
	m_physics = init.m_physics;
	m_fs = init.m_resourceFs;
	m_alloc = ResourceAllocator<U8>(init.m_allocCallback, init.m_allocCallbackData);

	m_tmpAlloc = TempResourceAllocator<U8>(init.m_allocCallback, init.m_allocCallbackData, 10 * 1024 * 1024);

	m_cacheDir.create(m_alloc, init.m_cacheDir);

	// Init some constants
	//
	m_maxTextureSize = init.m_config->getNumber("rsrc.maxTextureSize");
	m_textureAnisotropy = init.m_config->getNumber("rsrc.textureAnisotropy");

// Init type resource managers
//
#define ANKI_INSTANTIATE_RESOURCE(rsrc_, ptr_) TypeResourceManager<rsrc_>::init(m_alloc);

#define ANKI_INSTANSIATE_RESOURCE_DELIMITER()

#include <anki/resource/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_RESOURCE
#undef ANKI_INSTANSIATE_RESOURCE_DELIMITER

	// Init the thread
	m_asyncLoader = m_alloc.newInstance<AsyncLoader>();
	m_asyncLoader->init(m_alloc);

	m_transferGpuAlloc = m_alloc.newInstance<TransferGpuAllocator>();
	ANKI_CHECK(m_transferGpuAlloc->init(init.m_config->getNumber("rsrc.transferScratchMemorySize"), m_gr, m_alloc));

	m_shaderCompiler = m_alloc.newInstance<ShaderCompilerCache>(m_alloc, m_cacheDir.toCString());

	return Error::NONE;
}

U64 ResourceManager::getAsyncTaskCompletedCount() const
{
	return m_asyncLoader->getCompletedTaskCount();
}

} // end namespace anki
