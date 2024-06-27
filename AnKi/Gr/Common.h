// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Allocator.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Shaders/Include/Common.h>

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
class PipelineQueryInitInfo;

/// @addtogroup graphics
/// @{

#define ANKI_GR_LOGI(...) ANKI_LOG("GR", kNormal, __VA_ARGS__)
#define ANKI_GR_LOGE(...) ANKI_LOG("GR", kError, __VA_ARGS__)
#define ANKI_GR_LOGW(...) ANKI_LOG("GR", kWarning, __VA_ARGS__)
#define ANKI_GR_LOGF(...) ANKI_LOG("GR", kFatal, __VA_ARGS__)
#define ANKI_GR_LOGV(...) ANKI_LOG("GR", kVerbose, __VA_ARGS__)

class GrMemoryPool : public HeapMemoryPool, public MakeSingleton<GrMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

private:
	GrMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "GrMemPool")
	{
	}

	~GrMemoryPool() = default;
};

ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(Gr, GrMemoryPool)

// Some constants
constexpr U32 kMaxColorRenderTargets = 4;
constexpr U32 kMaxDescriptorSets = 3; ///< Groups that can be bound at the same time.
constexpr U32 kMaxBindingsPerDescriptorSet = 32;
constexpr U32 kMaxFramesInFlight = 3; ///< Triple buffering.
constexpr U32 kMaxGrObjectNameLength = 61;
constexpr U32 kMaxBindlessTextures = 512;
constexpr U32 kMaxBindlessReadonlyTextureBuffers = 512;
constexpr U32 kMaxPushConstantSize = 128; ///< Thanks AMD!!

/// The number of commands in a command buffer that make it a small batch command buffer.
constexpr U32 kCommandBufferSmallBatchMaxCommands = 100;

class GrObjectDeleter
{
public:
	void operator()(GrObject* ptr);
};

/// Smart pointer for resources.
template<typename T>
using GrObjectPtrT = IntrusivePtr<T, GrObjectDeleter>;

using GrObjectPtr = GrObjectPtrT<GrObject>;

#define ANKI_GR_CLASS(x_) \
	class x_##Impl; \
	class x_; \
	using x_##Ptr = GrObjectPtrT<x_>;

ANKI_GR_CLASS(Buffer)
ANKI_GR_CLASS(Texture)
ANKI_GR_CLASS(Sampler)
ANKI_GR_CLASS(CommandBuffer)
ANKI_GR_CLASS(Shader)
ANKI_GR_CLASS(Framebuffer)
ANKI_GR_CLASS(OcclusionQuery)
ANKI_GR_CLASS(TimestampQuery)
ANKI_GR_CLASS(PipelineQuery)
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
	template<typename T, typename... TArgs> \
	friend void callConstructor(T& p, TArgs&&... args);

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
	kUnknown,
	kArm,
	kNvidia,
	kAMD,
	kIntel,
	kQualcomm,
	kCount
};

inline constexpr Array<CString, U(GpuVendor::kCount)> kGPUVendorStrings = {"unknown", "ARM", "nVidia", "AMD", "Intel", "Qualcomm"};

/// Device capabilities.
ANKI_BEGIN_PACKED_STRUCT
class GpuDeviceCapabilities
{
public:
	/// The alignment of offsets when bounding constant buffers.
	U32 m_uniformBufferBindOffsetAlignment = kMaxU32;

	/// The max visible range of constant buffers inside the shaders.
	PtrSize m_uniformBufferMaxRange = 0;

	/// The alignment of offsets when bounding storage buffers.
	U32 m_storageBufferBindOffsetAlignment = kMaxU32;

	/// The max visible range of storage buffers inside the shaders.
	PtrSize m_storageBufferMaxRange = 0;

	/// The alignment of offsets when bounding texture buffers.
	U32 m_texelBufferBindOffsetAlignment = kMaxU32;

	/// The max visible range of texture buffers inside the shaders.
	PtrSize m_textureBufferMaxRange = 0;

	/// Max push constant size.
	PtrSize m_pushConstantsSize = 128;

	/// The max combined size of shared variables (with paddings) in compute shaders.
	PtrSize m_computeSharedMemorySize = 16_KB;

	/// Alignment of the scratch buffer used in AS building.
	U32 m_accelerationStructureBuildScratchOffsetAlignment = 0;

	/// Each SBT record should be a multiple of this.
	U32 m_sbtRecordAlignment = kMaxU32;

	/// The size of a shader group handle that will be placed inside an SBT record.
	U32 m_shaderGroupHandleSize = 0;

	/// Min subgroup size of the GPU.
	U32 m_minSubgroupSize = 0;

	/// Max subgroup size of the GPU.
	U32 m_maxSubgroupSize = 0;

	/// Min size of a texel in the shading rate image.
	U32 m_minShadingRateImageTexelSize = 0;

	/// The max number of drawcalls in draw indirect count calls.
	U32 m_maxDrawIndirectCount = 0;

	/// GPU vendor.
	GpuVendor m_gpuVendor = GpuVendor::kUnknown;

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

	/// Mesh shaders.
	Bool m_meshShaders = false;

	/// Can create PipelineQuery objects.
	Bool m_pipelineQuery = false;

	/// Has access to barycentrics.
	Bool m_barycentrics = false;

	/// WorkGraphs
	Bool m_workGraphs = false;
};
ANKI_END_PACKED_STRUCT

/// The type of the allocator for heap allocations
template<typename T>
using GrAllocator = HeapAllocator<T>;

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
			len = min(len, kMaxGrObjectNameLength);
			memcpy(&m_name[0], &name[0], len);
		}
	}

private:
	Array<char, kMaxGrObjectNameLength + 1> m_name;
};

enum class ColorBit : U8
{
	kNone = 0,
	kRed = 1 << 0,
	kGreen = 1 << 1,
	kBlue = 1 << 2,
	kAlpha = 1 << 3,
	kAll = kRed | kGreen | kBlue | kAlpha
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ColorBit)

enum class PrimitiveTopology : U8
{
	kPoints,
	kLines,
	kLineStip,
	kTriangles,
	kTriangleStrip,
	kPatches
};

enum class FillMode : U8
{
	kPoints,
	kWireframe,
	kSolid,
	kCount
};

enum class FaceSelectionBit : U8
{
	kNone = 0,
	kFront = 1 << 0,
	kBack = 1 << 1,
	kFrontAndBack = kFront | kBack
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(FaceSelectionBit)

enum class CompareOperation : U8
{
	kAlways,
	kLess,
	kEqual,
	kLessEqual,
	kGreater,
	kGreaterEqual,
	kNotEqual,
	kNever,
	kCount
};

enum class StencilOperation : U8
{
	kKeep,
	kZero,
	kReplace,
	kIncrementAndClamp,
	kDecrementAndClamp,
	kInvert,
	kIncrementAndWrap,
	kDecrementAndWrap,
	kCount
};

enum class BlendFactor : U8
{
	kZero,
	kOne,
	kSrcColor,
	kOneMinusSrcColor,
	kDstColor,
	kOneMinusDstColor,
	kSrcAlpha,
	kOneMinusSrcAlpha,
	kDstAlpha,
	kOneMinusDstAlpha,
	kConstantColor,
	kOneMinusConstantColor,
	kConstantAlpha,
	kOneMinusConstantAlpha,
	kSrcAlphaSaturate,
	kSrc1Color,
	kOneMinusSrc1Color,
	kSrc1Alpha,
	kOneMinusSrc1Alpha,
	kCount
};

enum class BlendOperation : U8
{
	kAdd,
	kSubtract,
	kReverseSubtract,
	kMin,
	kMax,
	kCount
};

enum class VertexStepRate : U8
{
	kVertex,
	kInstance,
	kCount
};

/// A way to distinguish the aspect of a depth stencil texture.
enum class DepthStencilAspectBit : U8
{
	kNone = 0,
	kDepth = 1 << 0,
	kStencil = 1 << 1,
	kDepthStencil = kDepth | kStencil
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DepthStencilAspectBit)

/// Pixel or vertex format.
/// WARNING: Keep it the same as vulkan (one conversion less).
enum class Format : U32
{
	kNone = 0,

#define ANKI_FORMAT_DEF(type, vk, d3d, componentCount, texelSize, blockWidth, blockHeight, blockSize, shaderType, depthStencil) k##type = vk,
#include <AnKi/Gr/BackendCommon/Format.def.h>
#undef ANKI_FORMAT_DEF
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(Format)

/// Contains info for a specific Format.
class FormatInfo
{
public:
	U8 m_componentCount; ///< The number of components.
	U8 m_texelSize; ///< The size of the texel. Only for incompressed, zero for compressed.
	U8 m_blockWidth; ///< The width of the block size of compressed formats. Zero otherwise.
	U8 m_blockHeight; ///< The height of the block size of compressed formats. Zero otherwise.
	U8 m_blockSize; ///< The size of the block of a compressed format. Zero otherwise.
	U8 m_shaderType; ///< It's 0 if the shader sees it as float, 1 if uint and 2 if signed int.
	DepthStencilAspectBit m_depthStencil; ///< Depth/stencil mask.
	const char* m_name;

	Bool isDepthStencil() const
	{
		return m_depthStencil != DepthStencilAspectBit::kNone;
	}

	Bool isDepth() const
	{
		return !!(m_depthStencil & DepthStencilAspectBit::kDepth);
	}

	Bool isStencil() const
	{
		return !!(m_depthStencil & DepthStencilAspectBit::kStencil);
	}

	Bool isCompressed() const
	{
		return m_blockSize > 0;
	}
};

/// Get info for a specific Format.
ANKI_PURE FormatInfo getFormatInfo(Format fmt);

/// Compute the size in bytes of a texture surface surface.
ANKI_PURE PtrSize computeSurfaceSize(U32 width, U32 height, Format fmt);

/// Compute the size in bytes of the texture volume.
ANKI_PURE PtrSize computeVolumeSize(U32 width, U32 height, U32 depth, Format fmt);

/// Texture type.
enum class TextureType : U8
{
	k1D,
	k2D,
	k3D,
	k2DArray,
	kCube,
	kCubeArray,
	kCount
};

inline Bool textureTypeIsCube(const TextureType t)
{
	return t == TextureType::kCube || t == TextureType::kCubeArray;
}

/// Texture usage hints. They are very important.
enum class TextureUsageBit : U32
{
	kNone = 0,

	kSampledGeometry = 1 << 0,
	kSampledFragment = 1 << 1,
	kSampledCompute = 1 << 2,
	kSampledTraceRays = 1 << 3,

	kStorageGeometryRead = 1 << 4,
	kStorageGeometryWrite = 1 << 5,
	kStorageFragmentRead = 1 << 6,
	kStorageFragmentWrite = 1 << 7,
	kStorageComputeRead = 1 << 8,
	kStorageComputeWrite = 1 << 9,
	kStorageTraceRaysRead = 1 << 10,
	kStorageTraceRaysWrite = 1 << 11,

	kFramebufferRead = 1 << 12,
	kFramebufferWrite = 1 << 13,
	kFramebufferShadingRate = 1 << 14,

	kTransferDestination = 1 << 15,

	kPresent = 1 << 17,

	// Derived
	kAllSampled = kSampledGeometry | kSampledFragment | kSampledCompute | kSampledTraceRays,
	kAllStorage = kStorageGeometryRead | kStorageGeometryWrite | kStorageFragmentRead | kStorageFragmentWrite | kStorageComputeRead
				  | kStorageComputeWrite | kStorageTraceRaysRead | kStorageTraceRaysWrite,
	kAllFramebuffer = kFramebufferRead | kFramebufferWrite,

	kAllGeometry = kSampledGeometry | kStorageGeometryRead | kStorageGeometryWrite,
	kAllFragment = kSampledFragment | kStorageFragmentRead | kStorageFragmentWrite,
	kAllGraphics = kSampledGeometry | kSampledFragment | kStorageGeometryRead | kStorageGeometryWrite | kStorageFragmentRead | kStorageFragmentWrite
				   | kFramebufferRead | kFramebufferWrite | kFramebufferShadingRate,
	kAllCompute = kSampledCompute | kStorageComputeRead | kStorageComputeWrite,
	kAllTransfer = kTransferDestination,

	kAllRead = kAllSampled | kStorageGeometryRead | kStorageFragmentRead | kStorageComputeRead | kStorageTraceRaysRead | kFramebufferRead
			   | kFramebufferShadingRate | kPresent,
	kAllWrite =
		kStorageGeometryWrite | kStorageFragmentWrite | kStorageComputeWrite | kStorageTraceRaysWrite | kFramebufferWrite | kTransferDestination,
	kAll = kAllRead | kAllWrite,
	kAllShaderResource = kAllSampled | kAllStorage,
	kAllSrv = (kAllSampled | kAllStorage) & kAllRead,
	kAllUav = kAllStorage & kAllWrite,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TextureUsageBit)

enum class SamplingFilter : U8
{
	kNearest,
	kLinear,
	kMin, ///< It calculates the min of a 2x2 quad. Only if GpuDeviceCapabilities::m_samplingFilterMinMax is supported.
	kMax, ///< It calculates the max of a 2x2 quad. Only if GpuDeviceCapabilities::m_samplingFilterMinMax is supported.
};

enum class SamplingAddressing : U8
{
	kClamp,
	kRepeat,
	kBlack,
	kWhite,

	kCount,
	kFirst = 0,
	kLast = kCount - 1,
};

enum class ShaderType : U16
{
	kVertex,
	kTessellationControl,
	kTessellationEvaluation,
	kGeometry,
	kTask,
	kMesh,
	kFragment,
	kCompute,
	kRayGen,
	kAnyHit,
	kClosestHit,
	kMiss,
	kIntersection,
	kCallable,
	kWorkGraph,

	kCount,
	kFirst = 0,
	kLast = kCount - 1,
	kFirstGraphics = kVertex,
	kLastGraphics = kFragment,
	kFirstRayTracing = kRayGen,
	kLastRayTracing = kCallable,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderType)

enum class ShaderTypeBit : U16
{
	kVertex = 1 << 0,
	kTessellationControl = 1 << 1,
	kTessellationEvaluation = 1 << 2,
	kGeometry = 1 << 3,
	kTask = 1 << 4,
	kMesh = 1 << 5,
	kFragment = 1 << 6,
	kCompute = 1 << 7,
	kRayGen = 1 << 8,
	kAnyHit = 1 << 9,
	kClosestHit = 1 << 10,
	kMiss = 1 << 11,
	kIntersection = 1 << 12,
	kCallable = 1 << 13,
	kWorkGraph = 1 << 14,

	kNone = 0,
	kAllGraphics = kVertex | kTessellationControl | kTessellationEvaluation | kGeometry | kTask | kMesh | kFragment,
	kAllLegacyGeometry = kVertex | kTessellationControl | kTessellationEvaluation | kGeometry,
	kAllModernGeometry = kTask | kMesh,
	kAllRayTracing = kRayGen | kAnyHit | kClosestHit | kMiss | kIntersection | kCallable,
	kAllHit = kAnyHit | kClosestHit,
	kAll = kAllGraphics | kCompute | kAllRayTracing | kWorkGraph,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderTypeBit)

enum class ShaderVariableDataType : U8
{
	kNone,

#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) k##type,
#define ANKI_SVDT_MACRO_OPAQUE(constant, type) k##constant,
#include <AnKi/Gr/ShaderVariableDataType.def.h>
#undef ANKI_SVDT_MACRO
#undef ANKI_SVDT_MACRO_OPAQUE

	// Derived
	kCount,

	kNumericsFirst = kI32,
	kNumericsLast = kMat4,

	kNumeric1ComponentFirst = kI32,
	kNumeric1ComponentLast = kF32,
	kNumeric2ComponentFirst = kIVec2,
	kNumeric2ComponentLast = kVec2,
	kNumeric3ComponentFirst = kIVec3,
	kNumeric3ComponentLast = kVec3,
	kNumeric4ComponentFirst = kIVec4,
	kNumeric4ComponentLast = kVec4,

	kMatrixFirst = kMat3,
	kMatrixLast = kMat4,

	kTextureFirst = kTexture1D,
	kTextureLast = kTextureCubeArray,

	kImageFirst = kImage1D,
	kImageLast = kImageCubeArray,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderVariableDataType)

class ShaderVariableDataTypeInfo
{
public:
	const Char* m_name;
	U32 m_size; ///< Size of the type.
	Bool m_opaque;
	Bool m_isIntegral; ///< If true is integral type. Else it's float.
};

ANKI_PURE const ShaderVariableDataTypeInfo& getShaderVariableDataTypeInfo(ShaderVariableDataType type);

/// Occlusion query result bit.
enum class OcclusionQueryResultBit : U8
{
	kNotAvailable = 1 << 0,
	kVisible = 1 << 1,
	kNotVisible = 1 << 2
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(OcclusionQueryResultBit)

/// Occlusion query result.
enum class OcclusionQueryResult : U8
{
	kNotAvailable,
	kVisible,
	kNotVisible
};

/// Timestamp query result.
enum class TimestampQueryResult : U8
{
	kNotAvailable,
	kAvailable
};

/// Pipeline query result.
enum class PipelineQueryResult : U8
{
	kNotAvailable,
	kAvailable
};

enum class PipelineQueryType : U8
{
	kPrimitivesPassedClipping,
	kCount
};

/// Attachment load operation.
enum class RenderTargetLoadOperation : U8
{
	kLoad,
	kClear,
	kDontCare
};

/// Attachment store operation.
enum class RenderTargetStoreOperation : U8
{
	kStore,
	kDontCare
};

/// Buffer usage modes.
/// The graphics work consists of the following pipes: indirect, geometry (all programmable and fixed function geometry stages) and finaly fragment.
/// The compute from the consists of the following: indirect and compute.
/// The trace rays from the: indirect and trace_rays
/// !!WARNING!! If you change this remember to change PrivateBufferUsageBit.
enum class BufferUsageBit : U64
{
	kNone = 0,

	kUniformGeometry = 1ull << 0ull,
	kUniformFragment = 1ull << 1ull,
	kUniformCompute = 1ull << 2ull,
	kUniformTraceRays = 1ull << 3ull,

	kStorageGeometryRead = 1ull << 4ull,
	kStorageGeometryWrite = 1ull << 5ull,
	kStorageFragmentRead = 1ull << 6ull,
	kStorageFragmentWrite = 1ull << 7ull,
	kStorageComputeRead = 1ull << 8ull,
	kStorageComputeWrite = 1ull << 9ull,
	kStorageTraceRaysRead = 1ull << 10ull,
	kStorageTraceRaysWrite = 1ull << 11ull,

	kTexelGeometryRead = 1ull << 12ull,
	kTexelGeometryWrite = 1ull << 13ull,
	kTexelFragmentRead = 1ull << 14ull,
	kTexelFragmentWrite = 1ull << 15ull,
	kTexelComputeRead = 1ull << 16ull,
	kTexelComputeWrite = 1ull << 17ull,
	kTexelTraceRaysRead = 1ull << 18ull,
	kTexelTraceRaysWrite = 1ull << 19ull,

	kIndex = 1ull << 20ull,
	kVertex = 1ull << 21ull,

	kIndirectCompute = 1ull << 22ll,
	kIndirectDraw = 1ull << 23ull,
	kIndirectTraceRays = 1ull << 24ull,

	kTransferSource = 1ull << 25ull,
	kTransferDestination = 1ull << 26ull,

	kAccelerationStructureBuild = 1ull << 27ull, ///< Will be used as a position or index buffer in a BLAS build.
	kShaderBindingTable = 1ull << 28ull, ///< Will be used as SBT in a traceRays() command.
	kAccelerationStructureBuildScratch = 1ull << 29ull, ///< Used in buildAccelerationStructureXXX commands.

	// Derived
	kAllUniform = kUniformGeometry | kUniformFragment | kUniformCompute | kUniformTraceRays,
	kAllStorage = kStorageGeometryRead | kStorageGeometryWrite | kStorageFragmentRead | kStorageFragmentWrite | kStorageComputeRead
				  | kStorageComputeWrite | kStorageTraceRaysRead | kStorageTraceRaysWrite,
	kAllTexel = kTexelGeometryRead | kTexelGeometryWrite | kTexelFragmentRead | kTexelFragmentWrite | kTexelComputeRead | kTexelComputeWrite
				| kTexelTraceRaysRead | kTexelTraceRaysWrite,
	kAllIndirect = kIndirectCompute | kIndirectDraw | kIndirectTraceRays,
	kAllTransfer = kTransferSource | kTransferDestination,

	kAllGeometry = kUniformGeometry | kStorageGeometryRead | kStorageGeometryWrite | kTexelGeometryRead | kTexelGeometryWrite | kIndex | kVertex,
	kAllFragment = kUniformFragment | kStorageFragmentRead | kStorageFragmentWrite | kTexelFragmentRead | kTexelFragmentWrite,
	kAllGraphics = kAllGeometry | kAllFragment | kIndirectDraw,
	kAllCompute = kUniformCompute | kStorageComputeRead | kStorageComputeWrite | kTexelComputeRead | kTexelComputeWrite | kIndirectCompute,
	kAllTraceRays = kUniformTraceRays | kStorageTraceRaysRead | kStorageTraceRaysWrite | kTexelTraceRaysRead | kTexelTraceRaysWrite
					| kIndirectTraceRays | kShaderBindingTable,

	kAllRayTracing = kAllTraceRays | kAccelerationStructureBuild | kAccelerationStructureBuildScratch,
	kAllRead = kAllUniform | kStorageGeometryRead | kStorageFragmentRead | kStorageComputeRead | kStorageTraceRaysRead | kTexelGeometryRead
			   | kTexelFragmentRead | kTexelComputeRead | kTexelTraceRaysRead | kIndex | kVertex | kIndirectCompute | kIndirectDraw
			   | kIndirectTraceRays | kTransferSource | kAccelerationStructureBuild | kShaderBindingTable,
	kAllWrite = kStorageGeometryWrite | kStorageFragmentWrite | kStorageComputeWrite | kStorageTraceRaysWrite | kTexelGeometryWrite
				| kTexelFragmentWrite | kTexelComputeWrite | kTexelTraceRaysWrite | kTransferDestination | kAccelerationStructureBuildScratch,

	kAll = kAllRead | kAllWrite,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BufferUsageBit)

/// Buffer access when mapped.
enum class BufferMapAccessBit : U8
{
	kNone = 0,
	kRead = 1 << 0,
	kWrite = 1 << 1,
	kReadWrite = kRead | kWrite
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BufferMapAccessBit)

/// Index buffer's index type.
enum class IndexType : U8
{
	kU16,
	kU32,
	kCount
};

inline U32 getIndexSize(IndexType type)
{
	ANKI_ASSERT(type < IndexType::kCount);
	return 2u << U32(type);
}

/// Acceleration structure type.
enum class AccelerationStructureType : U8
{
	kTopLevel,
	kBottomLevel,
	kCount
};

enum class AccelerationStructureUsageBit : U8
{
	kNone = 0,
	kBuild = 1 << 0,
	kAttach = 1 << 1, ///< Attached to a TLAS. Only for BLAS.
	kGeometryRead = 1 << 2,
	kFragmentRead = 1 << 3,
	kComputeRead = 1 << 4,
	kTraceRaysRead = 1 << 5,

	// Derived
	kAllGraphics = kGeometryRead | kFragmentRead,
	kAllRead = kAttach | kGeometryRead | kFragmentRead | kComputeRead | kTraceRaysRead,
	kAllWrite = kBuild
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(AccelerationStructureUsageBit)

/// VRS rates.
enum class VrsRate : U8
{
	k1x1, ///< Disable VRS. Always supported.
	k2x1, ///< Always supported.
	k1x2,
	k2x2, ///< Always supported.
	k4x2,
	k2x4,
	k4x4,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VrsRate)

enum class GpuQueueType : U8
{
	kGeneral,
	kCompute,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GpuQueueType)

enum class HlslResourceType : U8
{
	kCbv,
	kSrv,
	kUav,
	kSampler, // !!!!WARNING!!! Keep it last

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(HlslResourceType)

enum class VertexAttributeSemantic : U8
{
	kPosition,
	kNormal,
	kTexCoord,
	kColor,
	kMisc0,
	kMisc1,
	kMisc2,
	kMisc3,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexAttributeSemantic)

/// This doesn't match Vulkan or D3D.
enum class DescriptorType : U8
{
	kTexture, ///< Vulkan: Image (sampled or storage). D3D: Textures
	kSampler,
	kUniformBuffer,
	kStorageBuffer, ///< Vulkan: Storage buffers. D3D: Structured, ByteAddressBuffer
	kTexelBuffer,
	kAccelerationStructure,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DescriptorType)

enum class DescriptorFlag : U8
{
	kNone,
	kRead = 1 << 0,
	kWrite = 1 << 1,
	kReadWrite = kRead | kWrite,
	kByteAddressBuffer = 1 << 2
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DescriptorFlag)

inline HlslResourceType descriptorTypeToHlslResourceType(DescriptorType type, DescriptorFlag flag)
{
	ANKI_ASSERT(type < DescriptorType::kCount && flag != DescriptorFlag::kNone);
	if(type == DescriptorType::kSampler)
	{
		return HlslResourceType::kSampler;
	}
	else if(type == DescriptorType::kUniformBuffer)
	{
		return HlslResourceType::kCbv;
	}
	else if(!!(flag & DescriptorFlag::kWrite))
	{
		return HlslResourceType::kUav;
	}
	else
	{
		return HlslResourceType::kSrv;
	}
}

ANKI_BEGIN_PACKED_STRUCT
class ShaderReflectionBinding
{
public:
	U32 m_registerBindingPoint = kMaxU32;
	U16 m_arraySize = 0;

	union
	{
		U16 m_vkBinding = kMaxU16; ///< Filled by the VK backend.
		U16 m_d3dStructuredBufferStride;
	};

	DescriptorType m_type = DescriptorType::kCount;
	DescriptorFlag m_flags = DescriptorFlag::kNone;

	Bool operator<(const ShaderReflectionBinding& b) const
	{
#define ANKI_LESS(member) \
	if(member != b.member) \
	{ \
		return member < b.member; \
	}
		const HlslResourceType hlslType = descriptorTypeToHlslResourceType(m_type, m_flags);
		const HlslResourceType bhlslType = descriptorTypeToHlslResourceType(b.m_type, b.m_flags);
		if(hlslType != bhlslType)
		{
			return hlslType < bhlslType;
		}

		ANKI_LESS(m_registerBindingPoint)
		ANKI_LESS(m_arraySize)
		ANKI_LESS(m_d3dStructuredBufferStride)
#undef ANKI_LESS
		return false;
	}

	Bool operator==(const ShaderReflectionBinding& b) const
	{
		return memcmp(this, &b, sizeof(*this)) == 0;
	}

	void validate() const
	{
		ANKI_ASSERT(m_registerBindingPoint < kMaxU32);
		ANKI_ASSERT(m_type < DescriptorType::kCount);
		ANKI_ASSERT(m_flags != DescriptorFlag::kNone);
		ANKI_ASSERT(m_arraySize > 0);
		ANKI_ASSERT(ANKI_GR_BACKEND_VULKAN || m_type != DescriptorType::kStorageBuffer || m_d3dStructuredBufferStride != 0);
	}
};
ANKI_END_PACKED_STRUCT

ANKI_BEGIN_PACKED_STRUCT
class ShaderReflectionDescriptorRelated
{
public:
	Array2d<ShaderReflectionBinding, kMaxDescriptorSets, kMaxBindingsPerDescriptorSet> m_bindings;
	Array<U8, kMaxDescriptorSets> m_bindingCounts = {};

	U32 m_pushConstantsSize = 0;

	Bool m_hasVkBindlessDescriptorSet = false; ///< Filled by the shader compiler.
	U8 m_vkBindlessDescriptorSet = kMaxU8; ///< Filled by the VK backend.

	void validate() const
	{
		for(U32 set = 0; set < kMaxDescriptorSets; ++set)
		{
			for(U32 ibinding = 0; ibinding < kMaxBindingsPerDescriptorSet; ++ibinding)
			{
				const ShaderReflectionBinding& binding = m_bindings[set][ibinding];

				if(binding.m_type != DescriptorType::kCount)
				{
					ANKI_ASSERT(ibinding < m_bindingCounts[set]);
					binding.validate();
				}
				else
				{
					ANKI_ASSERT(ibinding >= m_bindingCounts[set]);
				}
			}
		}
	}
};
ANKI_END_PACKED_STRUCT

class ShaderReflection
{
public:
	ShaderReflectionDescriptorRelated m_descriptor;

	class
	{
	public:
		Array<U8, U32(VertexAttributeSemantic::kCount)> m_vkVertexAttributeLocations;
		BitSet<U32(VertexAttributeSemantic::kCount), U8> m_vertexAttributeMask = {false};
	} m_vertex;

	class
	{
	public:
		BitSet<kMaxColorRenderTargets, U8> m_colorAttachmentWritemask = {false};

		Bool m_discards = false;
	} m_fragment;

	ShaderReflection()
	{
		m_vertex.m_vkVertexAttributeLocations.fill(kMaxU8);
	}

	void validate() const
	{
		m_descriptor.validate();
		for([[maybe_unused]] VertexAttributeSemantic semantic : EnumIterable<VertexAttributeSemantic>())
		{
			ANKI_ASSERT(!m_vertex.m_vertexAttributeMask.get(semantic) || m_vertex.m_vkVertexAttributeLocations[semantic] != kMaxU8);
		}

		const U32 attachmentCount = m_fragment.m_colorAttachmentWritemask.getSetBitCount();
		for(U32 i = 0; i < attachmentCount; ++i)
		{
			ANKI_ASSERT(m_fragment.m_colorAttachmentWritemask.get(i) && "Should write to all attachments");
		}
	}

	/// Combine shader reflection.
	static Error linkShaderReflection(const ShaderReflection& a, const ShaderReflection& b, ShaderReflection& c);
};

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

/// Compute max number of mipmaps for a 2D texture.
U32 computeMaxMipmapCount2d(U32 w, U32 h, U32 minSizeOfLastMip = 1);

/// Compute max number of mipmaps for a 3D texture.
U32 computeMaxMipmapCount3d(U32 w, U32 h, U32 d, U32 minSizeOfLastMip = 1);

/// Visit a SPIR-V binary.
template<typename TArray, typename TFunc>
static void visitSpirv(TArray spv, TFunc func)
{
	ANKI_ASSERT(spv.getSize() > 5);

	auto it = &spv[5];
	do
	{
		const U32 instructionCount = *it >> 16u;
		const U32 opcode = *it & 0xFFFFu;

		TArray instructions(it + 1, instructionCount - 1);

		func(opcode, instructions);

		it += instructionCount;
	} while(it < spv.getEnd());

	ANKI_ASSERT(it == spv.getEnd());
}
/// @}

} // end namespace anki
