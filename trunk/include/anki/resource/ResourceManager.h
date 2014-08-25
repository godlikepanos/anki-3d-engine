// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_RESOURCE_MANAGER_H
#define ANKI_RESOURCE_RESOURCE_MANAGER_H

#include "anki/util/Vector.h"
#include "anki/util/Functions.h"
#include "anki/util/String.h"
#include "anki/resource/ResourcePointer.h"

namespace anki {

// Forward
class ConfigSet;

class Animation;
class Material;
class Mesh;
class Model;
class BucketMesh;
class ProgramResource;
class ParticleEmitterResource;
class TextureResource;

/// @addtogroup resource
/// @{

/// Manage resources of a certain type
template<typename Type, typename TResourceManager>
class TypeResourceManager
{
public:
	using ResourcePointerType = ResourcePointer<Type, TResourceManager>;
	using Container = 
		Vector<ResourcePointerType, HeapAllocator<ResourcePointerType>>;

	TypeResourceManager()
	{}

	~TypeResourceManager()
	{
		ANKI_ASSERT(m_ptrs.size() == 0 
			&& "Forgot to delete some resource ptrs");
	}

	/// @privatesection
	/// @{
	void init(HeapAllocator<U8>& alloc)
	{
		HeapAllocator<ResourcePointerType> alloc2 = alloc;
		Container ptrs(alloc2);
		m_ptrs = std::move(ptrs);
	}

	Bool _findLoadedResource(const char* filename, ResourcePointerType& ptr)
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

	void _registerResource(ResourcePointerType& ptr)
	{
		ANKI_ASSERT(ptr.getReferenceCount() == 1);
		ANKI_ASSERT(find(ptr.getResourceName()) == m_ptrs.end());
	
		m_ptrs.push_back(ptr);
	}

	void _unregisterResource(ResourcePointerType& ptr)
	{
		auto it = find(ptr.getResourceName());
		ANKI_ASSERT(it != m_ptrs.end());
	
		m_ptrs.erase(it);
	}
	/// @}

private:
	Container m_ptrs;

	typename Container::iterator find(const char* filename)
	{
		typename Container::iterator it;
		
		for(it = m_ptrs.begin(); it != m_ptrs.end(); ++it)
		{
			if(std::strcmp(it->getResourceName(), filename) == 0)
			{
				break;
			}
		}

		return it;
	}
};

/// Resource manager. It holds a few global variables
class ResourceManager: 
	public TypeResourceManager<Animation, ResourceManager>,
	public TypeResourceManager<Material, ResourceManager>,
	public TypeResourceManager<Mesh, ResourceManager>,
	public TypeResourceManager<Model, ResourceManager>,
	public TypeResourceManager<BucketMesh, ResourceManager>,
	public TypeResourceManager<ProgramResource, ResourceManager>,
	public TypeResourceManager<ParticleEmitterResource, ResourceManager>,
	public TypeResourceManager<TextureResource, ResourceManager>
{
public:
	ResourceManager(const ConfigSet& config);

	const String& getDataPath() const
	{
		return m_dataPath;
	}

	U32 getMaxTextureSize() const
	{
		return m_maxTextureSize;
	}

	U32 getTextureAnisotropy() const
	{
		return m_textureAnisotropy;
	}

	HeapAllocator<U8>& _getAllocator()
	{
		return m_alloc;
	}

	String fixResourcePath(const char* filename) const;

	/// @privatesection
	/// @{
	template<typename T>
	Bool _findLoadedResource(const char* filename, 
		ResourcePointer<T, ResourceManager>& ptr)
	{
		return TypeResourceManager<T, ResourceManager>::_findLoadedResource(
			filename, ptr);
	}

	template<typename T>
	void _registerResource(ResourcePointer<T, ResourceManager>& ptr)
	{
		TypeResourceManager<T, ResourceManager>::_registerResource(ptr);
	}

	template<typename T>
	void _unregisterResource(ResourcePointer<T, ResourceManager>& ptr)
	{
		TypeResourceManager<T, ResourceManager>::_unregisterResource(ptr);
	}
	/// @}

private:
	HeapAllocator<U8> m_alloc;
	String m_dataPath;
	U32 m_maxTextureSize;
	U32 m_textureAnisotropy;
};

/// @}

} // end namespace anki

#endif
