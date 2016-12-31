// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
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
class Renderer;
class StagingGpuMemoryManager;

/// @addtogroup resource
/// @{

/// Manage resources of a certain type
template<typename Type>
class TypeResourceManager
{
protected:
	/// @privatesection
	using Container = List<Type*>;

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
	StagingGpuMemoryManager* m_stagingMem = nullptr;
	PhysicsWorld* m_physics = nullptr;
	ResourceFilesystem* m_resourceFs = nullptr;
	const ConfigSet* m_config = nullptr;
	CString m_cacheDir;
	AllocAlignedCallback m_allocCallback = 0;
	void* m_allocCallbackData = nullptr;
};

/// Resource manager. It holds a few global variables
class ResourceManager : public TypeResourceManager<Animation>,
						public TypeResourceManager<TextureResource>,
						public TypeResourceManager<ShaderResource>,
						public TypeResourceManager<Material>,
						public TypeResourceManager<Mesh>,
						public TypeResourceManager<Skeleton>,
						public TypeResourceManager<ParticleEmitterResource>,
						public TypeResourceManager<Model>,
						public TypeResourceManager<Script>,
						public TypeResourceManager<DummyRsrc>,
						public TypeResourceManager<CollisionResource>,
						public TypeResourceManager<GenericResource>,
						public TypeResourceManager<TextureAtlas>
{
	template<typename T>
	friend class ResourcePtrDeleter;

public:
	ResourceManager();

	~ResourceManager();

	ANKI_USE_RESULT Error create(ResourceManagerInitInfo& init);

	/// Load a resource.
	template<typename T>
	ANKI_USE_RESULT Error loadResource(const CString& filename, ResourcePtr<T>& out);

	/// Load a resource to cache.
	template<typename T, typename... TArgs>
	ANKI_USE_RESULT Error loadResourceToCache(ResourcePtr<T>& out, TArgs&&... args);

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

	StagingGpuMemoryManager& getStagingGpuMemoryManager()
	{
		ANKI_ASSERT(m_stagingMem);
		return *m_stagingMem;
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

	const String& _getCacheDirectory() const
	{
		return m_cacheDir;
	}

	/// Set it with information from the renderer
	void _setShadersPrependedSource(const CString& cstr)
	{
		m_shadersPrependedSource.create(m_alloc, cstr);
	}

	const String& _getShadersPrependedSource() const
	{
		return m_shadersPrependedSource;
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

	/// Get the number of times loadResource() was called.
	U64 getLoadingRequestCount() const
	{
		return m_loadRequestCount;
	}

	/// Get the total number of completed async tasks.
	U64 getAsyncTaskCompletedCount() const;

private:
	GrManager* m_gr = nullptr;
	StagingGpuMemoryManager* m_stagingMem = nullptr;
	PhysicsWorld* m_physics = nullptr;
	ResourceFilesystem* m_fs = nullptr;
	ResourceAllocator<U8> m_alloc;
	TempResourceAllocator<U8> m_tmpAlloc;
	String m_cacheDir;
	U32 m_maxTextureSize;
	U32 m_textureAnisotropy;
	String m_shadersPrependedSource;
	AsyncLoader* m_asyncLoader = nullptr; ///< Async loading thread
	U64 m_uuid = 0;
	U64 m_loadRequestCount = 0;
};
/// @}

} // end namespace anki

#include <anki/resource/ResourceManager.inl.h>
