// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_COMMON_H
#define ANKI_RESOURCE_COMMON_H

#include "anki/util/Allocator.h"
#include "anki/util/Vector.h"
#include "anki/util/String.h"

namespace anki {

// Forward
class GlDevice;
class ResourceManager;

/// @addtogroup resource
/// @{

template<typename T>
using ResourceAllocator = HeapAllocator<T>;

template<typename T>
using ResourceVector = Vector<T, ResourceAllocator<T>>;

using ResourceString = BasicString<char, ResourceAllocator<char>>;

template<typename T>
using TempResourceAllocator = StackAllocator<T>;

template<typename T>
using TempResourceVector = Vector<T, TempResourceAllocator<T>>;

using TempResourceString = BasicString<char, TempResourceAllocator<char>>;

/// Contains initialization information for the resource classes.
class ResourceInitializer
{
public:
	ResourceAllocator<U8>& m_alloc;
	TempResourceAllocator<U8>& m_tempAlloc;
	const char* m_cacheDir;
	GlDevice& m_gl;
	U16 m_maxTextureSize;
	U16 m_anisotropyLevel;
	ResourceManager& m_resourceManager;

	ResourceInitializer(
		ResourceAllocator<U8>& alloc, 
		TempResourceAllocator<U8>& tempAlloc,
		const char* cacheDir,
		GlDevice& gl,
		U16 maxTextureSize,
		U16 anisotropyLevel,
		ResourceManager& resourceManager)
	:	m_alloc(alloc),
		m_tempAlloc(tempAlloc),
		m_cacheDir(cacheDir),
		m_gl(gl),
		m_maxTextureSize(maxTextureSize),
		m_anisotropyLevel(anisotropyLevel),
		m_resourceManager(resourceManager)
	{}
};

/// @}

} // end namespace anki

#endif
