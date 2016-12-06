// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

	/// Bind transient vertex buffer.
	void bindVertexBuffer(U32 binding, const TransientMemoryToken& token, PtrSize stride);

	/// Setup a vertex attribute.
	void setVertexAttribute(U32 location, U32 buffBinding, const PixelFormat& fmt, PtrSize relativeOffset);

	/// Bind index buffer.
	void bindIndexBuffer(BufferPtr buff, PtrSize offset, IndexType type);

	/// Bind transient index buffer.
	void bindIndexBuffer(const TransientMemoryToken& token, IndexType type);

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
	void setCullMode(FaceSelectionMask mode);

	/// Set depth offset and units. Set zeros to both to disable it.
	void setPolygonOffset(F32 factor, F32 units);

	/// Set stencil operations. To disable stencil test put StencilOperation::KEEP to all operations.
	void setStencilOperations(FaceSelectionMask face,
		StencilOperation stencilFail,
		StencilOperation stencilPassDepthFail,
		StencilOperation stencilPassDepthPass);

	/// Set stencil compare function.
	void setStencilCompareFunction(FaceSelectionMask face, CompareOperation comp);

	/// Set the stencil compare mask.
	void setStencilCompareMask(FaceSelectionMask face, U32 mask);

	/// Set the stencil write mask.
	void setStencilWriteMask(FaceSelectionMask face, U32 mask);

	/// Set the stencil reference.
	void setStencilReference(FaceSelectionMask face, U32 ref);

	/// Enable/disable depth write.
	void setDepthWrite(Bool enable);

	/// Set depth compare function.
	void setDepthCompareFunction(CompareOperation op);

	/// Enable/disable alpha to coverage.
	void setAlphaToCoverage(Bool enable);

	/// Set color channel write mask.
	void setColorChannelWriteMask(U32 attachment, ColorBit mask);

	/// Set blend methods. To disable blending set src to BlendMethod::ONE and dst BlendMethod::ZERO.
	void setBlendMethods(U32 attachment, BlendMethod src, BlendMethod dst);

	/// Set the blend function.
	void setBlendFunction(U32 attachment, BlendFunction func);

	/// Bind texture.
	void bindTexture(
		U32 set, U32 binding, TexturePtr tex, DepthStencilAspectMask aspect = DepthStencilAspectMask::DEPTH);

	/// Bind texture and sample.
	void bindTextureAndSampler(U32 set,
		U32 binding,
		TexturePtr tex,
		SamplerPtr sampler,
		DepthStencilAspectMask aspect = DepthStencilAspectMask::DEPTH);

	/// Bind uniform buffer.
	void bindUniformBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset);

	/// Bind transient uniform buffer.
	void bindUniformBuffer(U32 set, U32 binding, const TransientMemoryToken& token);

	/// Bind storage buffer.
	void bindStorageBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset);

	/// Bind transient storage buffer.
	void bindStorageBuffer(U32 set, U32 binding, const TransientMemoryToken& token);

	/// Bind load/store image.
	void bindImage(U32 set, U32 binding, TexturePtr img, U32 level);

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
		DepthStencilAspectMask aspect = DepthStencilAspectMask::NONE);

	/// Clear a volume out of a 3D texture.
	/// @param tex The texture to clear.
	/// @param vol The volume to clear.
	/// @param clearValue The value to clear it with.
	/// @param aspect The aspect of the depth stencil texture. Relevant only for depth stencil textures.
	void clearTextureVolume(TexturePtr tex,
		const TextureVolumeInfo& vol,
		const ClearValue& clearValue,
		DepthStencilAspectMask aspect = DepthStencilAspectMask::NONE);

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
	/// @}

	/// @name Resource upload
	/// @{

	/// Upload data to a texture surface. It's the base of all texture surface upload methods.
	void uploadTextureSurface(TexturePtr tex, const TextureSurfaceInfo& surf, const TransientMemoryToken& token);

	/// Same as uploadTextureSurface but it will perform the transient allocation as well. If that allocation fails
	/// expect the defaul OOM behaviour (crash).
	void uploadTextureSurfaceData(TexturePtr tex, const TextureSurfaceInfo& surf, void*& data, PtrSize& dataSize);

	/// Same as uploadTextureSurfaceData but it will return a nullptr in @a data if there is a OOM condition.
	void tryUploadTextureSurfaceData(TexturePtr tex, const TextureSurfaceInfo& surf, void*& data, PtrSize& dataSize);

	/// Same as uploadTextureSurface but it will perform the transient allocation and copy to it the @a data. If that
	/// allocation fails expect the defaul OOM behaviour (crash).
	void uploadTextureSurfaceCopyData(
		TexturePtr tex, const TextureSurfaceInfo& surf, const void* data, PtrSize dataSize);

	/// Upload data to a texture volume. It's the base of all texture volume upload methods.
	void uploadTextureVolume(TexturePtr tex, const TextureVolumeInfo& vol, const TransientMemoryToken& token);

	/// Same as uploadTextureVolume but it will perform the transient allocation and copy to it the @a data. If that
	/// allocation fails expect the defaul OOM behaviour (crash).
	void uploadTextureVolumeCopyData(TexturePtr tex, const TextureVolumeInfo& surf, const void* data, PtrSize dataSize);

	/// Upload data to a buffer.
	void uploadBuffer(BufferPtr buff, PtrSize offset, const TransientMemoryToken& token);
	/// @}

	/// @name Sync
	/// @{
	void setTextureSurfaceBarrier(
		TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureSurfaceInfo& surf);

	void setTextureVolumeBarrier(
		TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureVolumeInfo& vol);

	void setBufferBarrier(
		BufferPtr buff, BufferUsageBit prevUsage, BufferUsageBit nextUsage, PtrSize offset, PtrSize size);
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

#include <anki/gr/CommandBuffer.inl.h>
