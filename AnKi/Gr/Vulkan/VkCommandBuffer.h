// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Vulkan/VkCommandBufferFactory.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Shader.h>
#include <AnKi/Gr/Vulkan/VkPipelineQuery.h>
#include <AnKi/Gr/Vulkan/VkBuffer.h>
#include <AnKi/Gr/Vulkan/VkTexture.h>
#include <AnKi/Gr/Vulkan/VkGraphicsState.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Util/List.h>

namespace anki {

#define ANKI_BATCH_COMMANDS 1

// Forward
class CommandBufferInitInfo;

/// @addtogroup vulkan
/// @{

/// Command buffer implementation.
class CommandBufferImpl final : public CommandBuffer
{
	friend class CommandBuffer;

public:
	/// Default constructor
	CommandBufferImpl(CString name)
		: CommandBuffer(name)
	{
	}

	~CommandBufferImpl();

	Error init(const CommandBufferInitInfo& init);

	void setFence(VulkanMicroFence* fence)
	{
		m_microCmdb->setFence(fence);
	}

	const MicroCommandBufferPtr& getMicroCommandBuffer()
	{
		return m_microCmdb;
	}

	VkCommandBuffer getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	Bool renderedToDefaultFramebuffer() const
	{
		return m_renderedToDefaultFb;
	}

	Bool isEmpty() const
	{
		return m_empty;
	}

	void writeTimestampInternal(TimestampQuery* query);

	// To enable using Anki's commandbuffers for external workloads
	void beginRecordingExt()
	{
		commandCommon();
	}

	void endRecording();

	Bool isFinalized() const
	{
		return m_finalized;
	}

#if ANKI_ASSERTIONS_ENABLED
	void setSubmitted()
	{
		ANKI_ASSERT(!m_submitted);
		m_submitted = true;
	}
#endif

private:
	StackMemoryPool* m_pool = nullptr;

	MicroCommandBufferPtr m_microCmdb;
	VkCommandBuffer m_handle = VK_NULL_HANDLE;
	ThreadId m_tid = ~ThreadId(0);
	Bool m_renderedToDefaultFb : 1 = false;
	Bool m_finalized : 1 = false;
	Bool m_empty : 1 = true;
	Bool m_beganRecording : 1 = false;
	Bool m_debugMarkers : 1 = false;
#if ANKI_ASSERTIONS_ENABLED
	U32 m_commandCount = 0;
	U32 m_debugMarkersPushed = 0;
	Bool m_submitted = false;
	Bool m_insideRenderpass = false;
#endif

	GraphicsStateTracker m_graphicsState;
	DescriptorState m_descriptorState;

	ShaderProgramImpl* m_graphicsProg ANKI_DEBUG_CODE(= nullptr); ///< Last bound graphics program
	ShaderProgramImpl* m_computeProg ANKI_DEBUG_CODE(= nullptr);
	ShaderProgramImpl* m_rtProg ANKI_DEBUG_CODE(= nullptr);

	/// Some common operations per command.
	ANKI_FORCE_INLINE void commandCommon()
	{
		ANKI_ASSERT(!m_finalized);
#if ANKI_EXTRA_CHECKS
		++m_commandCount;
#endif
		m_empty = false;

		if(!m_beganRecording) [[unlikely]]
		{
			beginRecording();
			m_beganRecording = true;
		}

		ANKI_ASSERT(Thread::getCurrentThreadId() == m_tid && "Commands must be recorder and flushed by the thread this command buffer was created");
		ANKI_ASSERT(m_handle);
	}

	void drawcallCommon();

	void dispatchCommon();

	void setImageBarrier(VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, VkImageLayout prevLayout, VkPipelineStageFlags dstStage,
						 VkAccessFlags dstAccess, VkImageLayout newLayout, VkImage img, const VkImageSubresourceRange& range);

	void beginRecording();

	const ShaderProgramImpl& getBoundProgram()
	{
		if(m_graphicsProg)
		{
			ANKI_ASSERT(m_computeProg == nullptr && m_rtProg == nullptr);
			return *m_graphicsProg;
		}
		else if(m_computeProg)
		{
			ANKI_ASSERT(m_graphicsProg == nullptr && m_rtProg == nullptr);
			return *m_computeProg;
		}
		else
		{
			ANKI_ASSERT(m_graphicsProg == nullptr && m_computeProg == nullptr && m_rtProg != nullptr);
			return *m_rtProg;
		}
	}

	void traceRaysInternal(const BufferView& sbtBuffer, U32 sbtRecordSize32, U32 hitGroupSbtRecordCount, U32 rayTypeCount, U32 width, U32 height,
						   U32 depth, BufferView argsBuff);
};
/// @}

} // end namespace anki
