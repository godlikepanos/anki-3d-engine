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
#include <AnKi/Util/CVarSet.h>

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

inline BoolCVar g_validationCVar("Gr", "Validation", false, "Enable or not validation");
inline BoolCVar g_gpuValidationCVar("Gr", "GpuValidation", false, "Enable or not GPU validation");
inline BoolCVar g_vsyncCVar("Gr", "Vsync", false, "Enable or not vsync");
inline BoolCVar g_debugMarkersCVar("Gr", "DebugMarkers", false, "Enable or not debug markers");
inline BoolCVar g_meshShadersCVar("Gr", "MeshShaders", false, "Enable or not mesh shaders");
inline NumericCVar<U8> g_deviceCVar("Gr", "Device", 0, 0, 16, "Choose an available device. Devices are sorted by performance");
inline BoolCVar g_rayTracingCVar("Gr", "RayTracing", false, "Try enabling ray tracing");
inline BoolCVar g_vrsCVar("Gr", "Vrs", false, "Enable or not VRS");
inline BoolCVar g_workGraphcsCVar("Gr", "WorkGraphs", false, "Enable or not WorkGraphs");
inline NumericCVar<U32> g_maxBindlessSampledTextureCountCVar("Gr", "MaxBindlessSampledTextureCountCVar", 512, 16, kMaxU16);
inline NumericCVar<Second> g_gpuTimeoutCVar("Gr", "GpuTimeout", 120.0, 0.0, 24.0 * 60.0,
											"Max time to wait for GPU fences or semaphores. More than that it must be a GPU timeout");

#if ANKI_GR_BACKEND_DIRECT3D
inline NumericCVar<U16> g_maxRtvDescriptorsCVar("Gr", "MaxRvtDescriptors", 1024, 8, kMaxU16, "Max number of RTVs");
inline NumericCVar<U16> g_maxDsvDescriptorsCVar("Gr", "MaxDsvDescriptors", 512, 8, kMaxU16, "Max number of DSVs");
inline NumericCVar<U16> g_maxCpuCbvSrvUavDescriptorsCVar("Gr", "MaxCpuCbvSrvUavDescriptors", 16 * 1024, 8, kMaxU16,
														 "Max number of CBV/SRV/UAV descriptors");
inline NumericCVar<U16> g_maxCpuSamplerDescriptorsCVar("Gr", "MaxCpuSamplerDescriptors", 512, 8, kMaxU16, "Max number of sampler descriptors");
inline NumericCVar<U16> g_maxGpuCbvSrvUavDescriptorsCVar("Gr", "MaxGpuCbvSrvUavDescriptors", 16 * 1024, 8, kMaxU16,
														 "Max number of CBV/SRV/UAV descriptors");
inline NumericCVar<U16> g_maxGpuSamplerDescriptorsCVar("Gr", "MaxGpuSamplerDescriptors", 2 * 1024, 8, kMaxU16, "Max number of sampler descriptors");

inline BoolCVar g_dredCVar("Gr", "Dred", false, "Enable DRED");
#else
inline NumericCVar<PtrSize> g_diskShaderCacheMaxSizeCVar("Gr", "DiskShaderCacheMaxSize", 128_MB, 1_MB, 1_GB, "Max size of the pipeline cache file");
inline BoolCVar g_debugPrintfCVar("Gr", "DebugPrintf", false, "Enable or not debug printf");
inline BoolCVar g_samplerFilterMinMaxCVar("Gr", "SamplerFilterMinMax", true, "Enable or not min/max sample filtering");
inline BoolCVar g_asyncComputeCVar("Gr", "AsyncCompute", true, "Enable or not async compute");
inline StringCVar g_vkLayersCVar("Gr", "VkLayers", "", "VK layers to enable. Seperated by :");
#endif

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
constexpr U32 kMaxRegisterSpaces = 3; ///< Groups that can be bound at the same time.
constexpr U32 kMaxBindingsPerRegisterSpace = 32;
constexpr U32 kMaxFramesInFlight = 3; ///< Triple buffering.
constexpr U32 kMaxGrObjectNameLength = 61;
constexpr U32 kMaxFastConstantsSize = 128; ///< Push/root constants size. Thanks AMD!!

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
	U32 m_constantBufferBindOffsetAlignment = kMaxU32;

	/// The alignment of offsets when bounding structured buffers.
	U32 m_structuredBufferBindOffsetAlignment = kMaxU32;

	/// The alignment of offsets when bounding texture buffers.
	U32 m_texelBufferBindOffsetAlignment = kMaxU32;

	/// Max push/root constant size.
	PtrSize m_fastConstantsSize = 128;

	/// The max combined size of shared variables (with paddings) in compute shaders.
	PtrSize m_computeSharedMemorySize = 16_KB;

	/// Alignment of the scratch buffer used in AS building.
	U32 m_accelerationStructureBuildScratchOffsetAlignment = 0;

	/// Each SBT record should be a multiple of this.
	U32 m_sbtRecordAlignment = kMaxU32;

	/// The size of a shader group handle that will be placed inside an SBT record.
	U32 m_shaderGroupHandleSize = 0;

	/// Min subgroup size of the GPU.
	U32 m_minWaveSize = 0;

	/// Max subgroup size of the GPU.
	U32 m_maxWaveSize = 0;

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

	/// Align structured buffers using the structure's size and not the m_storageBufferBindOffsetAlignment.
	Bool m_structuredBufferNaturalAlignment = false;

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

	kSrvGeometry = 1 << 0,
	kSrvPixel = 1 << 1,
	kSrvCompute = 1 << 2,
	kSrvTraceRays = 1 << 3,

	kUavGeometry = 1 << 4,
	kUavPixel = 1 << 5,
	kUavCompute = 1 << 6,
	kUavTraceRays = 1 << 7,

	kRtvDsvRead = 1 << 8,
	kRtvDsvWrite = 1 << 9,
	kShadingRate = 1 << 10,

	kCopyDestination = 1 << 11,

	kPresent = 1 << 12,

	// Derived
	kAllSrv = kSrvGeometry | kSrvPixel | kSrvCompute | kSrvTraceRays,
	kAllUav = kUavGeometry | kUavPixel | kUavCompute | kUavTraceRays,
	kAllRtvDsv = kRtvDsvRead | kRtvDsvWrite,

	kAllGeometry = kSrvGeometry | kUavGeometry,
	kAllPixel = kSrvPixel | kUavPixel,
	kAllGraphics = kAllGeometry | kAllPixel | kRtvDsvRead | kRtvDsvWrite | kShadingRate,
	kAllCompute = kSrvCompute | kUavCompute,
	kAllTransfer = kCopyDestination,

	kAllRead = kAllSrv | kAllUav | kRtvDsvRead | kShadingRate | kPresent,
	kAllWrite = kAllUav | kRtvDsvWrite | kCopyDestination,
	kAll = kAllRead | kAllWrite,
	kAllShaderResource = kAllSrv | kAllUav,
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
	kHull,
	kDomain,
	kGeometry,
	kAmplification,
	kMesh,
	kPixel,
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
	kLastGraphics = kPixel,
	kFirstRayTracing = kRayGen,
	kLastRayTracing = kCallable,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderType)

enum class ShaderTypeBit : U16
{
	kVertex = 1 << 0,
	kHull = 1 << 1,
	kDomain = 1 << 2,
	kGeometry = 1 << 3,
	kAmplification = 1 << 4,
	kMesh = 1 << 5,
	kPixel = 1 << 6,
	kCompute = 1 << 7,
	kRayGen = 1 << 8,
	kAnyHit = 1 << 9,
	kClosestHit = 1 << 10,
	kMiss = 1 << 11,
	kIntersection = 1 << 12,
	kCallable = 1 << 13,
	kWorkGraph = 1 << 14,

	kNone = 0,
	kAllGraphics = kVertex | kHull | kDomain | kGeometry | kAmplification | kMesh | kPixel,
	kAllLegacyGeometry = kVertex | kHull | kDomain | kGeometry,
	kAllModernGeometry = kAmplification | kMesh,
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
/// The graphics work consists of the following pipes: indirect, geometry (all programmable and fixed function geometry stages) and finaly pixel.
/// The compute from the consists of the following: indirect and compute.
/// The trace rays from the: indirect and trace_rays
/// !!WARNING!! If you change this remember to change PrivateBufferUsageBit.
enum class BufferUsageBit : U64
{
	kNone = 0,

	kConstantGeometry = 1ull << 0ull,
	kConstantPixel = 1ull << 1ull,
	kConstantCompute = 1ull << 2ull,
	kConstantTraceRays = 1ull << 3ull,

	kSrvGeometry = 1ull << 4ull,
	kSrvPixel = 1ull << 5ull,
	kSrvCompute = 1ull << 6ull,
	kSrvTraceRays = 1ull << 7ull,

	kUavGeometry = 1ull << 8ull,
	kUavPixel = 1ull << 9ull,
	kUavCompute = 1ull << 10ull,
	kUavTraceRays = 1ull << 11ull,

	kIndex = 1ull << 12ull,
	kVertex = 1ull << 13ull,

	kIndirectCompute = 1ull << 14ll,
	kIndirectDraw = 1ull << 15ull,
	kIndirectTraceRays = 1ull << 16ull,

	kCopySource = 1ull << 17ull,
	kCopyDestination = 1ull << 18ull,

	kAccelerationStructureBuild = 1ull << 19ull, ///< Will be used as a position or index buffer in a BLAS build.
	kShaderBindingTable = 1ull << 20ull, ///< Will be used as SBT in a traceRays() command.
	kAccelerationStructureBuildScratch = 1ull << 21ull, ///< Used in buildAccelerationStructureXXX commands.

	// Derived
	kAllConstant = kConstantGeometry | kConstantPixel | kConstantCompute | kConstantTraceRays,
	kAllSrv = kSrvGeometry | kSrvPixel | kSrvCompute | kSrvTraceRays,
	kAllUav = kUavGeometry | kUavPixel | kUavCompute | kUavTraceRays,
	kAllIndirect = kIndirectCompute | kIndirectDraw | kIndirectTraceRays,
	kAllCopy = kCopySource | kCopyDestination,

	kAllGeometry = kConstantGeometry | kSrvGeometry | kUavGeometry | kIndex | kVertex,
	kAllPixel = kConstantPixel | kSrvPixel | kUavPixel,
	kAllGraphics = kAllGeometry | kAllPixel | kIndirectDraw,
	kAllCompute = kConstantCompute | kSrvCompute | kUavCompute | kIndirectCompute,
	kAllTraceRays = kConstantTraceRays | kSrvTraceRays | kUavTraceRays | kIndirectTraceRays | kShaderBindingTable,

	kAllRayTracing = kAllTraceRays | kAccelerationStructureBuild | kAccelerationStructureBuildScratch,
	kAllRead = kAllConstant | kAllSrv | kAllUav | kIndex | kVertex | kAllIndirect | kCopySource | kAccelerationStructureBuild | kShaderBindingTable,
	kAllWrite = kAllUav | kCopyDestination | kAccelerationStructureBuildScratch,

	kAllShaderResource = kAllConstant | kAllSrv | kAllUav,

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
	kGeometrySrv = 1 << 2,
	kPixelSrv = 1 << 3,
	kComputeSrv = 1 << 4,
	kTraceRaysSrv = 1 << 5,

	// Derived
	kAllGraphics = kGeometrySrv | kPixelSrv,
	kAllRead = kAttach | kGeometrySrv | kPixelSrv | kComputeSrv | kTraceRaysSrv,
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

enum class VertexAttributeSemanticBit : U8
{
	kNone,

	kPosition = 1u << 0u,
	kNormal = 1u << 1u,
	kTexCoord = 1u << 2u,
	kColor = 1u << 3u,
	kMisc0 = 1u << 4u,
	kMisc1 = 1u << 5u,
	kMisc2 = 1u << 6u,
	kMisc3 = 1u << 7u
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexAttributeSemanticBit)

/// This matches D3D.
enum class DescriptorType : U8
{
	kConstantBuffer,
	kSrvStructuredBuffer,
	kUavStructuredBuffer,
	kSrvByteAddressBuffer,
	kUavByteAddressBuffer,
	kSrvTexelBuffer,
	kUavTexelBuffer,
	kSrvTexture,
	kUavTexture,
	kAccelerationStructure,
	kSampler,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DescriptorType)

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

inline HlslResourceType descriptorTypeToHlslResourceType(DescriptorType type)
{
	HlslResourceType out = HlslResourceType::kCount;

	switch(type)
	{
	case DescriptorType::kConstantBuffer:
		out = HlslResourceType::kCbv;
		break;
	case DescriptorType::kSrvStructuredBuffer:
	case DescriptorType::kSrvByteAddressBuffer:
	case DescriptorType::kSrvTexelBuffer:
	case DescriptorType::kSrvTexture:
	case DescriptorType::kAccelerationStructure:
		out = HlslResourceType::kSrv;
		break;
	case DescriptorType::kUavStructuredBuffer:
	case DescriptorType::kUavByteAddressBuffer:
	case DescriptorType::kUavTexelBuffer:
	case DescriptorType::kUavTexture:
		out = HlslResourceType::kUav;
		break;
	case DescriptorType::kSampler:
		out = HlslResourceType::kSampler;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
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

	/// Order the bindings. THIS IS IMPORTANT because the backends expect them in a specific order.
	Bool operator<(const ShaderReflectionBinding& b) const
	{
		const HlslResourceType ahlsl = descriptorTypeToHlslResourceType(m_type);
		const HlslResourceType bhlsl = descriptorTypeToHlslResourceType(b.m_type);
		if(ahlsl != bhlsl)
		{
			return ahlsl < bhlsl;
		}

		if(m_registerBindingPoint != b.m_registerBindingPoint)
		{
			return m_registerBindingPoint < b.m_registerBindingPoint;
		}

		ANKI_ASSERT(!"Can't have 2 bindings with the same HlslResourceType and same binding point");
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
		ANKI_ASSERT(m_arraySize > 0);
		ANKI_ASSERT(!(ANKI_GR_BACKEND_DIRECT3D && (m_type == DescriptorType::kSrvStructuredBuffer || m_type == DescriptorType::kUavStructuredBuffer)
					  && m_d3dStructuredBufferStride == 0));
	}
};
ANKI_END_PACKED_STRUCT

ANKI_BEGIN_PACKED_STRUCT
class ShaderReflectionDescriptorRelated
{
public:
	/// The D3D backend expects bindings inside a space need to be ordered by HLSL type and then by register.
	Array2d<ShaderReflectionBinding, kMaxRegisterSpaces, kMaxBindingsPerRegisterSpace> m_bindings;

	Array<U8, kMaxRegisterSpaces> m_bindingCounts = {};

	U32 m_fastConstantsSize = 0;

	Bool m_hasVkBindlessDescriptorSet = false; ///< Filled by the shader compiler.
	U8 m_vkBindlessDescriptorSet = kMaxU8; ///< Filled by the VK backend.

	void validate() const
	{
		for(U32 set = 0; set < kMaxRegisterSpaces; ++set)
		{
			for(U32 ibinding = 0; ibinding < kMaxBindingsPerRegisterSpace; ++ibinding)
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
		VertexAttributeSemanticBit m_vertexAttributeMask = VertexAttributeSemanticBit::kNone;
	} m_vertex;

	class
	{
	public:
		BitSet<kMaxColorRenderTargets, U8> m_colorRenderTargetWritemask = {false};

		Bool m_discards = false;
	} m_pixel;

	ShaderReflection()
	{
		m_vertex.m_vkVertexAttributeLocations.fill(kMaxU8);
	}

	void validate() const
	{
		m_descriptor.validate();
		for([[maybe_unused]] VertexAttributeSemantic semantic : EnumIterable<VertexAttributeSemantic>())
		{
			ANKI_ASSERT(!(m_vertex.m_vertexAttributeMask & VertexAttributeSemanticBit(1 << semantic))
						|| m_vertex.m_vkVertexAttributeLocations[semantic] != kMaxU8);
		}

		const U32 attachmentCount = m_pixel.m_colorRenderTargetWritemask.getSetBitCount();
		for(U32 i = 0; i < attachmentCount; ++i)
		{
			ANKI_ASSERT(m_pixel.m_colorRenderTargetWritemask.get(i) && "Should write to all attachments");
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
