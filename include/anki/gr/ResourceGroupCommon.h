// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_RESOURCE_GROUP_COMMON_H
#define ANKI_GR_RESOURCE_GROUP_COMMON_H

#include "anki/gr/Common.h"
#include "anki/gr/TextureHandle.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Buffer binding point.
class BufferBinding
{
public:
	BufferHandle m_buffer;
	PtrSize m_offset = 0;
};

/// Resource group initializer.
class ResourceGroupInitializer
{
public:
	Array<TextureHandle, MAX_TEXTURE_BINDINGS> m_textures;
	Array<SamplerHandle, MAX_TEXTURE_BINDINGS> m_samplers;
	Array<BufferBinding, MAX_UNIFORM_BUFFER_BINDINGS> m_uniformBuffers;
	Array<BufferBinding, MAX_STORAGE_BUFFER_BINDINGS> m_storageBuffers;
	Array<BufferBinding, MAX_ATTRIBUTES> m_storageBuffers;
	BufferBinding m_indexBuffer;
};
/// @}

} // end namespace anki

#endif

