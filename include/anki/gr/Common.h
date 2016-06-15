// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Enums.h>
#include <anki/util/Allocator.h>
#include <anki/util/Ptr.h>
#include <anki/util/String.h>

namespace anki
{

// Forward
class GrObject;

class GrManager;
class GrManagerImpl;
class TextureInitInfo;
class SamplerInitInfo;
class GrManagerInitInfo;
class PipelineInitInfo;
class FramebufferInitInfo;
class ResourceGroupInitInfo;

/// @addtogroup graphics
/// @{

/// Smart pointer for resources.
template<typename T>
using GrObjectPtr = IntrusivePtr<T, DefaultPtrDeleter<T>>;

#define ANKI_GR_CLASS(x_)                                                      \
	class x_##Impl;                                                            \
	class x_;                                                                  \
	using x_##Ptr = GrObjectPtr<x_>;

ANKI_GR_CLASS(Buffer)
ANKI_GR_CLASS(Texture)
ANKI_GR_CLASS(Sampler)
ANKI_GR_CLASS(CommandBuffer)
ANKI_GR_CLASS(Shader)
ANKI_GR_CLASS(Pipeline)
ANKI_GR_CLASS(Framebuffer)
ANKI_GR_CLASS(OcclusionQuery)
ANKI_GR_CLASS(ResourceGroup)

#undef ANKI_GR_CLASS

/// Graphics object type.
enum GrObjectType : U16
{
	BUFFER,
	COMMAND_BUFFER,
	FRAMEBUFFER,
	OCCLUSION_QUERY,
	PIPELINE,
	RESOURCE_GROUP,
	SAMPLER,
	SHADER,
	TEXTURE,
	COUNT
};

/// The type of the allocator for heap allocations
template<typename T>
using GrAllocator = HeapAllocator<T>;

/// Clear values for textures or attachments.
class ClearValue
{
private:
	class Ds
	{
	public:
		F32 m_depth;
		I32 m_stencil;
	};

public:
	union
	{
		Array<F32, 4> m_colorf = {{0.0, 0.0, 0.0, 0.0}};
		Array<I32, 4> m_colori;
		Array<U32, 4> m_coloru;
		Ds m_depthStencil;
	};
};

/// A way to identify a surface in a texture.
class TextureSurfaceInfo
{
public:
	TextureSurfaceInfo() = default;

	TextureSurfaceInfo(const TextureSurfaceInfo&) = default;

	TextureSurfaceInfo(U level, U depth, U face)
		: m_level(level)
		, m_depth(depth)
		, m_face(face)
	{
	}

	U32 m_level = 0;
	U32 m_depth = 0;
	U32 m_face = 0;
};

// Some constants
// WARNING: If you change those you may need to update the shaders.
const U MAX_VERTEX_ATTRIBUTES = 8;
const U MAX_COLOR_ATTACHMENTS = 4;
const U MAX_MIPMAPS = 16;
const U MAX_TEXTURE_LAYERS = 32;
const U MAX_TEXTURE_BINDINGS = 10;
const U MAX_UNIFORM_BUFFER_BINDINGS = 4;
const U MAX_STORAGE_BUFFER_BINDINGS = 4;
const U MAX_ATOMIC_BUFFER_BINDINGS = 1;
const U MAX_FRAMES_IN_FLIGHT = 3; ///< Triple buffering.
/// Groups that can be bound at the same time.
const U MAX_BOUND_RESOURCE_GROUPS = 2;
/// An anoying limit for Vulkan.
const U MAX_RESOURCE_GROUPS = 1024;

/// The life expectancy of a TransientMemoryToken.
enum class TransientMemoryTokenLifetime : U8
{
	PER_FRAME,
	PERSISTENT
};

/// Token that gets returned when requesting for memory to write to a resource.
class TransientMemoryToken
{
anki_internal:
	PtrSize m_offset = 0;
	PtrSize m_range = 0;
	TransientMemoryTokenLifetime m_lifetime =
		TransientMemoryTokenLifetime::PER_FRAME;
	BufferUsage m_usage = BufferUsage::COUNT;

	void markUnused()
	{
		m_offset = m_range = MAX_U32;
	}
};

/// Struct to help update the offset of the dynamic buffers.
class TransientMemoryInfo
{
public:
	Array<TransientMemoryToken, MAX_UNIFORM_BUFFER_BINDINGS> m_uniformBuffers;
	Array<TransientMemoryToken, MAX_STORAGE_BUFFER_BINDINGS> m_storageBuffers;
	Array<TransientMemoryToken, MAX_VERTEX_ATTRIBUTES> m_vertexBuffers;
};

/// Compute max number of mipmaps for a 2D texture.
inline U computeMaxMipmapCount(U w, U h)
{
	U s = (w < h) ? w : h;
	U count = 0;
	while(s)
	{
		s /= 2;
		++count;
	}

	return count;
}

/// Compute max number of mipmaps for a 3D texture.
inline U computeMaxMipmapCount(U w, U h, U d)
{
	U s = (w < h) ? w : h;
	s = (s < d) ? s : d;
	U count = 0;
	while(s)
	{
		s /= 2;
		++count;
	}

	return count;
}

/// Internal function that logs a shader error.
void logShaderErrorCode(const CString& error,
	const CString& source,
	GenericMemoryPoolAllocator<U8> alloc);
/// @}

} // end namespace anki
