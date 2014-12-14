// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_RESOURCE_MANAGER_H
#define ANKI_RESOURCE_RESOURCE_MANAGER_H

#include "anki/resource/Common.h"
#include "anki/resource/ResourcePointer.h"
#include "anki/util/List.h"
#include "anki/util/Functions.h"
#include "anki/util/String.h"

namespace anki {

// Forward
class ConfigSet;
class GlDevice;
class ResourceManager;
class AsyncLoader;

// NOTE: Add resources in 3 places
#define ANKI_RESOURCE(rsrc_, name_) \
	class rsrc_; \
	using name_ = ResourcePointer<rsrc_, ResourceManager>;

ANKI_RESOURCE(Animation, AnimationResourcePointer)
ANKI_RESOURCE(TextureResource, TextureResourcePointer)
ANKI_RESOURCE(ProgramResource, ProgramResourcePointer)
ANKI_RESOURCE(Material, MaterialResourcePointer)
ANKI_RESOURCE(Mesh, MeshResourcePointer)
ANKI_RESOURCE(BucketMesh, BucketMeshResourcePointer)
ANKI_RESOURCE(Skeleton, SkeletonResourcePointer)
ANKI_RESOURCE(ParticleEmitterResource, ParticleEmitterResourcePointer)
ANKI_RESOURCE(Model, ModelResourcePointer)
ANKI_RESOURCE(Script, ScriptResourcePointer)
ANKI_RESOURCE(DummyRsrc, DummyResourcePointer)

#undef ANKI_RESOURCE

/// @addtogroup resource
/// @{

/// Manage resources of a certain type
template<typename Type, typename TResourceManager>
class TypeResourceManager
{
public:
	using ResourcePointerType = ResourcePointer<Type, TResourceManager>;
	using Container = List<ResourcePointerType>;

	TypeResourceManager()
	{}

	~TypeResourceManager()
	{
		ANKI_ASSERT(m_ptrs.isEmpty() 
			&& "Forgot to delete some resource ptrs");

		m_ptrs.destroy(m_alloc);
	}

	/// @privatesection
	/// @{
	Bool _findLoadedResource(const CString& filename, ResourcePointerType& ptr)
	{
		auto it = find(filename);
		
		if(it != m_ptrs.end())
		{
			ptr = *it;
			return true;
		}
		else
		{
			return false;
		}
	}

	ANKI_USE_RESULT Error _registerResource(ResourcePointerType& ptr)
	{
		ANKI_ASSERT(ptr.getReferenceCount() == 1);
		ANKI_ASSERT(find(ptr.getResourceName()) == m_ptrs.getEnd());
	
		return m_ptrs.pushBack(m_alloc, ptr);
	}

	void _unregisterResource(ResourcePointerType& ptr)
	{
		auto it = find(ptr.getResourceName());
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
			if(it->getResourceName() == filename)
			{
				break;
			}
		}

		return it;
	}
};

#define ANKI_RESOURCE(type_) \
	public TypeResourceManager<type_, ResourceManager>

/// Resource manager. It holds a few global variables
class ResourceManager: 
	ANKI_RESOURCE(Animation),
	ANKI_RESOURCE(TextureResource),
	ANKI_RESOURCE(ProgramResource),
	ANKI_RESOURCE(Material),
	ANKI_RESOURCE(Mesh),
	ANKI_RESOURCE(BucketMesh),
	ANKI_RESOURCE(Skeleton),
	ANKI_RESOURCE(ParticleEmitterResource),
	ANKI_RESOURCE(Model),
	ANKI_RESOURCE(Script),
	ANKI_RESOURCE(DummyRsrc)
{
public:
	class Initializer
	{
	public:
		GlDevice* m_gl = nullptr;
		const ConfigSet* m_config = nullptr;
		CString m_cacheDir;
		AllocAlignedCallback m_allocCallback = 0;
		void* m_allocCallbackData = nullptr;
		U32 m_tempAllocatorMemorySize = 1 * 1024 * 1024;
	};

	ResourceManager();

	~ResourceManager();

	ANKI_USE_RESULT Error create(Initializer& init);

	const ResourceString& getDataDirectory() const
	{
		return m_dataDir;
	}

	U32 getMaxTextureSize() const
	{
		return m_maxTextureSize;
	}

	U32 getTextureAnisotropy() const
	{
		return m_textureAnisotropy;
	}

	ANKI_USE_RESULT Error fixResourceFilename(
		const CString& filename,
		TempResourceString& out) const;

	/// @privatesection
	/// @{
	ResourceAllocator<U8>& _getAllocator()
	{
		return m_alloc;
	}

	TempResourceAllocator<U8>& _getTempAllocator()
	{
		return m_tmpAlloc;
	}

	GlDevice& _getGlDevice()
	{
		return *m_gl;
	}

	const ResourceString& _getCacheDirectory() const
	{
		return m_cacheDir;
	}

	/// Set it with information from the renderer
	ANKI_USE_RESULT Error _setShadersPrependedSource(const CString& cstr)
	{
		return m_shadersPrependedSource.create(m_alloc, cstr);
	}

	const ResourceString& _getShadersPrependedSource() const
	{
		return m_shadersPrependedSource;
	}

	template<typename T>
	Bool _findLoadedResource(const CString& filename, 
		ResourcePointer<T, ResourceManager>& ptr)
	{
		return TypeResourceManager<T, ResourceManager>::_findLoadedResource(
			filename, ptr);
	}

	template<typename T>
	ANKI_USE_RESULT Error _registerResource(
		ResourcePointer<T, ResourceManager>& ptr)
	{
		return TypeResourceManager<T, ResourceManager>::_registerResource(ptr);
	}

	template<typename T>
	void _unregisterResource(ResourcePointer<T, ResourceManager>& ptr)
	{
		TypeResourceManager<T, ResourceManager>::_unregisterResource(ptr);
	}

	AsyncLoader& _getAsyncLoader() 
	{
		return *m_asyncLoader;
	}
	/// @}

private:
	GlDevice* m_gl = nullptr;
	ResourceAllocator<U8> m_alloc;
	TempResourceAllocator<U8> m_tmpAlloc;
	ResourceString m_cacheDir;
	ResourceString m_dataDir;
	U32 m_maxTextureSize;
	U32 m_textureAnisotropy;
	ResourceString m_shadersPrependedSource;
	AsyncLoader* m_asyncLoader = nullptr; ///< Async loading thread
};

#undef ANKI_RESOURCE

/// @}

} // end namespace anki

#endif
