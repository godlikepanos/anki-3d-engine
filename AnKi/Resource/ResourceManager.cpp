// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Resource/ShaderProgramResourceSystem.h>
#include <AnKi/Resource/AnimationResource.h>
#include <AnKi/Resource/AccelerationStructureScratchAllocator.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/CVarSet.h>

#include <AnKi/Resource/MaterialResource.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Resource/CpuMeshResource.h>
#include <AnKi/Resource/ScriptResource.h>
#include <AnKi/Resource/DummyResource.h>
#include <AnKi/Resource/ParticleEmitterResource2.h>
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

	AsyncLoader::freeSingleton();
	ShaderProgramResourceSystem::freeSingleton();
	TransferGpuAllocator::freeSingleton();
	ResourceFilesystem::freeSingleton();

#define ANKI_INSTANTIATE_RESOURCE(className) \
	static_cast<TypeData<className>&>(m_allTypes).m_entries.destroy(); \
	static_cast<TypeData<className>&>(m_allTypes).m_map.destroy();
#include <AnKi/Resource/Resources.def.h>

	AccelerationStructureScratchAllocator::freeSingleton();

	ResourceMemoryPool::freeSingleton();
}

Error ResourceManager::init(AllocAlignedCallback allocCallback, void* allocCallbackData)
{
	ANKI_RESOURCE_LOGI("Initializing resource manager");

	ResourceMemoryPool::allocateSingleton(allocCallback, allocCallbackData);

	ResourceFilesystem::allocateSingleton();
	ANKI_CHECK(ResourceFilesystem::getSingleton().init());

	// Init the thread
	AsyncLoader::allocateSingleton();

	TransferGpuAllocator::allocateSingleton();
	ANKI_CHECK(TransferGpuAllocator::getSingleton().init(g_cvarRsrcTransferScratchMemorySize));

	// Init the programs
	ShaderProgramResourceSystem::allocateSingleton();
	ANKI_CHECK(ShaderProgramResourceSystem::getSingleton().init());

	if(GrManager::getSingleton().getDeviceCapabilities().m_rayTracing)
	{
		AccelerationStructureScratchAllocator::allocateSingleton();
	}

#if ANKI_WITH_EDITOR
	m_trackFileUpdateTimes = g_cvarRsrcTrackFileUpdates;
#endif

	return Error::kNone;
}

template<typename T>
Error ResourceManager::loadResource(CString filename, ResourcePtr<T>& out, Bool async)
{
	ANKI_ASSERT(!out.isCreated() && "Already loaded");

	TypeData<T>& type = static_cast<TypeData<T>&>(m_allTypes);

	// Check if registered
	using EntryType = typename TypeData<T>::Entry;
	EntryType* entry;
	{
		RLockGuard lock(type.m_mtx);
		auto it = type.m_map.find(filename);
		entry = (it != type.m_map.getEnd()) ? &type.m_entries[*it] : nullptr;
	}

	if(!entry)
	{
		// Resource entry doesn't exist, create one

		WLockGuard lock(type.m_mtx);

		auto it = type.m_map.find(filename); // Search again
		if(it != type.m_map.getEnd())
		{
			entry = &type.m_entries[*it];
		}
		else
		{
			auto arrit = type.m_entries.emplace();
			type.m_map.emplace(filename, arrit.getArrayIndex());
			entry = &(*arrit);
		}
	}

	ANKI_ASSERT(entry);

	// Try to load the resource
	Error err = Error::kNone;
	T* rsrc = nullptr;
	{
		LockGuard lock(entry->m_mtx);

		if(entry->m_resources.getSize() == 0
#if ANKI_WITH_EDITOR
		   || entry->m_resources.getBack()->isObsolete()
#endif
		)
		{
			// Resource hasn't been loaded or it needs update, load it

			rsrc = newInstance<T>(ResourceMemoryPool::getSingleton(), filename, m_uuid.fetchAdd(1));

			// Increment the refcount in that case where async jobs increment it and decrement it in the scope of a load()
			rsrc->retain();

			err = rsrc->load(filename, async);

			// Decrement because of the increment happened a few lines above
			rsrc->release();

			if(err)
			{
				ANKI_RESOURCE_LOGE("Failed to load resource: %s", filename.cstr());
				deleteInstance(ResourceMemoryPool::getSingleton(), rsrc);
				rsrc = nullptr;
			}
			else
			{
				entry->m_resources.emplaceBack(rsrc);

#if ANKI_WITH_EDITOR
				if(m_trackFileUpdateTimes)
				{
					entry->m_fileUpdateTime = ResourceFilesystem::getSingleton().getFileUpdateTime(filename);
				}
#endif
			}
		}
		else
		{
			rsrc = entry->m_resources.getBack();
		}

		if(!err)
		{
			ANKI_ASSERT(rsrc);
			out.reset(rsrc);
		}
	}

	return err;
}

template<typename T>
void ResourceManager::freeResource(T* ptr)
{
	ANKI_ASSERT(ptr);
	ANKI_ASSERT(ptr->m_refcount.load() == 0);

	TypeData<T>& type = static_cast<TypeData<T>&>(m_allTypes);

	typename TypeData<T>::Entry* entry = nullptr;
	{
		RLockGuard lock(type.m_mtx);
		auto it = type.m_map.find(ptr->m_fname);
		ANKI_ASSERT(it != type.m_map.getEnd());
		entry = &type.m_entries[*it];
	}

	{
		LockGuard lock(entry->m_mtx);

		auto it = entry->m_resources.getBegin();
		for(; it != entry->m_resources.getEnd(); ++it)
		{
			if(*it == ptr)
			{
				break;
			}
		}
		ANKI_ASSERT(it != entry->m_resources.getEnd());
		deleteInstance(ResourceMemoryPool::getSingleton(), *it);
		entry->m_resources.erase(it);
	}
}

// Instansiate
#define ANKI_INSTANTIATE_RESOURCE(className) \
	template Error ResourceManager::loadResource<className>(CString filename, ResourcePtr<className> & out, Bool async); \
	template void ResourceManager::freeResource<className>(className * ptr);
#include <AnKi/Resource/Resources.def.h>

#if ANKI_WITH_EDITOR
template<typename T>
void ResourceManager::refreshFileUpdateTimesInternal()
{
	TypeData<T>& type = static_cast<TypeData<T>&>(m_allTypes);

	WLockGuard lock(type.m_mtx);

	for(auto& entry : type.m_entries)
	{
		LockGuard lock(entry.m_mtx);

		if(entry.m_resources.getSize() == 0)
		{
			continue;
		}

		const U64 newTime = ResourceFilesystem::getSingleton().getFileUpdateTime(entry.m_resources[0]->getFilename());
		if(newTime != entry.m_fileUpdateTime)
		{
			ANKI_RESOURCE_LOGV("File updated, loaded resource now obsolete: %s", entry.m_resources[0]->getFilename().cstr());
			entry.m_fileUpdateTime = newTime;

			for(T* rsrc : entry.m_resources)
			{
				rsrc->m_isObsolete.store(1);
			}
		}
	}
}

void ResourceManager::refreshFileUpdateTimes()
{
	if(!m_trackFileUpdateTimes)
	{
		return;
	}

#	define ANKI_INSTANTIATE_RESOURCE(className) refreshFileUpdateTimesInternal<className>();
#	include <AnKi/Resource/Resources.def.h>
}
#endif // #if ANKI_WITH_EDITOR

} // end namespace anki
