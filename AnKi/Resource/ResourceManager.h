// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/TransferGpuAllocator.h>
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
class AccelerationStructureScratchAllocator;

ANKI_CVAR(NumericCVar<PtrSize>, Rsrc, TransferScratchMemorySize, 256_MB, 1_MB, 4_GB, "Memory that is used fot texture and buffer uploads")
ANKI_CVAR(BoolCVar, Rsrc, TrackFileUpdates, false, "If true the resource manager is able to track file update times")

// Resource manager. It holds a few global variables
class ResourceManager : public MakeSingleton<ResourceManager>
{
	template<typename T>
	friend class ResourcePtrDeleter;
	friend class ResourceObject;

	template<typename>
	friend class MakeSingleton;

public:
	Error init(AllocAlignedCallback allocCallback, void* allocCallbackData);

	// Load a resource.
	// Note: Thread-safe against itself, freeResource() and refreshFileUpdateTimes()
	template<typename T>
	Error loadResource(CString filename, ResourcePtr<T>& out, Bool async = true);

	// Iterate all loaded resource and check if the files have been updated since they were loaded.
	// Note: Thread-safe against itself, loadResource() and freeResource()
	void refreshFileUpdateTimes();

	// Internals:

	// Note: Thread-safe against itself, loadResource() and refreshFileUpdateTimes()
	template<typename T>
	ANKI_INTERNAL void freeResource(T* ptr);

private:
	template<typename Type>
	class TypeData
	{
	public:
		class Entry
		{
		public:
			Type* m_resource = nullptr;
			U64 m_fileUpdateTime = 0;
			SpinLock m_mtx;
		};

		ResourceHashMap<CString, U32> m_map;
		ResourceBlockArray<Entry> m_entries;
		RWMutex m_mtx;

		TypeData()
		{
			for([[maybe_unused]] const Entry& e : m_entries)
			{
				ANKI_ASSERT(e.m_resource == nullptr && "Forgot to release some resource");
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

	Bool m_trackFileUpdateTimes = false;

	ResourceManager();

	~ResourceManager();

	template<typename T>
	void refreshFileUpdateTimesInternal();
};

} // end namespace anki
