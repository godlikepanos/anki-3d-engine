// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Enums.h>
#include <AnKi/Util/Allocator.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Util/String.h>

namespace anki {

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
class AccelerationStructureInitInfo;
class GrUpscalerInitInfo;
/// @addtogroup graphics
/// @{

#define ANKI_GR_LOGI(...) ANKI_LOG("GR  ", NORMAL, __VA_ARGS__)
#define ANKI_GR_LOGE(...) ANKI_LOG("GR  ", ERROR, __VA_ARGS__)
#define ANKI_GR_LOGW(...) ANKI_LOG("GR  ", WARNING, __VA_ARGS__)
#define ANKI_GR_LOGF(...) ANKI_LOG("GR  ", FATAL, __VA_ARGS__)

// Some constants
constexpr U32 MAX_VERTEX_ATTRIBUTES = 8;
constexpr U32 MAX_COLOR_ATTACHMENTS = 4;
constexpr U32 MAX_DESCRIPTOR_SETS = 3; ///< Groups that can be bound at the same time.
constexpr U32 MAX_BINDINGS_PER_DESCRIPTOR_SET = 32;
constexpr U32 MAX_FRAMES_IN_FLIGHT = 3; ///< Triple buffering.
constexpr U32 MAX_GR_OBJECT_NAME_LENGTH = 31;
constexpr U32 MAX_BINDLESS_TEXTURES = 512;
constexpr U32 MAX_BINDLESS_READONLY_TEXTURE_BUFFERS = 512;

/// The number of commands in a command buffer that make it a small batch command buffer.
constexpr U32 COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS = 100;

/// Smart pointer for resources.
template<typename T>
using GrObjectPtrT = IntrusivePtr<T, DefaultPtrDeleter<GrObject>>;

using GrObjectPtr = GrObjectPtrT<GrObject>;

#define ANKI_GR_CLASS(x_) \
	class x_##Impl; \
	class x_; \
	using x_##Ptr = GrObjectPtrT<x_>;

ANKI_GR_CLASS(Buffer)
ANKI_GR_CLASS(Texture)
ANKI_GR_CLASS(TextureView)
ANKI_GR_CLASS(Sampler)
ANKI_GR_CLASS(CommandBuffer)
ANKI_GR_CLASS(Shader)
ANKI_GR_CLASS(Framebuffer)
ANKI_GR_CLASS(OcclusionQuery)
ANKI_GR_CLASS(TimestampQuery)
ANKI_GR_CLASS(ShaderProgram)
ANKI_GR_CLASS(Fence)
ANKI_GR_CLASS(RenderGraph)
ANKI_GR_CLASS(AccelerationStructure)
ANKI_GR_CLASS(GrUpscaler)

#undef ANKI_GR_CLASS

#define ANKI_GR_OBJECT \
	friend class GrManager; \
	template<typename, typename> \
	friend class IntrusivePtr; \
	template<typename, typename> \
	friend class GenericPoolAllocator;

/// Shader block information.
class ShaderVariableBlockInfo
{
public:
	I16 m_offset = -1; ///< Offset inside the block

	I16 m_arraySize = -1; ///< Number of elements.

	/// Stride between the each array element if the variable is array.
	I16 m_arrayStride = -1;

	/// Identifying the stride between columns of a column-major matrix or rows of a row-major matrix.
	I16 m_matrixStride = -1;
};

/// Knowing the vendor allows some optimizations
enum class GpuVendor : U8
{
	UNKNOWN,
	ARM,
	NVIDIA,
	AMD,
	INTEL,
	QUALCOMM,
	COUNT
};

extern Array<CString, U(GpuVendor::COUNT)> GPU_VENDOR_STR;

/// Device capabilities.
ANKI_BEGIN_PACKED_STRUCT
class GpuDeviceCapabilities
{
public:
	/// The alignment of offsets when bounding uniform buffers.
	U32 m_uniformBufferBindOffsetAlignment = MAX_U32;

	/// The max visible range of uniform buffers inside the shaders.
	PtrSize m_uniformBufferMaxRange = 0;

	/// The alignment of offsets when bounding storage buffers.
	U32 m_storageBufferBindOffsetAlignment = MAX_U32;

	/// The max visible range of storage buffers inside the shaders.
	PtrSize m_storageBufferMaxRange = 0;

	/// The alignment of offsets when bounding texture buffers.
	U32 m_textureBufferBindOffsetAlignment = MAX_U32;

	/// The max visible range of texture buffers inside the shaders.
	PtrSize m_textureBufferMaxRange = 0;

	/// Max push constant size.
	PtrSize m_pushConstantsSize = 128;

	/// The max combined size of shared variables (with paddings) in compute shaders.
	PtrSize m_computeSharedMemorySize = 16_KB;

	/// Each SBT record should be a multiple of this.
	U32 m_sbtRecordAlignment = MAX_U32;

	/// The size of a shader group handle that will be placed inside an SBT record.
	U32 m_shaderGroupHandleSize = 0;

	/// Min subgroup size of the GPU.
	U32 m_minSubgroupSize = 0;

	/// Max subgroup size of the GPU.
	U32 m_maxSubgroupSize = 0;

	/// Min size of a texel in the shading rate image.
	U32 m_minShadingRateImageTexelSize = 0;

	/// GPU vendor.
	GpuVendor m_gpuVendor = GpuVendor::UNKNOWN;

	/// Descrete or integrated GPU.
	Bool m_discreteGpu = false;

	/// API version.
	U8 m_minorApiVersion = 0;

	/// API version.
	U8 m_majorApiVersion = 0;

	/// RT.
	Bool m_rayTracingEnabled = false;

	/// 64 bit atomics.
	Bool m_64bitAtomics = false;

	/// VRS.
	Bool m_vrs = false;

	/// Supports min/max texture filtering.
	Bool m_samplingFilterMinMax = false;

	/// Supports or not 24bit, 48bit or 96bit texture formats.
	Bool m_unalignedBbpTextureFormats = false;

	/// DLSS.
	Bool m_dlss = false;
};
ANKI_END_PACKED_STRUCT
static_assert(sizeof(GpuDeviceCapabilities)
				  == sizeof(PtrSize) * 5 + sizeof(U32) * 8 + sizeof(U8) * 3 + sizeof(Bool) * 7,
			  "Should be packed");

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
		zeroMemory(*this);
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

	TextureSurfaceInfo(U32 level, U32 depth, U32 face, U32 layer)
		: m_level(level)
		, m_depth(depth)
		, m_face(face)
		, m_layer(layer)
	{
	}

	TextureSurfaceInfo& operator=(const TextureSurfaceInfo&) = default;

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

	TextureVolumeInfo(U32 level)
		: m_level(level)
	{
	}

	TextureVolumeInfo& operator=(const TextureVolumeInfo&) = default;
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
		, m_firstFace(U8(surf.m_face))
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

	TextureSubresourceInfo& operator=(const TextureSubresourceInfo&) = default;

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
		zeroMemory(m_name);

		U32 len;
		if(name && (len = name.getLength()) > 0)
		{
			len = min(len, MAX_GR_OBJECT_NAME_LENGTH);
			memcpy(&m_name[0], &name[0], len);
		}
	}

private:
	Array<char, MAX_GR_OBJECT_NAME_LENGTH + 1> m_name;
};

/// Compute max number of mipmaps for a 2D texture.
inline U32 computeMaxMipmapCount2d(U32 w, U32 h, U32 minSizeOfLastMip = 1)
{
	ANKI_ASSERT(w >= minSizeOfLastMip && h >= minSizeOfLastMip);
	U32 s = (w < h) ? w : h;
	U32 count = 0;
	while(s >= minSizeOfLastMip)
	{
		s /= 2;
		++count;
	}

	return count;
}

/// Compute max number of mipmaps for a 3D texture.
inline U32 computeMaxMipmapCount3d(U32 w, U32 h, U32 d, U32 minSizeOfLastMip = 1)
{
	U32 s = (w < h) ? w : h;
	s = (s < d) ? s : d;
	U32 count = 0;
	while(s >= minSizeOfLastMip)
	{
		s /= 2;
		++count;
	}

	return count;
}

/// Compute the size in bytes of a texture surface surface.
PtrSize computeSurfaceSize(U32 width, U32 height, Format fmt);

/// Compute the size in bytes of the texture volume.
PtrSize computeVolumeSize(U32 width, U32 height, U32 depth, Format fmt);
/// @}

} // end namespace anki
