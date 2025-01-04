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

	TextureUsageBit m_usage = TextureUsageBit::kRtvDsvWrite;

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

	CommandBufferInitInfo(CommandBufferFlag flags, CString name = {})
		: GrBaseInitInfo(name)
		, m_flags(flags)
	{
	}
};

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

	/// Set the line width. By default it's undefined.
	void setLineWidth(F32 lineWidth);

	/// Bind constant buffer.
	void bindConstantBuffer(U32 reg, U32 space, const BufferView& buff);

	/// Bind sampler.
	void bindSampler(U32 reg, U32 space, Sampler* sampler);

	/// Bind a texture.
	void bindSrv(U32 reg, U32 space, const TextureView& texView);

	/// Bind a buffer.
	void bindSrv(U32 reg, U32 space, const BufferView& buffer, Format fmt = Format::kNone);

	/// Bind AS.
	void bindSrv(U32 reg, U32 space, AccelerationStructure* as);

	/// Bind a texture.
	void bindUav(U32 reg, U32 space, const TextureView& texView);

	/// Bind a buffer.
	void bindUav(U32 reg, U32 space, const BufferView& buffer, Format fmt = Format::kNone);

	/// Set push constants (Vulkan) or root constants (D3D).
	void setFastConstants(const void* data, U32 dataSize);

	/// Bind a program.
	void bindShaderProgram(ShaderProgram* prog);

	/// Begin a renderpass.
	void beginRenderPass(ConstWeakArray<RenderTarget> colorRts, RenderTarget* depthStencilRt, const TextureView& vrsRt = TextureView(),
						 U8 vrsRtTexelSizeX = 0, U8 vrsRtTexelSizeY = 0);

	/// See beginRenderPass.
	void beginRenderPass(std::initializer_list<RenderTarget> colorRts, RenderTarget* depthStencilRt = nullptr,
						 const TextureView& vrsRt = TextureView(), U8 vrsRtTexelSizeX = 0, U8 vrsRtTexelSizeY = 0)
	{
		beginRenderPass(ConstWeakArray(colorRts.begin(), U32(colorRts.size())), depthStencilRt, vrsRt, vrsRtTexelSizeX, vrsRtTexelSizeY);
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

	void dispatchGraph(const BufferView& scratchBuffer, const void* records, U32 recordCount, U32 recordStride);

	/// Trace rays.
	///
	/// The 1st thing in the sbtBuffer is the ray gen shader group handle:
	/// @code RG = RG_offset @endcode
	/// The RG_offset is equal to the sbtBuffer.getOffset().
	///
	/// Then the sbtBuffer contains the miss shader group handles and their data. The indexing is as follows:
	/// @code M = M_offset + M_stride * R_miss @endcode
	/// The M_offset is equal to sbtBuffer.getOffset() + GpuDeviceCapabilities::m_sbtRecordSize.
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

	/// Same as traceRays but indirect.
	void traceRaysIndirect(const BufferView& sbtBuffer, U32 sbtRecordSize, U32 hitGroupSbtRecordCount, U32 rayTypeCount, BufferView argsBuffer);

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
