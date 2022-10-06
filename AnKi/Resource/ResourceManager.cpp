// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Resource/ShaderProgramResourceSystem.h>
#include <AnKi/Resource/AnimationResource.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Core/ConfigSet.h>

#include <AnKi/Resource/MaterialResource.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Resource/CpuMeshResource.h>
#include <AnKi/Resource/ModelResource.h>
#include <AnKi/Resource/ScriptResource.h>
#include <AnKi/Resource/DummyResource.h>
#include <AnKi/Resource/ParticleEmitterResource.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Resource/GenericResource.h>
#include <AnKi/Resource/ImageAtlasResource.h>
#include <AnKi/Resource/ShaderProgramResource.h>
#include <AnKi/Resource/SkeletonResource.h>

namespace anki {

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	ANKI_RESOURCE_LOGI("Destroying resource manager");

	deleteInstance(m_pool, m_asyncLoader);
	deleteInstance(m_pool, m_shaderProgramSystem);
	deleteInstance(m_pool, m_transferGpuAlloc);
}

Error ResourceManager::init(ResourceManagerInitInfo& init)
{
	ANKI_RESOURCE_LOGI("Initializing resource manager");

	m_gr = init.m_gr;
	m_physics = init.m_physics;
	m_fs = init.m_resourceFs;
	m_config = init.m_config;
	m_unifiedGometryMemoryPool = init.m_unifiedGometryMemoryPool;

	m_pool.init(init.m_allocCallback, init.m_allocCallbackData, "Resources");
	m_tmpPool.init(init.m_allocCallback, init.m_allocCallbackData, 10_MB);

	// Init type resource managers
#define ANKI_INSTANTIATE_RESOURCE(rsrc_, ptr_) TypeResourceManager<rsrc_>::init(&m_pool);
#define ANKI_INSTANSIATE_RESOURCE_DELIMITER()
#include <AnKi/Resource/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_RESOURCE
#undef ANKI_INSTANSIATE_RESOURCE_DELIMITER

	// Init the thread
	m_asyncLoader = newInstance<AsyncLoader>(m_pool);
	m_asyncLoader->init(&m_pool);

	m_transferGpuAlloc = newInstance<TransferGpuAllocator>(m_pool);
	ANKI_CHECK(m_transferGpuAlloc->init(m_config->getRsrcTransferScratchMemorySize(), m_gr, &m_pool));

	// Init the programs
	m_shaderProgramSystem = newInstance<ShaderProgramResourceSystem>(m_pool, &m_pool);
	ANKI_CHECK(m_shaderProgramSystem->init(*m_fs, *m_gr));

	return Error::kNone;
}

U64 ResourceManager::getAsyncTaskCompletedCount() const
{
	return m_asyncLoader->getCompletedTaskCount();
}

template<typename T>
Error ResourceManager::loadResource(const CString& filename, ResourcePtr<T>& out, Bool async)
{
	ANKI_ASSERT(!out.isCreated() && "Already loaded");

	Error err = Error::kNone;
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
		T* ptr = newInstance<T>(m_pool, this);
		ANKI_ASSERT(ptr->getRefcount() == 0);

		// Increment the refcount in that case where async jobs increment it and decrement it in the scope of a load()
		ptr->retain();

		// Populate the ptr. Use a block to cleanup temp_pool allocations
		StackMemoryPool& tmpPool = m_tmpPool;

		{
			[[maybe_unused]] const U allocsCountBefore = tmpPool.getAllocationCount();

			err = ptr->load(filename, async);
			if(err)
			{
				ANKI_RESOURCE_LOGE("Failed to load resource: %s", &filename[0]);
				deleteInstance(m_pool, ptr);
				return err;
			}

			ANKI_ASSERT(tmpPool.getAllocationCount() == allocsCountBefore && "Forgot to deallocate");
		}

		ptr->setFilename(filename);
		ptr->setUuid(++m_uuid);

		// Reset the memory pool if no-one is using it.
		// NOTE: Check because resources load other resources
		if(tmpPool.getAllocationCount() == 0)
		{
			tmpPool.reset();
		}

		// Register resource
		registerResource(ptr);
		out.reset(ptr);

		// Decrement because of the increment happened a few lines above
		ptr->release();
	}

	return err;
}

// Instansiate the ResourceManager::loadResource()
#define ANKI_INSTANTIATE_RESOURCE(rsrc_, ptr_) \
	template Error ResourceManager::loadResource<rsrc_>(const CString& filename, ResourcePtr<rsrc_>& out, Bool async);
#define ANKI_INSTANSIATE_RESOURCE_DELIMITER()
#include <AnKi/Resource/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_RESOURCE
#undef ANKI_INSTANSIATE_RESOURCE_DELIMITER

} // end namespace anki
