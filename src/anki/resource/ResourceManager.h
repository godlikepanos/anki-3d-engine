// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/TransferGpuAllocator.h>
#include <anki/util/List.h>
#include <anki/util/Functions.h>
#include <anki/util/String.h>

namespace anki
{

// Forward
class ConfigSet;
class GrManager;
class PhysicsWorld;
class ResourceManager;
class AsyncLoader;
class ResourceManagerModel;
class ShaderCompilerCache;

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
		m_ptrs.destroy(m_alloc);
	}

	Type* findLoadedResource(const CString& filename)
	{
		auto it = find(filename);
		return (it != m_ptrs.end()) ? *it : nullptr;
	}

	void registerResource(Type* ptr)
	{
		ANKI_ASSERT(ptr->getRefcount().load() == 0);
		ANKI_ASSERT(find(ptr->getFilename()) == m_ptrs.getEnd());
		m_ptrs.pushBack(m_alloc, ptr);
	}

	void unregisterResource(Type* ptr)
	{
		auto it = find(ptr->getFilename());
		ANKI_ASSERT(it != m_ptrs.end());
		m_ptrs.erase(m_alloc, it);
	}

	void init(ResourceAllocator<U8> alloc)
	{
		m_alloc = alloc;
	}

private:
	using Container = List<Type*>;

	ResourceAllocator<U8> m_alloc;
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
	const ConfigSet* m_config = nullptr;
	CString m_cacheDir;
	AllocAlignedCallback m_allocCallback = 0;
	void* m_allocCallbackData = nullptr;
};

/// Resource manager. It holds a few global variables
class ResourceManager:

#define ANKI_INSTANTIATE_RESOURCE(rsrc_, ptr_) \
public \
	TypeResourceManager<rsrc_>

#define ANKI_INSTANSIATE_RESOURCE_DELIMITER() ,

#include <anki/resource/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_RESOURCE
#undef ANKI_INSTANSIATE_RESOURCE_DELIMITER

{
	template<typename T>
	friend class ResourcePtrDeleter;

public:
	ResourceManager();

	~ResourceManager();

	ANKI_USE_RESULT Error init(ResourceManagerInitInfo& init);

	/// Load a resource.
	template<typename T>
	ANKI_USE_RESULT Error loadResource(const CString& filename, ResourcePtr<T>& out, Bool async = true);

anki_internal:
	U32 getMaxTextureSize() const
	{
		return m_maxTextureSize;
	}

	U32 getTextureAnisotropy() const
	{
		return m_textureAnisotropy;
	}

	ResourceAllocator<U8>& getAllocator()
	{
		return m_alloc;
	}

	TempResourceAllocator<U8>& getTempAllocator()
	{
		return m_tmpAlloc;
	}

	GrManager& getGrManager()
	{
		ANKI_ASSERT(m_gr);
		return *m_gr;
	}

	TransferGpuAllocator& getTransferGpuAllocator()
	{
		return *m_transferGpuAlloc;
	}

	PhysicsWorld& getPhysicsWorld()
	{
		ANKI_ASSERT(m_physics);
		return *m_physics;
	}

	ResourceFilesystem& getFilesystem()
	{
		ANKI_ASSERT(m_fs);
		return *m_fs;
	}

	const String& getCacheDirectory() const
	{
		return m_cacheDir;
	}

	template<typename T>
	T* findLoadedResource(const CString& filename)
	{
		return TypeResourceManager<T>::findLoadedResource(filename);
	}

	template<typename T>
	void registerResource(T* ptr)
	{
		TypeResourceManager<T>::registerResource(ptr);
	}

	template<typename T>
	void unregisterResource(T* ptr)
	{
		TypeResourceManager<T>::unregisterResource(ptr);
	}

	AsyncLoader& getAsyncLoader()
	{
		return *m_asyncLoader;
	}

	const ShaderCompilerCache& getShaderCompiler() const
	{
		ANKI_ASSERT(m_shaderCompiler);
		return *m_shaderCompiler;
	}

	/// Get the number of times loadResource() was called.
	U64 getLoadingRequestCount() const
	{
		return m_loadRequestCount;
	}

	/// Get the total number of completed async tasks.
	U64 getAsyncTaskCompletedCount() const;

private:
	GrManager* m_gr = nullptr;
	PhysicsWorld* m_physics = nullptr;
	ResourceFilesystem* m_fs = nullptr;
	ResourceAllocator<U8> m_alloc;
	TempResourceAllocator<U8> m_tmpAlloc;
	String m_cacheDir;
	U32 m_maxTextureSize;
	U32 m_textureAnisotropy;
	AsyncLoader* m_asyncLoader = nullptr; ///< Async loading thread
	U64 m_uuid = 0;
	U64 m_loadRequestCount = 0;
	TransferGpuAllocator* m_transferGpuAlloc = nullptr;
	ShaderCompilerCache* m_shaderCompiler = nullptr;
};
/// @}

} // end namespace anki

#include <anki/resource/ResourceManager.inl.h>
