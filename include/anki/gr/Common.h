// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Enums.h>
#include <anki/util/Allocator.h>
#include <anki/util/Ptr.h>

namespace anki
{

// Forward
class GrManager;
class GrManagerImpl;
class TextureInitializer;
class SamplerInitializer;
class GrManagerInitializer;
class PipelineInitializer;
class FramebufferInitializer;

class DynamicBufferToken;
class DynamicBufferInfo;

#define ANKI_GR_CLASS(x_)                                                      \
	class x_##Impl;                                                            \
	class x_;                                                                  \
	using x_##Ptr = IntrusivePtr<x_>;

ANKI_GR_CLASS(Buffer)
ANKI_GR_CLASS(Texture)
ANKI_GR_CLASS(Sampler)
ANKI_GR_CLASS(CommandBuffer)
ANKI_GR_CLASS(Shader)
ANKI_GR_CLASS(Pipeline)
ANKI_GR_CLASS(Framebuffer)
ANKI_GR_CLASS(OcclusionQuery)
ANKI_GR_CLASS(ResourceGroup)

/// @addtogroup graphics
/// @{

/// The type of the allocator for heap allocations
template<typename T>
using GrAllocator = HeapAllocator<T>;

/// Token that gets returned when requesting for memory to write to a dynamic
/// buffer.
class DynamicBufferToken
{
anki_internal:
	U32 m_offset = 0;
	U32 m_range = 0;

	void markUnused()
	{
		m_offset = m_range = MAX_U32;
	}
};

// Some constants
// WARNING: If you change those update the shaders
const U MAX_VERTEX_ATTRIBUTES = 8;
const U MAX_COLOR_ATTACHMENTS = 4;
const U MAX_MIPMAPS = 16;
const U MAX_TEXTURE_LAYERS = 32;
const U MAX_TEXTURE_BINDINGS = 8;
const U MAX_UNIFORM_BUFFER_BINDINGS = 1;
const U MAX_STORAGE_BUFFER_BINDINGS = 8;
const U MAX_FRAMES_IN_FLIGHT = 3;
const U MAX_RESOURCE_GROUPS = 2;

/// Compute max number of mipmaps for a 2D surface.
inline U32 computeMaxMipmapCount(U32 w, U32 h)
{
	U32 s = (w < h) ? w : h;
	U count = 0;
	while(s)
	{
		s /= 2;
		++count;
	}

	return count;
}
/// @}

} // end namespace anki
