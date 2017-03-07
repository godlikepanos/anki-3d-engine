// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Framebuffer.h>
#include <anki/util/Functions.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// The draw indirect structure for index drawing, also the parameters of a regular drawcall
class DrawElementsIndirectInfo
{
public:
	U32 m_count = MAX_U32;
	U32 m_instanceCount = 1;
	U32 m_firstIndex = 0;
	U32 m_baseVertex = 0;
	U32 m_baseInstance = 0;

	DrawElementsIndirectInfo() = default;

	DrawElementsIndirectInfo(const DrawElementsIndirectInfo&) = default;

	DrawElementsIndirectInfo(U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
		: m_count(count)
		, m_instanceCount(instanceCount)
		, m_firstIndex(firstIndex)
		, m_baseVertex(baseVertex)
		, m_baseInstance(baseInstance)
	{
	}

	Bool operator==(const DrawElementsIndirectInfo& b) const
	{
		return m_count == b.m_count && m_instanceCount == b.m_instanceCount && m_firstIndex == b.m_firstIndex
			&& m_baseVertex == b.m_baseVertex && m_baseInstance == b.m_baseInstance;
	}

	Bool operator!=(const DrawElementsIndirectInfo& b) const
	{
		return !(operator==(b));
	}
};

/// The draw indirect structure for arrays drawing, also the parameters of a regular drawcall
class DrawArraysIndirectInfo
{
public:
	U32 m_count = MAX_U32;
	U32 m_instanceCount = 1;
	U32 m_first = 0;
	U32 m_baseInstance = 0;

	DrawArraysIndirectInfo() = default;

	DrawArraysIndirectInfo(const DrawArraysIndirectInfo&) = default;

	DrawArraysIndirectInfo(U32 count, U32 instanceCount, U32 first, U32 baseInstance)
		: m_count(count)
		, m_instanceCount(instanceCount)
		, m_first(first)
		, m_baseInstance(baseInstance)
	{
	}

	Bool operator==(const DrawArraysIndirectInfo& b) const
	{
		return m_count == b.m_count && m_instanceCount == b.m_instanceCount && m_first == b.m_first
			&& m_baseInstance == b.m_baseInstance;
	}

	Bool operator!=(const DrawArraysIndirectInfo& b) const
	{
		return !(operator==(b));
	}
};

/// Command buffer initialization hints. They are used to optimize the allocators of a command buffer.
class CommandBufferInitHints
{
	friend class CommandBufferImpl;

private:
	PtrSize m_chunkSize = 1024 * 1024;
};

/// Command buffer initialization flags.
enum class CommandBufferFlag : U8
{
	NONE = 0,

	SECOND_LEVEL = 1 << 0,

	/// It will contain a handfull of commands.
	SMALL_BATCH = 1 << 3,

	/// Will contain graphics work.
	GRAPHICS_WORK = 1 << 4,

	/// Will contain transfer commands.
	TRANSFER_WORK = 1 << 5,

	/// Will contain compute work.
	COMPUTE_WORK = 1 << 6,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(CommandBufferFlag, inline)

/// Command buffer init info.
class CommandBufferInitInfo
{
public:
	FramebufferPtr m_framebuffer; ///< For second level command buffers.
	CommandBufferInitHints m_hints;

	CommandBufferFlag m_flags = CommandBufferFlag::NONE;
};

/// Command buffer.
class CommandBuffer : public GrObject
{
	ANKI_GR_OBJECT

public:
	/// Compute initialization hints.
	CommandBufferInitHints computeInitHints() const;

	/// Finalize and submit if it's primary command buffer and just finalize if it's second level.
	void flush();

	/// Flush and wait to finish.
	void finish();

	/// @name State manipulation
	/// @{

	/// Bind vertex buffer.
	void bindVertexBuffer(U32 binding, BufferPtr buff, PtrSize offset, PtrSize stride);

	/// Setup a vertex attribute.
	void setVertexAttribute(U32 location, U32 buffBinding, const PixelFormat& fmt, PtrSize relativeOffset);

	/// Bind index buffer.
	void bindIndexBuffer(BufferPtr buff, PtrSize offset, IndexType type);

	/// Enable primitive restart.
	void setPrimitiveRestart(Bool enable);

	/// Set the viewport.
	void setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy);

	/// Enable scissor test.
	void setScissorTest(Bool enable);

	/// Set the scissor rect.
	void setScissorRect(U16 minx, U16 miny, U16 maxx, U16 maxy);

	/// Set fill mode.
	void setFillMode(FillMode mode);

	/// Set cull mode.
	void setCullMode(FaceSelectionBit mode);

	/// Set depth offset and units. Set zeros to both to disable it.
	void setPolygonOffset(F32 factor, F32 units);

	/// Set stencil operations. To disable stencil test put StencilOperation::KEEP to all operations.
	void setStencilOperations(FaceSelectionBit face,
		StencilOperation stencilFail,
		StencilOperation stencilPassDepthFail,
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

	/// Set depth compare operation.
	void setDepthCompareOperation(CompareOperation op);

	/// Enable/disable alpha to coverage.
	void setAlphaToCoverage(Bool enable);

	/// Set color channel write mask.
	void setColorChannelWriteMask(U32 attachment, ColorBit mask);

	/// Set blend factors seperate.
	void setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA);

	/// Set blend factors.
	void setBlendFactors(U32 attachment, BlendFactor src, BlendFactor dst)
	{
		setBlendFactors(attachment, src, dst, src, dst);
	}

	/// Set the blend operation seperate.
	void setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA);

	/// Set the blend operation.
	void setBlendOperation(U32 attachment, BlendOperation func)
	{
		setBlendOperation(attachment, func, func);
	}

	/// Bind texture.
	void bindTexture(U32 set, U32 binding, TexturePtr tex, DepthStencilAspectBit aspect = DepthStencilAspectBit::DEPTH);

	/// Bind texture and sample.
	void bindTextureAndSampler(U32 set,
		U32 binding,
		TexturePtr tex,
		SamplerPtr sampler,
		DepthStencilAspectBit aspect = DepthStencilAspectBit::DEPTH);

	/// Bind uniform buffer.
	/// @param set The set to bind to.
	/// @param binding The binding to bind to.
	/// @param[in,out] buff The buffer to bind.
	/// @param offset The base of the binding.
	/// @param range The bytes to bind starting from the offset. If it's MAX_PTR_SIZE then map from offset to the end
	///              of the buffer.
	void bindUniformBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range);

	/// Bind storage buffer.
	/// @param set The set to bind to.
	/// @param binding The binding to bind to.
	/// @param[in,out] buff The buffer to bind.
	/// @param offset The base of the binding.
	/// @param range The bytes to bind starting from the offset. If it's MAX_PTR_SIZE then map from offset to the end
	///              of the buffer.
	void bindStorageBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range);

	/// Bind load/store image.
	void bindImage(U32 set, U32 binding, TexturePtr img, U32 level);

	/// Bind texture buffer.
	void bindTextureBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range, PixelFormat fmt);

	/// Bind a program.
	void bindShaderProgram(ShaderProgramPtr prog);

	/// Begin renderpass.
	void beginRenderPass(FramebufferPtr fb);

	/// End renderpass.
	void endRenderPass();
	/// @}

	/// @name Jobs
	/// @{
	void drawElements(PrimitiveTopology topology,
		U32 count,
		U32 instanceCount = 1,
		U32 firstIndex = 0,
		U32 baseVertex = 0,
		U32 baseInstance = 0);

	void drawArrays(PrimitiveTopology topology, U32 count, U32 instanceCount = 1, U32 first = 0, U32 baseInstance = 0);

	void drawElementsIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, BufferPtr indirectBuff);

	void drawArraysIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, BufferPtr indirectBuff);

	void dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ);

	/// Generate mipmaps for non-3D textures.
	/// @param tex The texture to generate mips.
	/// @param face The face of a cube texture or zero.
	/// @param layer The layer of an array texture or zero.
	void generateMipmaps2d(TexturePtr tex, U face, U layer);

	/// Generate mipmaps only for 3D textures.
	/// @param tex The texture to generate mips.
	void generateMipmaps3d(TexturePtr tex);

	// TODO Rename to blit
	void copyTextureSurfaceToTextureSurface(
		TexturePtr src, const TextureSurfaceInfo& srcSurf, TexturePtr dest, const TextureSurfaceInfo& destSurf);

	void copyTextureVolumeToTextureVolume(
		TexturePtr src, const TextureVolumeInfo& srcVol, TexturePtr dest, const TextureVolumeInfo& destVol);

	/// Clear a single texture surface. Can be used for all textures except 3D.
	/// @param tex The texture to clear.
	/// @param surf The surface to clear.
	/// @param clearValue The value to clear it with.
	/// @param aspect The aspect of the depth stencil texture. Relevant only for depth stencil textures.
	void clearTextureSurface(TexturePtr tex,
		const TextureSurfaceInfo& surf,
		const ClearValue& clearValue,
		DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE);

	/// Clear a volume out of a 3D texture.
	/// @param tex The texture to clear.
	/// @param vol The volume to clear.
	/// @param clearValue The value to clear it with.
	/// @param aspect The aspect of the depth stencil texture. Relevant only for depth stencil textures.
	void clearTextureVolume(TexturePtr tex,
		const TextureVolumeInfo& vol,
		const ClearValue& clearValue,
		DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE);

	/// Copy a buffer to a texture surface.
	void copyBufferToTextureSurface(
		BufferPtr buff, PtrSize offset, PtrSize range, TexturePtr tex, const TextureSurfaceInfo& surf);

	/// Copy buffer to a texture volume.
	void copyBufferToTextureVolume(
		BufferPtr buff, PtrSize offset, PtrSize range, TexturePtr tex, const TextureVolumeInfo& vol);

	/// Fill a buffer with some value.
	/// @param[in,out] buff The buffer to fill.
	/// @param offset From where to start filling. Must be multiple of 4.
	/// @param size The bytes to fill. Must be multiple of 4 or MAX_PTR_SIZE to indicate the whole buffer.
	/// @param value The value to fill the buffer with.
	void fillBuffer(BufferPtr buff, PtrSize offset, PtrSize size, U32 value);

	/// Write the occlusion result to buffer.
	/// @param[in] query The query to get the result from.
	/// @param offset The offset inside the buffer to write the result.
	/// @param buff The buffer to update.
	void writeOcclusionQueryResultToBuffer(OcclusionQueryPtr query, PtrSize offset, BufferPtr buff);

	/// Copy buffer to buffer.
	/// @param[in] src Source buffer.
	/// @param srcOffset Offset in the src buffer.
	/// @param[out] dst Destination buffer.
	/// @param dstOffset Offset in the destination buffer.
	/// @param range Size to copy.
	void copyBufferToBuffer(BufferPtr src, PtrSize srcOffset, BufferPtr dst, PtrSize dstOffset, PtrSize range);
	/// @}

	/// @name Sync
	/// @{
	void setTextureSurfaceBarrier(
		TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureSurfaceInfo& surf);

	void setTextureVolumeBarrier(
		TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureVolumeInfo& vol);

	void setBufferBarrier(
		BufferPtr buff, BufferUsageBit prevUsage, BufferUsageBit nextUsage, PtrSize offset, PtrSize size);

	/// The command buffer will have to know the current usage of a texture. That can be known if there was a barrier
	/// but if it wasn't use this method.
	/// @param tex The texture.
	/// @param surf The texture surface.
	/// @param crntUsage The texture's current usage.
	void informTextureSurfaceCurrentUsage(TexturePtr tex, const TextureSurfaceInfo& surf, TextureUsageBit crntUsage);

	/// The command buffer will have to know the current usage of a texture. That can be known if there was a barrier
	/// but if it wasn't use this method.
	/// @param tex The texture.
	/// @param vol The texture volume.
	/// @param crntUsage The texture's current usage.
	void informTextureVolumeCurrentUsage(TexturePtr tex, const TextureVolumeInfo& vol, TextureUsageBit crntUsage);

	/// The command buffer will have to know the current usage of a texture. That can be known if there was a barrier
	/// but if it wasn't use this method.
	/// @param tex The texture.
	/// @param crntUsage The texture's current usage.
	void informTextureCurrentUsage(TexturePtr tex, TextureUsageBit crntUsage);
	/// @}

	/// @name Other
	/// @{

	/// Reset query before beginOcclusionQuery.
	void resetOcclusionQuery(OcclusionQueryPtr query);

	/// Begin query.
	void beginOcclusionQuery(OcclusionQueryPtr query);

	/// End query.
	void endOcclusionQuery(OcclusionQueryPtr query);

	/// Append a second level command buffer.
	void pushSecondLevelCommandBuffer(CommandBufferPtr cmdb);

	Bool isEmpty() const;
	/// @}

anki_internal:
	UniquePtr<CommandBufferImpl> m_impl;

	static const GrObjectType CLASS_TYPE = GrObjectType::COMMAND_BUFFER;

	/// Construct.
	CommandBuffer(GrManager* manager, U64 hash, GrObjectCache* cache);

	/// Destroy.
	~CommandBuffer();

	/// Init the command buffer.
	void init(CommandBufferInitInfo& inf);
};
/// @}

} // end namespace anki
