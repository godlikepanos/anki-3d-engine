// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ResourceManager.h>
#include <anki/resource/AsyncLoader.h>
#include <anki/resource/ShaderProgramResourceSystem.h>
#include <anki/resource/AnimationResource.h>
#include <anki/util/Logger.h>
#include <anki/core/ConfigSet.h>

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
#include <anki/resource/CollisionResource.h>

namespace anki
{

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	m_cacheDir.destroy(m_alloc);
	m_alloc.deleteInstance(m_asyncLoader);
	m_alloc.deleteInstance(m_shaderProgramSystem);
	m_alloc.deleteInstance(m_transferGpuAlloc);
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
	m_maxTextureSize = init.m_config->getNumberU32("rsrc_maxTextureSize");
	m_dumpShaderSource = init.m_config->getBool("rsrc_dumpShaderSources");

	// Init type resource managers
#define ANKI_INSTANTIATE_RESOURCE(rsrc_, ptr_) TypeResourceManager<rsrc_>::init(m_alloc);
#define ANKI_INSTANSIATE_RESOURCE_DELIMITER()
#include <anki/resource/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_RESOURCE
#undef ANKI_INSTANSIATE_RESOURCE_DELIMITER

	// Init the thread
	m_asyncLoader = m_alloc.newInstance<AsyncLoader>();
	m_asyncLoader->init(m_alloc);

	m_transferGpuAlloc = m_alloc.newInstance<TransferGpuAllocator>();
	ANKI_CHECK(m_transferGpuAlloc->init(init.m_config->getNumberU32("rsrc_transferScratchMemorySize"), m_gr, m_alloc));

	// Init the programs
	m_shaderProgramSystem = m_alloc.newInstance<ShaderProgramResourceSystem>(m_cacheDir, m_gr, m_fs, m_alloc);
	ANKI_CHECK(m_shaderProgramSystem->init());

	return Error::NONE;
}

U64 ResourceManager::getAsyncTaskCompletedCount() const
{
	return m_asyncLoader->getCompletedTaskCount();
}

template<typename T>
Error ResourceManager::loadResource(const CString& filename, ResourcePtr<T>& out, Bool async)
{
	ANKI_ASSERT(!out.isCreated() && "Already loaded");

	Error err = Error::NONE;
	++m_loadRequestCount;

	T* const other = findLoadedResource<T>(filename);

	if(other)
	{
		// Found
		out.reset(other);
	}
	else
	{
		// Allocate ptr
		T* ptr = m_alloc.newInstance<T>(this);
		ANKI_ASSERT(ptr->getRefcount().load() == 0);

		// Populate the ptr. Use a block to cleanup temp_pool allocations
		auto& pool = m_tmpAlloc.getMemoryPool();

		{
			U allocsCountBefore = pool.getAllocationsCount();
			(void)allocsCountBefore;

			err = ptr->load(filename, async);
			if(err)
			{
				ANKI_RESOURCE_LOGE("Failed to load resource: %s", &filename[0]);
				m_alloc.deleteInstance(ptr);
				return err;
			}

			ANKI_ASSERT(pool.getAllocationsCount() == allocsCountBefore && "Forgot to deallocate");
		}

		ptr->setFilename(filename);
		ptr->setUuid(++m_uuid);

		// Reset the memory pool if no-one is using it.
		// NOTE: Check because resources load other resources
		if(pool.getAllocationsCount() == 0)
		{
			pool.reset();
		}

		// Register resource
		registerResource(ptr);
		out.reset(ptr);
	}

	return err;
}

// Instansiate the ResourceManager::loadResource()
#define ANKI_INSTANTIATE_RESOURCE(rsrc_, ptr_) \
	template Error ResourceManager::loadResource<rsrc_>(const CString& filename, ResourcePtr<rsrc_>& out, Bool async);
#define ANKI_INSTANSIATE_RESOURCE_DELIMITER()
#include <anki/resource/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_RESOURCE
#undef ANKI_INSTANSIATE_RESOURCE_DELIMITER

} // end namespace anki
