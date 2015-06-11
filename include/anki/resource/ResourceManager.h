// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_RESOURCE_MANAGER_H
#define ANKI_RESOURCE_RESOURCE_MANAGER_H

#include "anki/resource/Common.h"
#include "anki/util/List.h"
#include "anki/util/Functions.h"
#include "anki/util/String.h"

namespace anki {

// Forward
class ConfigSet;
class GrManager;
class PhysicsWorld;
class ResourceManager;
class AsyncLoader;
class MaterialResourceData;

/// @addtogroup resource
/// @{

/// Manage resources of a certain type
template<typename Type>
class TypeResourceManager
{
public:
	using Container = List<Type*>;

	TypeResourceManager()
	{}

	~TypeResourceManager()
	{
		ANKI_ASSERT(m_ptrs.isEmpty() && "Forgot to delete some resources");
		m_ptrs.destroy(m_alloc);
	}

	/// @privatesection
	/// @{
	Type* findLoadedResource(const CString& filename)
	{
		auto it = find(filename);
		return (it != m_ptrs.end()) ? *it : nullptr;
	}

	void registerResource(Type* ptr)
	{
		ANKI_ASSERT(ptr->getRefcount().load() == 0);
		ANKI_ASSERT(find(ptr->getUuid().toCString()) == m_ptrs.getEnd());
		m_ptrs.pushBack(m_alloc, ptr);
	}

	void unregisterResource(Type* ptr)
	{
		auto it = find(ptr->getUuid().toCString());
		ANKI_ASSERT(it != m_ptrs.end());
		m_ptrs.erase(m_alloc, it);
	}
	/// @}

protected:
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
			if((*it)->getUuid() == filename)
			{
				break;
			}
		}

		return it;
	}
};

#define ANKI_RESOURCE(type_) public TypeResourceManager<type_>

/// Resource manager. It holds a few global variables
class ResourceManager:
	ANKI_RESOURCE(Animation),
	ANKI_RESOURCE(TextureResource),
	ANKI_RESOURCE(ShaderResource),
	ANKI_RESOURCE(Material),
	ANKI_RESOURCE(Mesh),
	ANKI_RESOURCE(BucketMesh),
	ANKI_RESOURCE(Skeleton),
	ANKI_RESOURCE(ParticleEmitterResource),
	ANKI_RESOURCE(Model),
	ANKI_RESOURCE(Script),
	ANKI_RESOURCE(DummyRsrc),
	ANKI_RESOURCE(CollisionResource)
{
public:
	class Initializer
	{
	public:
		GrManager* m_gr = nullptr;
		PhysicsWorld* m_physics = nullptr;
		ResourceFilesystem* m_resourceFs = nullptr;
		const ConfigSet* m_config = nullptr;
		CString m_cacheDir;
		AllocAlignedCallback m_allocCallback = 0;
		void* m_allocCallbackData = nullptr;
		U32 m_tempAllocatorMemorySize = 1 * 1024 * 1024;
	};

	ResourceManager();

	~ResourceManager();

	ANKI_USE_RESULT Error create(Initializer& init);

	U32 getMaxTextureSize() const
	{
		return m_maxTextureSize;
	}

	U32 getTextureAnisotropy() const
	{
		return m_textureAnisotropy;
	}

	/// @privatesection
	/// @{
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

	PhysicsWorld& _getPhysicsWorld()
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

	AsyncLoader& _getAsyncLoader()
	{
		return *m_asyncLoader;
	}

	MaterialResourceData& getMaterialData()
	{
		ANKI_ASSERT(m_materialData);
		return *m_materialData;
	}
	/// @}

private:
	GrManager* m_gr = nullptr;
	PhysicsWorld* m_physics = nullptr;
	ResourceFilesystem* m_fs = nullptr;
	ResourceAllocator<U8> m_alloc;
	TempResourceAllocator<U8> m_tmpAlloc;
	String m_cacheDir;
	String m_dataDir;
	U32 m_maxTextureSize;
	U32 m_textureAnisotropy;
	String m_shadersPrependedSource;
	AsyncLoader* m_asyncLoader = nullptr; ///< Async loading thread
	MaterialResourceData* m_materialData = nullptr;
};

#undef ANKI_RESOURCE
/// @}

} // end namespace anki

#endif
