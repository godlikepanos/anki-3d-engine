// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Math.h>

namespace anki {

/// @addtogroup graphics
/// @{

class TextureBarrierInfo
{
public:
	TextureView m_textureView;
	TextureUsageBit m_previousUsage = TextureUsageBit::kNone;
	TextureUsageBit m_nextUsage = TextureUsageBit::kNone;
};

class BufferBarrierInfo
{
public:
	BufferView m_bufferView;
	BufferUsageBit m_previousUsage = BufferUsageBit::kNone;
	BufferUsageBit m_nextUsage = BufferUsageBit::kNone;
};

class AccelerationStructureBarrierInfo
{
public:
	AccelerationStructure* m_as = nullptr;
	AccelerationStructureUsageBit m_previousUsage = AccelerationStructureUsageBit::kNone;
	AccelerationStructureUsageBit m_nextUsage = AccelerationStructureUsageBit::kNone;
};

class CopyBufferToBufferInfo
{
public:
	PtrSize m_sourceOffset = 0;
	PtrSize m_destinationOffset = 0;
	PtrSize m_range = 0;
};

class RenderTarget
{
public:
	TextureView m_textureView;

	RenderTargetLoadOperation m_loadOperation = RenderTargetLoadOperation::kClear;
	RenderTargetStoreOperation m_storeOperation = RenderTargetStoreOperation::kStore;

	RenderTargetLoadOperation m_stencilLoadOperation = RenderTargetLoadOperation::kClear;
	RenderTargetStoreOperation m_stencilStoreOperation = RenderTargetStoreOperation::kStore;

	ClearValue m_clearValue;

	TextureUsageBit m_usage = TextureUsageBit::kFramebufferWrite;

	RenderTarget() = default;

	RenderTarget(const TextureView& view)
		: m_textureView(view)
	{
	}
};

/// Command buffer initialization flags.
enum class CommandBufferFlag : U8
{
	kNone = 0,

	/// It will contain a handfull of commands.
	kSmallBatch = 1 << 0,

	/// Will contain graphics, compute and transfer work.
	kGeneralWork = 1 << 1,

	/// Will contain only compute work. It binds to async compute queues.
	kComputeWork = 1 << 2,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(CommandBufferFlag)

/// Command buffer init info.
class CommandBufferInitInfo : public GrBaseInitInfo
{
public:
	CommandBufferFlag m_flags = CommandBufferFlag::kGeneralWork;

	CommandBufferInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}
};

/// Maps to HLSL register(X#, S)
class Register
{
public:
	U32 m_bindPoint = kMaxU32;
	HlslResourceType m_resourceType = HlslResourceType::kCount;
	U8 m_space = kMaxU8;

	Register(HlslResourceType type, U32 bindingPoint, U8 space = 0)
		: m_bindPoint(bindingPoint)
		, m_resourceType(type)
		, m_space(space)
	{
		validate();
	}

	/// Construct using a couple of strings like ("t0", "space10")
	Register(const Char* reg, const Char* space = "space0")
	{
		ANKI_ASSERT(reg && space);
		m_resourceType = toResourceType(reg[0]);
		++reg;
		m_bindPoint = 0;
		do
		{
			ANKI_ASSERT(*reg >= '0' && *reg <= '9');
			m_bindPoint *= 10;
			m_bindPoint += *reg - '0';
			++reg;
		} while(*reg != '\0');
		ANKI_ASSERT(strlen(space) == 6);
		ANKI_ASSERT(space[5] >= '0' && space[5] <= '9');
		m_space = U8(space[5] - '0');
	}

	void validate() const
	{
		ANKI_ASSERT(m_bindPoint != kMaxU32);
		ANKI_ASSERT(m_resourceType < HlslResourceType::kCount);
		ANKI_ASSERT(m_space < kMaxDescriptorSets);
	}

private:
	static HlslResourceType toResourceType(Char c)
	{
		switch(c)
		{
		case 'b':
			return HlslResourceType::kCbv;
		case 'u':
			return HlslResourceType::kUav;
		case 't':
			return HlslResourceType::kSrv;
		case 's':
			return HlslResourceType::kSampler;
		default:
			ANKI_ASSERT(0);
			return HlslResourceType::kCount;
		}
	}
};

/// Break the code style to define something HLSL like
#define ANKI_REG(reg) Register(ANKI_STRINGIZE(reg))
#define ANKI_REG2(reg, space) Register(ANKI_STRINGIZE(reg), ANKI_STRINGIZE(space))

/// Command buffer.
class CommandBuffer : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kCommandBuffer;

	CommandBufferFlag getFlags() const
	{
		return m_flags;
	}

	/// Finalize the command buffer.
	void endRecording();

	/// @name State manipulation
	/// @{

	/// Bind vertex buffer.
	void bindVertexBuffer(U32 binding, const BufferView& buff, U32 stride, VertexStepRate stepRate = VertexStepRate::kVertex);

	/// Setup a vertex attribute.
	void setVertexAttribute(VertexAttributeSemantic attribute, U32 buffBinding, Format fmt, U32 relativeOffset);

	/// Bind index buffer.
	void bindIndexBuffer(const BufferView& buff, IndexType type);

	/// Enable primitive restart.
	void setPrimitiveRestart(Bool enable);

	/// Set the viewport.
	void setViewport(U32 minx, U32 miny, U32 width, U32 height);

	/// Set the scissor rect. To disable the scissor just set a rect bigger than the viewport. By default it's disabled.
	void setScissor(U32 minx, U32 miny, U32 width, U32 height);

	/// Set fill mode.
	void setFillMode(FillMode mode);

	/// Set cull mode.
	/// By default it's FaceSelectionBit::BACK.
	void setCullMode(FaceSelectionBit mode);

	/// Set depth offset and units. Set zeros to both to disable it.
	void setPolygonOffset(F32 factor, F32 units);

	/// Set stencil operations. To disable stencil test put StencilOperation::KEEP to all operations.
	void setStencilOperations(FaceSelectionBit face, StencilOperation stencilFail, StencilOperation stencilPassDepthFail,
							  StencilOperation stencilPassDepthPass);

	/// Set stencil compare operation.
	void setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp);

	/// Set the stencil compare mask.
	void setStencilCompareMask(FaceSelectionBit face, U32 mask);

	/// Set the stencil write mask.
	void setStencilWriteMask(FaceSelectionBit face, U32 mask);

	/// Set the stencil reference.
	void setStencilReference(FaceSelectionBit face, U32 ref);

	/// Enable/disable depth write. To disable depth testing set depth write to false and depth compare operation to
	/// always.
	void setDepthWrite(Bool enable);

	/// Set depth compare operation. By default it's less.
	void setDepthCompareOperation(CompareOperation op);

	/// Enable/disable alpha to coverage.
	void setAlphaToCoverage(Bool enable);

	/// Set color channel write mask.
	void setColorChannelWriteMask(U32 attachment, ColorBit mask);

	/// Set blend factors seperate.
	/// By default the values of srcRgb, dstRgb, srcA and dstA are BlendFactor::ONE, BlendFactor::ZERO,
	/// BlendFactor::ONE, BlendFactor::ZERO respectively.
	void setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA);

	/// Set blend factors.
	void setBlendFactors(U32 attachment, BlendFactor src, BlendFactor dst)
	{
		setBlendFactors(attachment, src, dst, src, dst);
	}

	/// Set the blend operation seperate.
	/// By default the values of funcRgb and funcA are BlendOperation::ADD, BlendOperation::ADD respectively.
	void setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA);

	/// Set the blend operation.
	void setBlendOperation(U32 attachment, BlendOperation func)
	{
		setBlendOperation(attachment, func, func);
	}

	/// Set the rasterizatin order. By default it's RasterizationOrder::ORDERED.
	void setRasterizationOrder(RasterizationOrder order);

	/// Set the line width. By default it's undefined.
	void setLineWidth(F32 lineWidth);

	/// Bind sampler.
	void bindSampler(Register reg, Sampler* sampler);

	/// Bind a texture.
	void bindTexture(Register reg, const TextureView& texView);

	/// Bind uniform buffer.
	void bindUniformBuffer(Register reg, const BufferView& buff);

	/// Bind storage buffer.
	void bindStorageBuffer(Register reg, const BufferView& buff);

	/// Bind texel buffer.
	void bindTexelBuffer(Register reg, const BufferView& buff, Format fmt);

	/// Bind AS.
	void bindAccelerationStructure(Register reg, AccelerationStructure* as);

	/// Set push constants.
	void setPushConstants(const void* data, U32 dataSize);

	/// Bind a program.
	void bindShaderProgram(ShaderProgram* prog);

	/// Begin a renderpass.
	/// The minx, miny, width, height control the area that the load and store operations will happen. If the scissor is bigger than the render area
	/// the results are undefined.
	void beginRenderPass(ConstWeakArray<RenderTarget> colorRts, RenderTarget* depthStencilRt, U32 minx = 0, U32 miny = 0, U32 width = kMaxU32,
						 U32 height = kMaxU32, const TextureView& vrsRt = TextureView(), U8 vrsRtTexelSizeX = 0, U8 vrsRtTexelSizeY = 0);

	/// See beginRenderPass.
	void beginRenderPass(std::initializer_list<RenderTarget> colorRts, RenderTarget* depthStencilRt = nullptr, U32 minx = 0, U32 miny = 0,
						 U32 width = kMaxU32, U32 height = kMaxU32, const TextureView& vrsRt = TextureView(), U8 vrsRtTexelSizeX = 0,
						 U8 vrsRtTexelSizeY = 0)
	{
		beginRenderPass(ConstWeakArray(colorRts.begin(), U32(colorRts.size())), depthStencilRt, minx, miny, width, height, vrsRt, vrsRtTexelSizeX,
						vrsRtTexelSizeY);
	}

	/// End renderpass.
	void endRenderPass();

	/// Set VRS rate of the following drawcalls. By default it's 1x1.
	void setVrsRate(VrsRate rate);
	/// @}

	/// @name Jobs
	/// @{
	void drawIndexed(PrimitiveTopology topology, U32 count, U32 instanceCount = 1, U32 firstIndex = 0, U32 baseVertex = 0, U32 baseInstance = 0);

	void draw(PrimitiveTopology topology, U32 count, U32 instanceCount = 1, U32 first = 0, U32 baseInstance = 0);

	void drawIndexedIndirect(PrimitiveTopology topology, const BufferView& indirectBuff, U32 drawCount = 1);

	void drawIndirect(PrimitiveTopology topology, const BufferView& indirectBuff, U32 drawCount = 1);

	void drawIndexedIndirectCount(PrimitiveTopology topology, const BufferView& argBuffer, U32 argBufferStride, const BufferView& countBuffer,
								  U32 maxDrawCount);

	void drawIndirectCount(PrimitiveTopology topology, const BufferView& argBuffer, U32 argBufferStride, const BufferView& countBuffer,
						   U32 maxDrawCount);

	void drawMeshTasks(U32 groupCountX, U32 groupCountY, U32 groupCountZ);

	void drawMeshTasksIndirect(const BufferView& argBuffer, U32 drawCount = 1);

	void dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ);

	void dispatchComputeIndirect(const BufferView& argBuffer);

	/// Trace rays.
	///
	/// The 1st thing in the sbtBuffer is the ray gen shader group handle:
	/// @code RG = RG_offset @endcode
	/// The RG_offset is equal to the stbBufferOffset.
	///
	/// Then the sbtBuffer contains the miss shader group handles and their data. The indexing is as follows:
	/// @code M = M_offset + M_stride * R_miss @endcode
	/// The M_offset is equal to stbBufferOffset + GpuDeviceCapabilities::m_sbtRecordSize.
	/// The M_stride is equal to GpuDeviceCapabilities::m_sbtRecordSize.
	/// The R_miss is defined in the traceRayEXT and it's the "ray type".
	///
	/// After the miss shaders the sbtBuffer has the hit group shader group handles and their data. The indexing is:
	/// @code HG = HG_offset + (HG_stride * (R_offset + R_stride * G_id + I_offset)) @endcode
	/// The HG_offset is equal to sbtBufferOffset + GpuDeviceCapabilities::m_sbtRecordSize * (missShaderCount + 1).
	/// The HG_stride is equal GpuDeviceCapabilities::m_sbtRecordSize * rayTypecount.
	/// The R_offset and R_stride are provided in traceRayEXT. The R_offset is the "ray type" and R_stride the number of ray types.
	/// The G_id is always 0 ATM.
	/// The I_offset is the AccelerationStructureInstance::m_hitgroupSbtRecordIndex.
	///
	/// @param[in] sbtBuffer The SBT buffer.
	/// @param sbtRecordSize The size of an SBT record
	/// @param hitGroupSbtRecordCount The number of SBT records that contain hit groups.
	/// @param rayTypecount The number of ray types hosted in the pipeline. See above on how it's been used.
	/// @param width Width.
	/// @param height Height.
	/// @param depth Depth.
	void traceRays(const BufferView& sbtBuffer, U32 sbtRecordSize, U32 hitGroupSbtRecordCount, U32 rayTypeCount, U32 width, U32 height, U32 depth);

	/// Blit from surface to surface.
	void blitTexture(const TextureView& srcView, const TextureView& destView);

	/// Clear a single texture surface. Can be used for all textures except 3D.
	void clearTexture(const TextureView& texView, const ClearValue& clearValue);

	/// Copy a buffer to a texture surface or volume.
	void copyBufferToTexture(const BufferView& buff, const TextureView& texView);

	/// Fill a buffer with some value.
	void fillBuffer(const BufferView& buff, U32 value);

	/// Write the occlusion result to buffer.
	void writeOcclusionQueriesResultToBuffer(ConstWeakArray<OcclusionQuery*> queries, const BufferView& buff);

	/// Copy buffer to buffer.
	void copyBufferToBuffer(const BufferView& src, const BufferView& dst)
	{
		ANKI_ASSERT(src.getRange() == dst.getRange());
		Array<CopyBufferToBufferInfo, 1> copies = {{{src.getOffset(), dst.getOffset(), src.getRange()}}};
		copyBufferToBuffer(&src.getBuffer(), &dst.getBuffer(), copies);
	}

	/// Copy buffer to buffer.
	void copyBufferToBuffer(Buffer* src, Buffer* dst, ConstWeakArray<CopyBufferToBufferInfo> copies);

	/// Build the acceleration structure.
	void buildAccelerationStructure(AccelerationStructure* as, const BufferView& scratchBuffer);

	/// Do upscaling by an external upscaler
	/// @param[in] upscaler the upscaler to use for upscaling
	/// @param[in] inColor Source LowRes RenderTarget.
	/// @param[out] outUpscaledColor Destination HighRes RenderTarget
	/// @param[in] motionVectors Motion Vectors
	/// @param[in] depth Depth attachment
	/// @param[in] exposure 1x1 Texture containing exposure
	/// @param[in] resetAccumulation Whether to clean or not any temporal history
	/// @param[in] jitterOffset Jittering offset that was applied during the generation of sourceTexture
	/// @param[in] motionVectorsScale Any scale factor that might need to be applied to the motionVectorsTexture (i.e UV space to Pixel space
	///                               conversion)
	void upscale(GrUpscaler* upscaler, const TextureView& inColor, const TextureView& outUpscaledColor, const TextureView& motionVectors,
				 const TextureView& depth, const TextureView& exposure, Bool resetAccumulation, const Vec2& jitterOffset,
				 const Vec2& motionVectorsScale);
	/// @}

	/// @name Sync
	/// @{
	void setPipelineBarrier(ConstWeakArray<TextureBarrierInfo> textures, ConstWeakArray<BufferBarrierInfo> buffers,
							ConstWeakArray<AccelerationStructureBarrierInfo> accelerationStructures);
	/// @}

	/// @name Other
	/// @{

	/// Begin query.
	void beginOcclusionQuery(OcclusionQuery* query);

	/// End query.
	void endOcclusionQuery(OcclusionQuery* query);

	/// Begin query.
	void beginPipelineQuery(PipelineQuery* query);

	/// End query.
	void endPipelineQuery(PipelineQuery* query);

	/// Write a timestamp.
	void writeTimestamp(TimestampQuery* query);

	Bool isEmpty() const;

	void pushDebugMarker(CString name, Vec3 color);

	void popDebugMarker();
	/// @}

protected:
	CommandBufferFlag m_flags = CommandBufferFlag::kNone;

	/// Construct.
	CommandBuffer(CString name)
		: GrObject(kClassType, name)
	{
	}

	/// Destroy.
	~CommandBuffer()
	{
	}

private:
	/// Allocate and initialize a new instance.
	[[nodiscard]] static CommandBuffer* newInstance(const CommandBufferInitInfo& init);
};
/// @}

} // end namespace anki
