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

inline NumericCVar<PtrSize> g_transferScratchMemorySizeCVar("Rsrc", "TransferScratchMemorySize", 256_MB, 1_MB, 4_GB,
															"Memory that is used fot texture and buffer uploads");

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

	ANKI_INTERNAL TransferGpuAllocator& getTransferGpuAllocator()
	{
		return *m_transferGpuAlloc;
	}

	ANKI_INTERNAL AsyncLoader& getAsyncLoader()
	{
		return *m_asyncLoader;
	}

	/// Return the container of program libraries.
	ANKI_INTERNAL const ShaderProgramResourceSystem& getShaderProgramResourceSystem() const
	{
		return *m_shaderProgramSystem;
	}

	ANKI_INTERNAL ResourceFilesystem& getFilesystem()
	{
		return *m_fs;
	}

	ANKI_INTERNAL AccelerationStructureScratchAllocator& getAccelerationStructureScratchAllocator()
	{
		ANKI_ASSERT(m_asScratchAlloc);
		return *m_asScratchAlloc;
	}

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

	ResourceFilesystem* m_fs = nullptr;
	AsyncLoader* m_asyncLoader = nullptr; ///< Async loading thread
	ShaderProgramResourceSystem* m_shaderProgramSystem = nullptr;
	TransferGpuAllocator* m_transferGpuAlloc = nullptr;
	AccelerationStructureScratchAllocator* m_asScratchAlloc = nullptr;

	AllTypeData m_allTypes;

	Atomic<U32> m_uuid = {1};

	ResourceManager();

	~ResourceManager();
};
/// @}

} // end namespace anki
