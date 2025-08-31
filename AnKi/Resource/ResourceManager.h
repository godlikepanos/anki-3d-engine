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

/// @addtogroup resource
/// @{

ANKI_CVAR(NumericCVar<PtrSize>, Rsrc, TransferScratchMemorySize, 256_MB, 1_MB, 4_GB, "Memory that is used fot texture and buffer uploads")

/// Resource manager. It holds a few global variables
class ResourceManager : public MakeSingleton<ResourceManager>
{
	template<typename T>
	friend class ResourcePtrDeleter;
	friend class ResourceObject;

	template<typename>
	friend class MakeSingleton;

public:
	Error init(AllocAlignedCallback allocCallback, void* allocCallbackData);

	/// Load a resource.
	/// @node Thread-safe against itself and freeResource.
	template<typename T>
	Error loadResource(const CString& filename, ResourcePtr<T>& out, Bool async = true);

	// Internals:

	/// @node Thread-safe against itself and loadResource.
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

	ResourceManager();

	~ResourceManager();
};
/// @}

} // end namespace anki
