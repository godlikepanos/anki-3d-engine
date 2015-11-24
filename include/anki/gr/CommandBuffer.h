// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/util/Functions.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// The draw indirect structure for index drawing, also the parameters of a
/// regular drawcall
class DrawElementsIndirectInfo
{
public:
	DrawElementsIndirectInfo()
	{}

	DrawElementsIndirectInfo(U32 count, U32 instanceCount, U32 firstIndex,
		U32 baseVertex, U32 baseInstance)
		: m_count(count)
		, m_instanceCount(instanceCount)
		, m_firstIndex(firstIndex)
		, m_baseVertex(baseVertex)
		, m_baseInstance(baseInstance)
	{}

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
	{}

	DrawArraysIndirectInfo(U32 count, U32 instanceCount, U32 first,
		U32 baseInstance)
		: m_count(count)
		, m_instanceCount(instanceCount)
		, m_first(first)
		, m_baseInstance(baseInstance)
	{}

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
	enum
	{
		MAX_CHUNK_SIZE = 4 * 1024 * 1024 // 4MB
	};

	PtrSize m_chunkSize = 1024;
};

/// Command buffer.
class CommandBuffer: public GrObject
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

	/// Create command buffer.
	void create(CommandBufferInitHints hints = CommandBufferInitHints());

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

	/// Bind pipeline.
	void bindPipeline(PipelinePtr ppline);

	/// Bind framebuffer.
	void bindFramebuffer(FramebufferPtr fb);

	/// Bind resources.
	void bindResourceGroup(ResourceGroupPtr rc, U slot,
		const DynamicBufferInfo* dynInfo);
	/// @}

	/// @name Jobs
	/// @{
	void drawElements(U32 count, U32 instanceCount = 1, U32 firstIndex = 0,
		U32 baseVertex = 0, U32 baseInstance = 0);

	void drawArrays(U32 count, U32 instanceCount = 1, U32 first = 0,
		U32 baseInstance = 0);

	void drawElementsConditional(OcclusionQueryPtr query, U32 count,
		U32 instanceCount = 1, U32 firstIndex = 0, U32 baseVertex = 0,
		U32 baseInstance = 0);

	void drawArraysConditional(OcclusionQueryPtr query, U32 count,
		U32 instanceCount = 1, U32 first = 0, U32 baseInstance = 0);

	void dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ);

	void generateMipmaps(TexturePtr tex);

	void generateMipmaps(TexturePtr tex, U surface);

	void copyTextureToTexture(TexturePtr src, U srcSlice, U srcLevel,
		TexturePtr dest, U destSlice, U destLevel);
	/// @}

	/// @name Resource upload
	/// @{

	/// Upload data to a texture.
	void textureUpload(TexturePtr tex, U32 mipmap, U32 slice,
		const DynamicBufferToken& token);

	/// Write data to a buffer. It will copy the dynamic memory to the buffer
	/// starting from offset to the range indicated by the allocation of the
	/// token.
	void writeBuffer(BufferPtr buff, PtrSize offset,
		const DynamicBufferToken& token);
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
