// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_RESOURCE_GROUP_COMMON_H
#define ANKI_GR_RESOURCE_GROUP_COMMON_H

#include "anki/gr/Common.h"
#include "anki/gr/TextureHandle.h"
#include "anki/gr/SamplerHandle.h"
#include "anki/gr/BufferHandle.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Texture/Sampler Binding.
class TextureBinding
{
public:
	TextureHandle m_texture;
	SamplerHandle m_sampler; ///< Use it to override texture's sampler.
};

/// Buffer binding point.
class BufferBinding
{
public:
	BufferHandle m_buffer;
	PtrSize m_offset = 0;
	PtrSize m_range = 0;
};

/// Resource group initializer.
class ResourceGroupInitializer
{
public:
	Array<TextureBinding, MAX_TEXTURE_BINDINGS> m_textures;
	Array<BufferBinding, MAX_UNIFORM_BUFFER_BINDINGS> m_uniformBuffers;
	Array<BufferBinding, MAX_STORAGE_BUFFER_BINDINGS> m_storageBuffers;
	Array<BufferBinding, MAX_VERTEX_ATTRIBUTES> m_vertexBuffers;
	BufferBinding m_indexBuffer;
};
/// @}

} // end namespace anki

#endif

