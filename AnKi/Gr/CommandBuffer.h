// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>
#include <AnKi/Gr/Framebuffer.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Math.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// Command buffer initialization flags.
enum class CommandBufferFlag : U8
{
	NONE = 0,

	SECOND_LEVEL = 1 << 0,

	/// It will contain a handfull of commands.
	SMALL_BATCH = 1 << 3,

	/// Will contain graphics, compute and transfer work.
	GENERAL_WORK = 1 << 4,

	/// Will contain only compute work. It binds to async compute queues.
	COMPUTE_WORK = 1 << 5,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(CommandBufferFlag)

/// Command buffer init info.
class CommandBufferInitInfo : public GrBaseInitInfo
{
public:
	FramebufferPtr m_framebuffer; ///< For second level command buffers.
	Array<TextureUsageBit, MAX_COLOR_ATTACHMENTS> m_colorAttachmentUsages = {};
	TextureUsageBit m_depthStencilAttachmentUsage = TextureUsageBit::NONE;

	CommandBufferFlag m_flags = CommandBufferFlag::NONE;

	CommandBufferInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}
};

/// Command buffer.
class CommandBuffer : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType CLASS_TYPE = GrObjectType::COMMAND_BUFFER;

	/// Finalize and submit if it's primary command buffer and just finalize if it's second level.
	/// @param[in]  waitFences Optionally wait for some fences.
	/// @param[out] signalFence Optionaly create fence that will be signaled when the submission is done.
	void flush(ConstWeakArray<FencePtr> waitFences = {}, FencePtr* signalFence = nullptr);

	/// @name State manipulation
	/// @{

	/// Bind vertex buffer.
	void bindVertexBuffer(U32 binding, const BufferPtr& buff, PtrSize offset, PtrSize stride,
						  VertexStepRate stepRate = VertexStepRate::VERTEX);

	/// Setup a vertex attribute.
	void setVertexAttribute(U32 location, U32 buffBinding, Format fmt, PtrSize relativeOffset);

	/// Bind index buffer.
	void bindIndexBuffer(const BufferPtr& buff, PtrSize offset, IndexType type);

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
	void setStencilOperations(FaceSelectionBit face, StencilOperation stencilFail,
							  StencilOperation stencilPassDepthFail, StencilOperation stencilPassDepthPass);

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

	/// Bind texture and sample.
	/// @param set The set to bind to.
	/// @param binding The binding to bind to.
	/// @param texView The texture view to bind.
	/// @param sampler The sampler to override the default sampler of the tex.
	/// @param arrayIdx The array index if the binding is an array.
	void bindTextureAndSampler(U32 set, U32 binding, const TextureViewPtr& texView, const SamplerPtr& sampler,
							   U32 arrayIdx = 0);

	/// Bind sampler.
	/// @param set The set to bind to.
	/// @param binding The binding to bind to.
	/// @param sampler The sampler to override the default sampler of the tex.
	/// @param arrayIdx The array index if the binding is an array.
	void bindSampler(U32 set, U32 binding, const SamplerPtr& sampler, U32 arrayIdx = 0);

	/// Bind a texture.
	/// @param set The set to bind to.
	/// @param binding The binding to bind to.
	/// @param texView The texture view to bind.
	/// @param arrayIdx The array index if the binding is an array.
	void bindTexture(U32 set, U32 binding, const TextureViewPtr& texView, U32 arrayIdx = 0);

	/// Bind uniform buffer.
	/// @param set The set to bind to.
	/// @param binding The binding to bind to.
	/// @param[in,out] buff The buffer to bind.
	/// @param offset The base of the binding.
	/// @param range The bytes to bind starting from the offset. If it's MAX_PTR_SIZE then map from offset to the end
	///              of the buffer.
	/// @param arrayIdx The array index if the binding is an array.
	void bindUniformBuffer(U32 set, U32 binding, const BufferPtr& buff, PtrSize offset, PtrSize range,
						   U32 arrayIdx = 0);

	/// Bind storage buffer.
	/// @param set The set to bind to.
	/// @param binding The binding to bind to.
	/// @param[in,out] buff The buffer to bind.
	/// @param offset The base of the binding.
	/// @param range The bytes to bind starting from the offset. If it's MAX_PTR_SIZE then map from offset to the end
	///              of the buffer.
	/// @param arrayIdx The array index if the binding is an array.
	void bindStorageBuffer(U32 set, U32 binding, const BufferPtr& buff, PtrSize offset, PtrSize range,
						   U32 arrayIdx = 0);

	/// Bind load/store image.
	/// @param set The set to bind to.
	/// @param binding The binding to bind to.
	/// @param img The view to bind.
	/// @param arrayIdx The array index if the binding is an array.
	void bindImage(U32 set, U32 binding, const TextureViewPtr& img, U32 arrayIdx = 0);

	/// Bind texture buffer.
	/// @param set The set to bind to.
	/// @param binding The binding to bind to.
	/// @param[in,out] buff The buffer to bind.
	/// @param offset The base of the binding.
	/// @param range The bytes to bind starting from the offset. If it's MAX_PTR_SIZE then map from offset to the end
	///              of the buffer.
	/// @param fmt The format of the buffer.
	/// @param arrayIdx The array index if the binding is an array.
	void bindReadOnlyTextureBuffer(U32 set, U32 binding, const BufferPtr& buff, PtrSize offset, PtrSize range,
								   Format fmt, U32 arrayIdx = 0);

	/// Bind an acceleration structure.
	/// @param set The set to bind to.
	/// @param binding The binding to bind to.
	/// @param[in,out] as The AS to bind.
	/// @param arrayIdx The array index if the binding is an array.
	void bindAccelerationStructure(U32 set, U32 binding, const AccelerationStructurePtr& as, U32 arrayIdx = 0);

	/// Bind the bindless descriptor set into a slot.
	void bindAllBindless(U32 set);

	/// Set push constants.
	void setPushConstants(const void* data, U32 dataSize);

	/// Bind a program.
	void bindShaderProgram(const ShaderProgramPtr& prog);

	/// Begin renderpass.
	/// The minx, miny, width, height control the area that the load and store operations will happen. If the scissor is
	/// bigger than the render area the results are undefined.
	void beginRenderPass(const FramebufferPtr& fb,
						 const Array<TextureUsageBit, MAX_COLOR_ATTACHMENTS>& colorAttachmentUsages,
						 TextureUsageBit depthStencilAttachmentUsage, U32 minx = 0, U32 miny = 0, U32 width = MAX_U32,
						 U32 height = MAX_U32);

	/// End renderpass.
	void endRenderPass();

	/// Set VRS rate of the following drawcalls. By default it's 1x1.
	void setVrsRate(VrsRate rate);
	/// @}

	/// @name Jobs
	/// @{
	void drawElements(PrimitiveTopology topology, U32 count, U32 instanceCount = 1, U32 firstIndex = 0,
					  U32 baseVertex = 0, U32 baseInstance = 0);

	void drawArrays(PrimitiveTopology topology, U32 count, U32 instanceCount = 1, U32 first = 0, U32 baseInstance = 0);

	void drawElementsIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, const BufferPtr& indirectBuff);

	void drawArraysIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, const BufferPtr& indirectBuff);

	void dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ);

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
	/// The R_offset and R_stride are provided in traceRayEXT. The R_offset is the "ray type" and R_stride the number of
	/// ray types.
	/// The G_id is always 0 ATM.
	/// The I_offset is the AccelerationStructureInstance::m_hitgroupSbtRecordIndex.
	///
	/// @param[in] sbtBuffer The SBT buffer.
	/// @param sbtBufferOffset Offset inside the sbtBuffer where SBT records start.
	/// @param hitGroupSbtRecordCount The number of SBT records that contain hit groups.
	/// @param sbtRecordSize The size of an SBT record
	/// @param rayTypecount The number of ray types hosted in the pipeline. See above on how it's been used.
	/// @param width Width.
	/// @param height Height.
	/// @param depth Depth.
	void traceRays(const BufferPtr& sbtBuffer, PtrSize sbtBufferOffset, U32 sbtRecordSize, U32 hitGroupSbtRecordCount,
				   U32 rayTypeCount, U32 width, U32 height, U32 depth);

	/// Generate mipmaps for non-3D textures. You have to transition all the mip levels of this face and layer to
	/// TextureUsageBit::GENERATE_MIPMAPS before calling this method.
	/// @param texView The texture view to generate mips. It should point to a subresource that contains the whole
	///                mip chain and only one face and one layer.
	void generateMipmaps2d(const TextureViewPtr& texView);

	/// Generate mipmaps only for 3D textures.
	/// @param texView The texture view to generate mips.
	void generateMipmaps3d(const TextureViewPtr& tex);

	/// Blit from surface to surface.
	/// @param srcView The source view that points to a surface.
	/// @param dstView The destination view that points to a surface.
	void blitTextureViews(const TextureViewPtr& srcView, const TextureViewPtr& destView);

	/// Clear a single texture surface. Can be used for all textures except 3D.
	/// @param[in,out] texView The texture view to clear.
	/// @param[in] clearValue The value to clear it with.
	void clearTextureView(const TextureViewPtr& texView, const ClearValue& clearValue);

	/// Copy a buffer to a texture surface or volume.
	/// @param buff The source buffer to copy from.
	/// @param offset The offset in the buffer to start reading from.
	/// @param range The size of the buffer to read.
	/// @param texView The texture view that points to a surface or volume to write to.
	void copyBufferToTextureView(const BufferPtr& buff, PtrSize offset, PtrSize range, const TextureViewPtr& texView);

	/// Fill a buffer with some value.
	/// @param[in,out] buff The buffer to fill.
	/// @param offset From where to start filling. Must be multiple of 4.
	/// @param size The bytes to fill. Must be multiple of 4 or MAX_PTR_SIZE to indicate the whole buffer.
	/// @param value The value to fill the buffer with.
	void fillBuffer(const BufferPtr& buff, PtrSize offset, PtrSize size, U32 value);

	/// Write the occlusion result to buffer.
	/// @param[in] query The query to get the result from.
	/// @param offset The offset inside the buffer to write the result.
	/// @param buff The buffer to update.
	void writeOcclusionQueryResultToBuffer(const OcclusionQueryPtr& query, PtrSize offset, const BufferPtr& buff);

	/// Copy buffer to buffer.
	/// @param[in] src Source buffer.
	/// @param srcOffset Offset in the src buffer.
	/// @param[out] dst Destination buffer.
	/// @param dstOffset Offset in the destination buffer.
	/// @param range Size to copy.
	void copyBufferToBuffer(const BufferPtr& src, PtrSize srcOffset, const BufferPtr& dst, PtrSize dstOffset,
							PtrSize range);

	/// Build the acceleration structure.
	void buildAccelerationStructure(const AccelerationStructurePtr& as);

	/// Do upscaling by an external upscaler
	/// @param[in] upscaler the upscaler to use for upscaling
	/// @param[in] inColor Source LowRes RenderTarget.
	/// @param[out] outUpscaledColor Destination HighRes RenderTarget
	/// @param[in] motionVectors Motion Vectors
	/// @param[in] depth Depth attachment
	/// @param[in] exposure 1x1 Texture containing exposure
	/// @param[in] resetAccumulation Whether to clean or not any temporal history
	/// @param[in] jitterOffset Jittering offset that was applied during the generation of sourceTexture
	/// @param[in] motionVectorsScale Any scale factor that might need to be applied to the motionVectorsTexture (i.e UV
	///                               space to Pixel space conversion)
	void upscale(const GrUpscalerPtr& upscaler, const TextureViewPtr& inColor, const TextureViewPtr& outUpscaledColor,
				 const TextureViewPtr& motionVectors, const TextureViewPtr& depth, const TextureViewPtr& exposure,
				 const Bool resetAccumulation, const Vec2& jitterOffset, const Vec2& motionVectorsScale);
	/// @}

	/// @name Sync
	/// @{
	[[deprecated]] void setTextureBarrier(const TexturePtr& tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage,
										  const TextureSubresourceInfo& subresource);

	[[deprecated]] void setTextureSurfaceBarrier(const TexturePtr& tex, TextureUsageBit prevUsage,
												 TextureUsageBit nextUsage, const TextureSurfaceInfo& surf);

	[[deprecated]] void setTextureVolumeBarrier(const TexturePtr& tex, TextureUsageBit prevUsage,
												TextureUsageBit nextUsage, const TextureVolumeInfo& vol);

	[[deprecated]] void setBufferBarrier(const BufferPtr& buff, BufferUsageBit prevUsage, BufferUsageBit nextUsage,
										 PtrSize offset, PtrSize size);

	[[deprecated]] void setAccelerationStructureBarrier(const AccelerationStructurePtr& as,
														AccelerationStructureUsageBit prevUsage,
														AccelerationStructureUsageBit nextUsage);

	void setPipelineBarrier(ConstWeakArray<TextureBarrierInfo> textures, ConstWeakArray<BufferBarrierInfo> buffers,
							ConstWeakArray<AccelerationStructureBarrierInfo> accelerationStructures);
	/// @}

	/// @name Other
	/// @{

	/// Reset query before beginOcclusionQuery.
	void resetOcclusionQuery(const OcclusionQueryPtr& query);

	/// Begin query.
	void beginOcclusionQuery(const OcclusionQueryPtr& query);

	/// End query.
	void endOcclusionQuery(const OcclusionQueryPtr& query);

	/// Reset timestamp query before writeTimestamp.
	void resetTimestampQuery(const TimestampQueryPtr& query);

	/// Write a timestamp.
	void writeTimestamp(const TimestampQueryPtr& query);

	/// Append a second level command buffer.
	void pushSecondLevelCommandBuffer(const CommandBufferPtr& cmdb);

	Bool isEmpty() const;
	/// @}

protected:
	/// Construct.
	CommandBuffer(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~CommandBuffer()
	{
	}

private:
	/// Allocate and initialize a new instance.
	[[nodiscard]] static CommandBuffer* newInstance(GrManager* manager, const CommandBufferInitInfo& init);
};
/// @}

} // end namespace anki
