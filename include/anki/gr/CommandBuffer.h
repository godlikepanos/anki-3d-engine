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
	PtrSize m_chunkSize = 1024 * 64;
};

/// Command buffer init info.
class CommandBufferInitInfo
{
public:
	Bool m_secondLevel = false;
	FramebufferPtr m_framebuffer; ///< For second level command buffers.
	CommandBufferInitHints m_hints;
};

/// Command buffer.
class CommandBuffer : public GrObject
{
public:
	/// Construct.
	CommandBuffer(GrManager* manager);

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
	void setPolygonOffset(F32 offset, F32 units);

	/// Bind pipeline.
	void bindPipeline(PipelinePtr ppline);

	/// Begin renderpass.
	void beginRenderPass(FramebufferPtr fb);

	/// End renderpass.
	void endRenderPass();

	/// Bind resources.
	void bindResourceGroup(
		ResourceGroupPtr rc, U slot, const DynamicBufferInfo* dynInfo);
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

	void generateMipmaps(TexturePtr tex, U depth, U face);

	void copyTextureToTexture(TexturePtr src,
		const TextureSurfaceInfo& srcSurf,
		TexturePtr dest,
		const TextureSurfaceInfo& destSurf);

	void clearTexture(TexturePtr tex,
		const TextureSurfaceInfo& surf,
		const ClearValue& clearValue);
	/// @}

	/// @name Resource upload
	/// @{

	/// Upload data to a texture.
	void textureUpload(TexturePtr tex,
		const TextureSurfaceInfo& surf,
		const DynamicBufferToken& token);

	/// Write data to a buffer. It will copy the dynamic memory to the buffer
	/// starting from offset to the range indicated by the allocation of the
	/// token.
	void writeBuffer(
		BufferPtr buff, PtrSize offset, const DynamicBufferToken& token);
	/// @}

	/// @name Sync
	/// @{

	void setPipelineBarrier(PipelineStageBit src, PipelineStageBit dst);

	void setBufferMemoryBarrier(
		BufferPtr buff, ResourceAccessBit src, ResourceAccessBit dst);
	/// @}

	/// @name Other
	/// @{

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
