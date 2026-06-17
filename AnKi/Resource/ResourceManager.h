// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/HashMap.h>
#include <AnKi/Util/BlockArray.h>

namespace anki {

// Forward
class GrManager;
class PhysicsWorld;
class ResourceManager;
class AsyncLoader;
class ResourceManagerModel;
class ShaderCompilerCache;
class ShaderProgramResourceSystem;
class ResourceObject;

#if ANKI_WITH_EDITOR
ANKI_CVAR(BoolCVar, Rsrc, TrackFileUpdates, false, "If true the resource manager is able to track file update times")
#endif

// Resource manager. It holds a few global variables
class ResourceManager : public MakeSingleton<ResourceManager>
{
	friend class ResourceObject;

	template<typename>
	friend class MakeSingleton;

public:
	Error init(AllocAlignedCallback allocCallback, void* allocCallbackData);

	// Load a resource.
	// Note: Thread-safe against itself, freeResource() and refreshFileUpdateTimes()
	template<typename T>
	Error loadResource(CString filename, IntrusiveNoDelPtr<T>& out, Bool async = true);

#if ANKI_WITH_EDITOR
	// Iterate all loaded resource and check if the files have been updated since they were loaded.
	// Note: Thread-safe against itself, loadResource() and freeResource()
	void refreshFileUpdateTimes();
#endif

private:
	template<typename Type>
	class TypeData
	{
	public:
		class Resource
		{
		public:
			DynamicArray<Type*> m_versions; // Hosts multiple versions of a resource. The last element is the newest
			SpinLock m_mtx;
#if ANKI_WITH_EDITOR
			U64 m_fileUpdateTime = 0;
#endif
		};

		ResourceHashMap<CString, U32> m_map; // Filename to index in the m_resources

		// Once a Resource is created in that array it will never be destroyed until shutdown. That is fine because it's the Resource::m_versions
		// that holds the actual resource
		ResourceBlockArray<Resource> m_resources;

		RWMutex m_mtx;

		~TypeData()
		{
			for([[maybe_unused]] const Resource& v : m_resources)
			{
				ANKI_ASSERT(v.m_versions.getSize() == 0 && "Forgot to release some resource");
			}
		}
	};

	class AllTypeData:
#define ANKI_INSTANTIATE_RESOURCE(className) \
public \
	TypeData<className>
#define ANKI_INSTANSIATE_RESOURCE_DELIMITER() ,
#include <AnKi/Resource/Resources.def.h>
	{
	};

	AllTypeData m_allTypes;

	Atomic<U32> m_uuid = {1};

#if ANKI_WITH_EDITOR
	Bool m_trackFileUpdateTimes = false;
#endif

	ResourceManager();

	~ResourceManager();

#if ANKI_WITH_EDITOR
	template<typename T>
	void refreshFileUpdateTimesInternal();
#endif

	// Note: Thread-safe against itself, loadResource() and refreshFileUpdateTimes()
	template<typename T>
	void freeResource(U32 uuid, U32 versionedResourceIdx);
};

} // end namespace anki
