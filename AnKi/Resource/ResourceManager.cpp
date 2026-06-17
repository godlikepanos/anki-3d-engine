// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Resource/ShaderProgramResourceSystem.h>
#include <AnKi/Resource/AnimationResource.h>
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
	ResourceFilesystem::freeSingleton();

#define ANKI_INSTANTIATE_RESOURCE(className) \
	static_cast<TypeData<className>&>(m_allTypes).m_resources.destroy(); \
	static_cast<TypeData<className>&>(m_allTypes).m_map.destroy();
#include <AnKi/Resource/Resources.def.h>

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

	// Init the programs
	ShaderProgramResourceSystem::allocateSingleton();
	ANKI_CHECK(ShaderProgramResourceSystem::getSingleton().init());

#if ANKI_WITH_EDITOR
	m_trackFileUpdateTimes = g_cvarRsrcTrackFileUpdates;
#endif

	return Error::kNone;
}

template<typename T>
Error ResourceManager::loadResource(CString filename, IntrusiveNoDelPtr<T>& out, Bool async)
{
	ANKI_ASSERT(!out.isCreated() && "Already loaded");

	TypeData<T>& type = static_cast<TypeData<T>&>(m_allTypes);

	// Try to find the resource
	using Rsrc = typename TypeData<T>::Resource;
	Rsrc* rsrc = nullptr;
	U32 resourceArrayIdx = kMaxU32;
	{
		RLockGuard lock(type.m_mtx);

		auto it = type.m_map.find(filename);

		if(it != type.m_map.getEnd())
		{
			rsrc = &type.m_resources[*it];
			resourceArrayIdx = *it;
		}
	}

	// Resource not found, create it
	if(!rsrc)
	{
		WLockGuard lock(type.m_mtx);

		// Check again...
		auto it = type.m_map.find(filename);

		// ... and create
		if(it != type.m_map.getEnd())
		{
			rsrc = &type.m_resources[*it];
			resourceArrayIdx = *it;
		}
		else
		{
			auto it = type.m_resources.emplace();
			resourceArrayIdx = it.getArrayIndex();
			rsrc = &(*it);

			type.m_map.emplace(filename, resourceArrayIdx);
		}
	}

	ANKI_ASSERT(rsrc && resourceArrayIdx < kMaxU32);

	LockGuard lock(rsrc->m_mtx);

	T* ver = (rsrc->m_versions.getSize()) ? rsrc->m_versions.getBack() : nullptr;

	if(ver
#if ANKI_WITH_EDITOR
	   && !ver->isObsolete()
#endif
	)
	{
		// We are done
		out.reset(ver);
		return Error::kNone;
	}

	// Versioned resource hasn't been loaded or it needs update, load it

	ver = newInstance<T>(ResourceMemoryPool::getSingleton(), filename, m_uuid.fetchAdd(1));
	ver->m_versionResourceIdx = resourceArrayIdx;

	// Increment the refcount in that case where async jobs increment it and decrement it in the scope of a load()
	ver->retain();

	const Error err = ver->load(filename, async);

	if(err)
	{
		ANKI_RESOURCE_LOGE("Failed to load resource: %s", filename.cstr());
		deleteInstance(ResourceMemoryPool::getSingleton(), ver);
	}
	else
	{
		rsrc->m_versions.emplaceBack(ver);

#if ANKI_WITH_EDITOR
		if(m_trackFileUpdateTimes)
		{
			rsrc->m_fileUpdateTime = ResourceFilesystem::getSingleton().getFileUpdateTime(filename);
		}
#endif

		out.reset(ver);

		// Decrement because of the increment happened a few lines above.
		ver->release();
	}

	return err;
}

template<typename T>
void ResourceManager::freeResource(U32 uuid, U32 versionedResourceIdx)
{
	TypeData<T>& type = static_cast<TypeData<T>&>(m_allTypes);

	using Rsrc = typename TypeData<T>::Resource;

	Rsrc* rsrc;
	{
		RLockGuard lock(type.m_mtx);
		rsrc = &type.m_resources[versionedResourceIdx];
	}

	T* toDelete = nullptr;
	{
		LockGuard lock(rsrc->m_mtx);

		auto it = rsrc->m_versions.getBegin();
		for(; it != rsrc->m_versions.getEnd(); ++it)
		{
			if((*it)->m_uuid == uuid)
			{
				break;
			}
		}

		// Check the refcount because there can be a race condition where:
		// - ResourceObject::release() is called, refcount reaches 0
		// - This function is called but didn't got the lock
		// - loadResource is called and the resource is found. Refcount goes to 1
		// - ResourceObject::release() is called, refcount reaches 0, objects delets
		// - Then this function finaly gets the lock, the object is already deleted... boom
		if(it != rsrc->m_versions.getEnd() && (*it)->m_refcount.load() == 0)
		{
			toDelete = *it;
			rsrc->m_versions.erase(it);
		}
	}

	// Now you can delete outside any locks
	deleteInstance(ResourceMemoryPool::getSingleton(), toDelete);
}

// Instansiate
#define ANKI_INSTANTIATE_RESOURCE(className) \
	template Error ResourceManager::loadResource<className>(CString filename, IntrusiveNoDelPtr<className> & out, Bool async); \
	template void ResourceManager::freeResource<className>(U32, U32);
#include <AnKi/Resource/Resources.def.h>

#if ANKI_WITH_EDITOR
template<typename T>
void ResourceManager::refreshFileUpdateTimesInternal()
{
	TypeData<T>& type = static_cast<TypeData<T>&>(m_allTypes);

	WLockGuard lock(type.m_mtx);

	for(auto& entry : type.m_resources)
	{
		LockGuard lock(entry.m_mtx);

		if(entry.m_versions.getSize() == 0)
		{
			continue;
		}

		const U64 newTime = ResourceFilesystem::getSingleton().getFileUpdateTime(entry.m_versions[0]->getFilename());
		if(newTime != entry.m_fileUpdateTime)
		{
			ANKI_RESOURCE_LOGV("File updated, loaded resource now obsolete: %s", entry.m_versions[0]->getFilename().cstr());
			entry.m_fileUpdateTime = newTime;

			for(T* rsrc : entry.m_versions)
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
