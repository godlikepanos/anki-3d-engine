// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
class TextureViewInitInfo;
class SamplerInitInfo;
class GrManagerInitInfo;
class FramebufferInitInfo;
class BufferInitInfo;
class ShaderInitInfo;
class ShaderProgramInitInfo;
class CommandBufferInitInfo;

/// @addtogroup graphics
/// @{

#define ANKI_GR_LOGI(...) ANKI_LOG("GR  ", NORMAL, __VA_ARGS__)
#define ANKI_GR_LOGE(...) ANKI_LOG("GR  ", ERROR, __VA_ARGS__)
#define ANKI_GR_LOGW(...) ANKI_LOG("GR  ", WARNING, __VA_ARGS__)
#define ANKI_GR_LOGF(...) ANKI_LOG("GR  ", FATAL, __VA_ARGS__)

// Some constants
const U MAX_VERTEX_ATTRIBUTES = 8;
const U MAX_COLOR_ATTACHMENTS = 4;
const U MAX_MIPMAPS = 16;
const U MAX_TEXTURE_LAYERS = 32;
const U MAX_SPECIALIZED_CONSTS = 64;

const U MAX_TEXTURE_BINDINGS = 16;
const U MAX_UNIFORM_BUFFER_BINDINGS = 6;
const U MAX_STORAGE_BUFFER_BINDINGS = 4;
const U MAX_IMAGE_BINDINGS = 4;
const U MAX_TEXTURE_BUFFER_BINDINGS = 4;

const U MAX_BINDINGS_PER_DESCRIPTOR_SET = MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS
										  + MAX_STORAGE_BUFFER_BINDINGS + MAX_IMAGE_BINDINGS
										  + MAX_TEXTURE_BUFFER_BINDINGS;

const U MAX_FRAMES_IN_FLIGHT = 3; ///< Triple buffering.
const U MAX_DESCRIPTOR_SETS = 2; ///< Groups that can be bound at the same time.

const U MAX_GR_OBJECT_NAME_LENGTH = 15;

/// The number of commands in a command buffer that make it a small batch command buffer.
const U COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS = 100;

/// Smart pointer for resources.
template<typename T>
using GrObjectPtr = IntrusivePtr<T, DefaultPtrDeleter<T>>;

#define ANKI_GR_CLASS(x_) \
	class x_##Impl; \
	class x_; \
	using x_##Ptr = GrObjectPtr<x_>;

ANKI_GR_CLASS(Buffer)
ANKI_GR_CLASS(Texture)
ANKI_GR_CLASS(TextureView)
ANKI_GR_CLASS(Sampler)
ANKI_GR_CLASS(CommandBuffer)
ANKI_GR_CLASS(Shader)
ANKI_GR_CLASS(Framebuffer)
ANKI_GR_CLASS(OcclusionQuery)
ANKI_GR_CLASS(ShaderProgram)
ANKI_GR_CLASS(Fence)
ANKI_GR_CLASS(RenderGraph)

#undef ANKI_GR_CLASS

#define ANKI_GR_OBJECT \
	friend class GrManager; \
	template<typename, typename> \
	friend class IntrusivePtr; \
	template<typename, typename> \
	friend class GenericPoolAllocator;

/// Knowing the vendor allows some optimizations
enum class GpuVendor : U8
{
	UNKNOWN,
	ARM,
	NVIDIA,
	AMD,
	INTEL,
	COUNT
};

extern Array<CString, U(GpuVendor::COUNT)> GPU_VENDOR_STR;

/// Device capabilities.
class GpuDeviceCapabilities
{
public:
	/// The alignment of offsets when bounding uniform buffers.
	PtrSize m_uniformBufferBindOffsetAlignment = MAX_U32;

	/// The max visible range of uniform buffers inside the shaders.
	PtrSize m_uniformBufferMaxRange = 0;

	/// The alignment of offsets when bounding storage buffers.
	PtrSize m_storageBufferBindOffsetAlignment = MAX_U32;

	/// The max visible range of storage buffers inside the shaders.
	PtrSize m_storageBufferMaxRange = 0;

	/// The alignment of offsets when bounding texture buffers.
	PtrSize m_textureBufferBindOffsetAlignment = MAX_U32;

	/// The max visible range of texture buffers inside the shaders.
	PtrSize m_textureBufferMaxRange = 0;

	/// Max push constant size.
	U32 m_pushConstantsSize = 128;

	/// GPU vendor.
	GpuVendor m_gpuVendor = GpuVendor::UNKNOWN;

	/// API version.
	U8 m_minorApiVersion = 0;

	/// API version.
	U8 m_majorApiVersion = 0;

	// WARNING Remember to pad it because valgrind complains.
	U8 _m_padding[1];
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
		Array<F32, 4> m_colorf;
		Array<I32, 4> m_colori;
		Array<U32, 4> m_coloru;
		Ds m_depthStencil;
	};

	ClearValue()
	{
		memset(this, 0, sizeof(*this));
	}

	ClearValue(const ClearValue& b)
	{
		operator=(b);
	}

	ClearValue& operator=(const ClearValue& b)
	{
		memcpy(this, &b, sizeof(*this));
		return *this;
	}
};

/// A way to identify a surface in a texture.
class TextureSurfaceInfo
{
public:
	U32 m_level = 0;
	U32 m_depth = 0;
	U32 m_face = 0;
	U32 m_layer = 0;

	TextureSurfaceInfo() = default;

	TextureSurfaceInfo(const TextureSurfaceInfo&) = default;

	TextureSurfaceInfo(U level, U depth, U face, U layer)
		: m_level(level)
		, m_depth(depth)
		, m_face(face)
		, m_layer(layer)
	{
	}

	Bool operator==(const TextureSurfaceInfo& b) const
	{
		return m_level == b.m_level && m_depth == b.m_depth && m_face == b.m_face && m_layer == b.m_layer;
	}

	Bool operator!=(const TextureSurfaceInfo& b) const
	{
		return !(*this == b);
	}

	U64 computeHash() const
	{
		return anki::computeHash(this, sizeof(*this), 0x1234567);
	}

	static TextureSurfaceInfo newZero()
	{
		return TextureSurfaceInfo();
	}
};

/// A way to identify a volume in 3D textures.
class TextureVolumeInfo
{
public:
	U32 m_level = 0;

	TextureVolumeInfo() = default;

	TextureVolumeInfo(const TextureVolumeInfo&) = default;

	TextureVolumeInfo(U level)
		: m_level(level)
	{
	}
};

/// Defines a subset of a texture.
class TextureSubresourceInfo
{
public:
	U32 m_firstMipmap = 0;
	U32 m_mipmapCount = 1;

	U32 m_firstLayer = 0;
	U32 m_layerCount = 1;

	U8 m_firstFace = 0;
	U8 m_faceCount = 1;

	DepthStencilAspectBit m_depthStencilAspect = DepthStencilAspectBit::NONE;

	U8 _m_padding[1] = {0};

	TextureSubresourceInfo(DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE)
		: m_depthStencilAspect(aspect)
	{
	}

	TextureSubresourceInfo(const TextureSubresourceInfo&) = default;

	TextureSubresourceInfo(const TextureSurfaceInfo& surf, DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE)
		: m_firstMipmap(surf.m_level)
		, m_mipmapCount(1)
		, m_firstLayer(surf.m_layer)
		, m_layerCount(1)
		, m_firstFace(surf.m_face)
		, m_faceCount(1)
		, m_depthStencilAspect(aspect)
	{
	}

	TextureSubresourceInfo(const TextureVolumeInfo& vol, DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE)
		: m_firstMipmap(vol.m_level)
		, m_mipmapCount(1)
		, m_firstLayer(0)
		, m_layerCount(1)
		, m_firstFace(0)
		, m_faceCount(1)
		, m_depthStencilAspect(aspect)
	{
	}

	Bool operator==(const TextureSubresourceInfo& b) const
	{
		ANKI_ASSERT(_m_padding[0] == b._m_padding[0]);
		return memcmp(this, &b, sizeof(*this)) == 0;
	}

	Bool operator!=(const TextureSubresourceInfo& b) const
	{
		return !(*this == b);
	}

	U64 computeHash() const
	{
		static_assert(sizeof(*this) == sizeof(U32) * 4 + sizeof(U8) * 4, "Should be hashable");
		ANKI_ASSERT(_m_padding[0] == 0);
		return anki::computeHash(this, sizeof(*this));
	}
};

enum class DescriptorType : U8
{
	TEXTURE,
	UNIFORM_BUFFER,
	STORAGE_BUFFER,
	IMAGE,
	TEXTURE_BUFFER,

	COUNT
};

/// The base of all init infos for GR.
class GrBaseInitInfo
{
public:
	/// @name The name of the object.
	GrBaseInitInfo(CString name)
	{
		setName(name);
	}

	GrBaseInitInfo()
		: GrBaseInitInfo(CString())
	{
	}

	GrBaseInitInfo(const GrBaseInitInfo& b)
	{
		m_name = b.m_name;
	}

	GrBaseInitInfo& operator=(const GrBaseInitInfo& b)
	{
		m_name = b.m_name;
		return *this;
	}

	CString getName() const
	{
		return (m_name[0] != '\0') ? CString(&m_name[0]) : CString();
	}

	void setName(CString name)
	{
		// Zero it because the derived classes may be hashed.
		memset(&m_name[0], 0, sizeof(m_name));

		if(name && name.getLength())
		{
			ANKI_ASSERT(name.getLength() <= MAX_GR_OBJECT_NAME_LENGTH);
			memcpy(&m_name[0], &name[0], name.getLength() + 1);
		}
	}

private:
	Array<char, MAX_GR_OBJECT_NAME_LENGTH + 1> m_name;
};

/// Compute max number of mipmaps for a 2D texture.
inline U computeMaxMipmapCount2d(U w, U h, U minSizeOfLastMip = 1)
{
	ANKI_ASSERT(w > minSizeOfLastMip && h > minSizeOfLastMip);
	U s = (w < h) ? w : h;
	U count = 0;
	while(s >= minSizeOfLastMip)
	{
		s /= 2;
		++count;
	}

	return count;
}

/// Compute max number of mipmaps for a 3D texture.
inline U computeMaxMipmapCount3d(U w, U h, U d)
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

/// Compute the size in bytes of a texture surface surface.
PtrSize computeSurfaceSize(U width, U height, Format fmt);

/// Compute the size in bytes of the texture volume.
PtrSize computeVolumeSize(U width, U height, U depth, Format fmt);
/// @}

} // end namespace anki
