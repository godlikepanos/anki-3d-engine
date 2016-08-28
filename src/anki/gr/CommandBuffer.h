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

/// The draw indirect structure for index drawing, also the parameters of a
/// regular drawcall
class DrawElementsIndirectInfo
{
public:
	DrawElementsIndirectInfo()
	{
	}

	DrawElementsIndirectInfo(U32 count,
		U32 instanceCount,
		U32 firstIndex,
		U32 baseVertex,
		U32 baseInstance)
		: m_count(count)
		, m_instanceCount(instanceCount)
		, m_firstIndex(firstIndex)
		, m_baseVertex(baseVertex)
		, m_baseInstance(baseInstance)
	{
	}

	U32 m_count = MAX_U32;
	U32 m_instanceCount = 1;
	U32 m_firstIndex = 0;
	U32 m_baseVertex = 0;
	U32 m_baseInstance = 0;
};

/// The draw indirect structure for arrays drawing, also the parameters of a
/// regular drawcall
class DrawArraysIndirectInfo
{
public:
	DrawArraysIndirectInfo()
	{
	}

	DrawArraysIndirectInfo(
		U32 count, U32 instanceCount, U32 first, U32 baseInstance)
		: m_count(count)
		, m_instanceCount(instanceCount)
		, m_first(first)
		, m_baseInstance(baseInstance)
	{
	}

	U32 m_count = MAX_U32;
	U32 m_instanceCount = 1;
	U32 m_first = 0;
	U32 m_baseInstance = 0;
};

/// Command buffer initialization hints. They are used to optimize the
/// allocators of a command buffer.
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
public:
	static const GrObjectType CLASS_TYPE = GrObjectType::COMMAND_BUFFER;

	/// Construct.
	CommandBuffer(GrManager* manager, U64 hash = 0);

	/// Destroy.
	~CommandBuffer();

	/// Access the implementation.
	CommandBufferImpl& getImplementation()
	{
		return *m_impl;
	}

	/// Init the command buffer.
	void init(CommandBufferInitInfo& inf);

	/// Compute initialization hints.
	CommandBufferInitHints computeInitHints() const;

	/// Flush command buffer for deferred execution.
	void flush();

	/// Flush and wait to finish.
	void finish();

	/// @name State manipulation
	/// @{

	/// Set the viewport.
	void setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy);

	/// Set depth offset and units.
	void setPolygonOffset(F32 factor, F32 units);

	/// Bind pipeline.
	void bindPipeline(PipelinePtr ppline);

	/// Begin renderpass.
	void beginRenderPass(FramebufferPtr fb);

	/// End renderpass.
	void endRenderPass();

	/// Bind resources.
	void bindResourceGroup(
		ResourceGroupPtr rc, U slot, const TransientMemoryInfo* dynInfo);
	/// @}

	/// @name Jobs
	/// @{
	void drawElements(U32 count,
		U32 instanceCount = 1,
		U32 firstIndex = 0,
		U32 baseVertex = 0,
		U32 baseInstance = 0);

	void drawArrays(
		U32 count, U32 instanceCount = 1, U32 first = 0, U32 baseInstance = 0);

	void drawElementsConditional(OcclusionQueryPtr query,
		U32 count,
		U32 instanceCount = 1,
		U32 firstIndex = 0,
		U32 baseVertex = 0,
		U32 baseInstance = 0);

	void drawArraysConditional(OcclusionQueryPtr query,
		U32 count,
		U32 instanceCount = 1,
		U32 first = 0,
		U32 baseInstance = 0);

	void dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ);

	/// Generate mipmaps for non-3D textures.
	void generateMipmaps2d(TexturePtr tex, U face, U layer);

	/// Generate mipmaps only for 3D textures.
	void generateMipmaps3d(TexturePtr tex);

	void copyTextureSurfaceToTextureSurface(TexturePtr src,
		const TextureSurfaceInfo& srcSurf,
		TexturePtr dest,
		const TextureSurfaceInfo& destSurf);

	void copyTextureVolumeToTextureVolume(TexturePtr src,
		const TextureVolumeInfo& srcVol,
		TexturePtr dest,
		const TextureVolumeInfo& destVol);

	void clearTexture(TexturePtr tex, const ClearValue& clearValue);

	void clearTextureSurface(TexturePtr tex,
		const TextureSurfaceInfo& surf,
		const ClearValue& clearValue);

	void clearTextureVolume(TexturePtr tex,
		const TextureVolumeInfo& vol,
		const ClearValue& clearValue);

	/// Fill a buffer with some value.
	/// @param[in,out] buff The buffer to fill.
	/// @param offset From where to start filling. Must be multiple of 4.
	/// @param size The bytes to fill. Must be multiple of 4 or MAX_PTR_SIZE to
	///             indicate the whole buffer.
	/// @param value The value to fill the buffer with.
	void fillBuffer(BufferPtr buff, PtrSize offset, PtrSize size, U32 value);
	/// @}

	/// @name Resource upload
	/// @{

	/// Upload data to a texture surface. It's the base of all texture surface
	/// upload methods.
	void uploadTextureSurface(TexturePtr tex,
		const TextureSurfaceInfo& surf,
		const TransientMemoryToken& token);

	/// Same as uploadTextureSurface but it will perform the transient
	/// allocation as well. If that allocation fails expect the defaul OOM
	/// behaviour (crash).
	void uploadTextureSurfaceData(TexturePtr tex,
		const TextureSurfaceInfo& surf,
		void*& data,
		PtrSize& dataSize);

	/// Same as uploadTextureSurfaceData but it will return a nullptr in @a data
	/// if there is a OOM condition.
	void tryUploadTextureSurfaceData(TexturePtr tex,
		const TextureSurfaceInfo& surf,
		void*& data,
		PtrSize& dataSize);

	/// Same as uploadTextureSurface but it will perform the transient
	/// allocation and copy to it the @a data. If that allocation fails expect
	/// the defaul OOM behaviour (crash).
	void uploadTextureSurfaceCopyData(TexturePtr tex,
		const TextureSurfaceInfo& surf,
		void* data,
		PtrSize dataSize);

	/// Upload data to a texture volume. It's the base of all texture volume
	/// upload methods.
	void uploadTextureVolume(TexturePtr tex,
		const TextureVolumeInfo& vol,
		const TransientMemoryToken& token);

	/// Same as uploadTextureVolume but it will perform the transient
	/// allocation and copy to it the @a data. If that allocation fails expect
	/// the defaul OOM behaviour (crash).
	void uploadTextureVolumeCopyData(TexturePtr tex,
		const TextureVolumeInfo& surf,
		void* data,
		PtrSize dataSize);

	/// Upload data to a buffer.
	void uploadBuffer(
		BufferPtr buff, PtrSize offset, const TransientMemoryToken& token);
	/// @}

	/// @name Sync
	/// @{
	void setTextureSurfaceBarrier(TexturePtr tex,
		TextureUsageBit prevUsage,
		TextureUsageBit nextUsage,
		const TextureSurfaceInfo& surf);

	void setTextureVolumeBarrier(TexturePtr tex,
		TextureUsageBit prevUsage,
		TextureUsageBit nextUsage,
		const TextureVolumeInfo& vol);

	void setBufferBarrier(BufferPtr buff,
		BufferUsageBit prevUsage,
		BufferUsageBit nextUsage,
		PtrSize offset,
		PtrSize size);
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

private:
	UniquePtr<CommandBufferImpl> m_impl;
};
/// @}

} // end namespace anki

#include <anki/gr/CommandBuffer.inl.h>
