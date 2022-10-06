// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/TransferGpuAllocator.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/String.h>

namespace anki {

// Forward
class ConfigSet;
class GrManager;
class PhysicsWorld;
class ResourceManager;
class AsyncLoader;
class ResourceManagerModel;
class ShaderCompilerCache;
class ShaderProgramResourceSystem;
class UnifiedGeometryMemoryPool;

/// @addtogroup resource
/// @{

/// Manage resources of a certain type
template<typename Type>
class TypeResourceManager
{
protected:
	TypeResourceManager()
	{
	}

	~TypeResourceManager()
	{
		ANKI_ASSERT(m_ptrs.isEmpty() && "Forgot to delete some resources");
		m_ptrs.destroy(*m_pool);
	}

	Type* findLoadedResource(const CString& filename)
	{
		auto it = find(filename);
		return (it != m_ptrs.end()) ? *it : nullptr;
	}

	void registerResource(Type* ptr)
	{
		ANKI_ASSERT(find(ptr->getFilename()) == m_ptrs.getEnd());
		m_ptrs.pushBack(*m_pool, ptr);
	}

	void unregisterResource(Type* ptr)
	{
		auto it = find(ptr->getFilename());
		ANKI_ASSERT(it != m_ptrs.end());
		m_ptrs.erase(*m_pool, it);
	}

	void init(HeapMemoryPool* pool)
	{
		ANKI_ASSERT(pool);
		m_pool = pool;
	}

private:
	using Container = List<Type*>;

	HeapMemoryPool* m_pool = nullptr;
	Container m_ptrs;

	typename Container::Iterator find(const CString& filename)
	{
		typename Container::Iterator it;

		for(it = m_ptrs.getBegin(); it != m_ptrs.getEnd(); ++it)
		{
			if((*it)->getFilename() == filename)
			{
				break;
			}
		}

		return it;
	}
};

class ResourceManagerInitInfo
{
public:
	GrManager* m_gr = nullptr;
	PhysicsWorld* m_physics = nullptr;
	ResourceFilesystem* m_resourceFs = nullptr;
	ConfigSet* m_config = nullptr;
	UnifiedGeometryMemoryPool* m_unifiedGometryMemoryPool = nullptr;
	AllocAlignedCallback m_allocCallback = 0;
	void* m_allocCallbackData = nullptr;
};

/// Resource manager. It holds a few global variables
class ResourceManager:

#define ANKI_INSTANTIATE_RESOURCE(rsrc_, ptr_) \
public \
	TypeResourceManager<rsrc_>

#define ANKI_INSTANSIATE_RESOURCE_DELIMITER() ,

#include <AnKi/Resource/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_RESOURCE
#undef ANKI_INSTANSIATE_RESOURCE_DELIMITER

{
	template<typename T>
	friend class ResourcePtrDeleter;

public:
	ResourceManager();

	~ResourceManager();

	Error init(ResourceManagerInitInfo& init);

	/// Load a resource.
	template<typename T>
	Error loadResource(const CString& filename, ResourcePtr<T>& out, Bool async = true);

	// Internals:

	ANKI_INTERNAL HeapMemoryPool& getMemoryPool() const
	{
		return m_pool;
	}

	ANKI_INTERNAL StackMemoryPool& getTempMemoryPool() const
	{
		return m_tmpPool;
	}

	ANKI_INTERNAL GrManager& getGrManager()
	{
		ANKI_ASSERT(m_gr);
		return *m_gr;
	}

	ANKI_INTERNAL TransferGpuAllocator& getTransferGpuAllocator()
	{
		return *m_transferGpuAlloc;
	}

	ANKI_INTERNAL PhysicsWorld& getPhysicsWorld()
	{
		ANKI_ASSERT(m_physics);
		return *m_physics;
	}

	ANKI_INTERNAL ResourceFilesystem& getFilesystem()
	{
		ANKI_ASSERT(m_fs);
		return *m_fs;
	}

	template<typename T>
	ANKI_INTERNAL T* findLoadedResource(const CString& filename)
	{
		return TypeResourceManager<T>::findLoadedResource(filename);
	}

	template<typename T>
	ANKI_INTERNAL void registerResource(T* ptr)
	{
		TypeResourceManager<T>::registerResource(ptr);
	}

	template<typename T>
	ANKI_INTERNAL void unregisterResource(T* ptr)
	{
		TypeResourceManager<T>::unregisterResource(ptr);
	}

	ANKI_INTERNAL AsyncLoader& getAsyncLoader()
	{
		return *m_asyncLoader;
	}

	/// Get the number of times loadResource() was called.
	ANKI_INTERNAL U64 getLoadingRequestCount() const
	{
		return m_loadRequestCount;
	}

	/// Get the total number of completed async tasks.
	ANKI_INTERNAL U64 getAsyncTaskCompletedCount() const;

	/// Return the container of program libraries.
	const ShaderProgramResourceSystem& getShaderProgramResourceSystem() const
	{
		return *m_shaderProgramSystem;
	}

	UnifiedGeometryMemoryPool& getUnifiedGeometryMemoryPool()
	{
		ANKI_ASSERT(m_unifiedGometryMemoryPool);
		return *m_unifiedGometryMemoryPool;
	}

	const ConfigSet& getConfig() const
	{
		ANKI_ASSERT(m_config);
		return *m_config;
	}

private:
	GrManager* m_gr = nullptr;
	PhysicsWorld* m_physics = nullptr;
	ResourceFilesystem* m_fs = nullptr;
	ConfigSet* m_config = nullptr;
	mutable HeapMemoryPool m_pool; ///< Mutable because it's thread-safe and is may be called by const methods.
	mutable StackMemoryPool m_tmpPool; ///< Same as above.
	AsyncLoader* m_asyncLoader = nullptr; ///< Async loading thread
	ShaderProgramResourceSystem* m_shaderProgramSystem = nullptr;
	UnifiedGeometryMemoryPool* m_unifiedGometryMemoryPool = nullptr;
	U64 m_uuid = 0;
	U64 m_loadRequestCount = 0;
	TransferGpuAllocator* m_transferGpuAlloc = nullptr;
};
/// @}

} // end namespace anki
