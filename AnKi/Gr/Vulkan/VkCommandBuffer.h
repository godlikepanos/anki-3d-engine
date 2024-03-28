// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Gr/Vulkan/VkPipeline.h>
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

	void setFence(MicroFence* fence)
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

	Bool isSecondLevel() const
	{
		return !!(m_flags & CommandBufferFlag::kSecondLevel);
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

#if ANKI_EXTRA_CHECKS
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
#if ANKI_EXTRA_CHECKS
	U32 m_commandCount = 0;
	U32 m_setPushConstantsSize = 0;
	U32 m_debugMarkersPushed = 0;
	Bool m_submitted = false;
#endif

	Framebuffer* m_activeFb = nullptr;
	Array<U32, 4> m_renderArea = {0, 0, kMaxU32, kMaxU32};
	Array<U32, 2> m_fbSize = {0, 0};
	U32 m_rpCommandCount = 0; ///< Number of drawcalls or pushed cmdbs in rp.
	Array<TextureUsageBit, kMaxColorRenderTargets> m_colorAttachmentUsages = {};
	TextureUsageBit m_depthStencilAttachmentUsage = TextureUsageBit::kNone;

	PipelineStateTracker m_state;

	Array<DSStateTracker, kMaxDescriptorSets> m_dsetState;

	ShaderProgramImpl* m_graphicsProg ANKI_DEBUG_CODE(= nullptr); ///< Last bound graphics program
	ShaderProgramImpl* m_computeProg ANKI_DEBUG_CODE(= nullptr);
	ShaderProgramImpl* m_rtProg ANKI_DEBUG_CODE(= nullptr);

	VkSubpassContents m_subpassContents = VK_SUBPASS_CONTENTS_MAX_ENUM;

	/// @name state_opts
	/// @{
	Array<U32, 4> m_viewport = {0, 0, 0, 0};
	Array<U32, 4> m_scissor = {0, 0, kMaxU32, kMaxU32};
	VkViewport m_lastViewport = {};
	Bool m_viewportDirty = true;
	Bool m_scissorDirty = true;
	VkRect2D m_lastScissor = {{-1, -1}, {kMaxU32, kMaxU32}};
	Array<U32, 2> m_stencilCompareMasks = {0x5A5A5A5A, 0x5A5A5A5A}; ///< Use a stupid number to initialize.
	Array<U32, 2> m_stencilWriteMasks = {0x5A5A5A5A, 0x5A5A5A5A};
	Array<U32, 2> m_stencilReferenceMasks = {0x5A5A5A5A, 0x5A5A5A5A};
#if ANKI_ASSERTIONS_ENABLED
	Bool m_lineWidthSet = false;
#endif
	Bool m_vrsRateDirty = true;
	VrsRate m_vrsRate = VrsRate::k1x1;

	/// Rebind the above dynamic state. Needed after pushing secondary command buffers (they dirty the state).
	void rebindDynamicState();
	/// @}

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

	Bool insideRenderPass() const
	{
		return m_activeFb != nullptr;
	}

	void beginRenderPassInternal();

	Bool secondLevel() const
	{
		return !!(m_flags & CommandBufferFlag::kSecondLevel);
	}

	void setImageBarrier(VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, VkImageLayout prevLayout, VkPipelineStageFlags dstStage,
						 VkAccessFlags dstAccess, VkImageLayout newLayout, VkImage img, const VkImageSubresourceRange& range);

	void beginRecording();

	Bool flipViewport() const
	{
		return static_cast<const FramebufferImpl&>(*m_activeFb).hasPresentableTexture();
	}

	static VkViewport computeViewport(U32* viewport, U32 fbWidth, U32 fbHeight, Bool flipvp)
	{
		const U32 minx = viewport[0];
		const U32 miny = viewport[1];
		const U32 width = min<U32>(fbWidth, viewport[2]);
		const U32 height = min<U32>(fbHeight, viewport[3]);
		ANKI_ASSERT(width > 0 && height > 0);
		ANKI_ASSERT(minx + width <= fbWidth);
		ANKI_ASSERT(miny + height <= fbHeight);

		VkViewport s = {};
		s.x = F32(minx);
		s.y = (flipvp) ? F32(fbHeight - miny) : F32(miny); // Move to the bottom;
		s.width = F32(width);
		s.height = (flipvp) ? -F32(height) : F32(height);
		s.minDepth = 0.0f;
		s.maxDepth = 1.0f;
		return s;
	}

	static VkRect2D computeScissor(U32* scissor, U32 fbWidth, U32 fbHeight, Bool flipvp)
	{
		const U32 minx = scissor[0];
		const U32 miny = scissor[1];
		const U32 width = min<U32>(fbWidth, scissor[2]);
		const U32 height = min<U32>(fbHeight, scissor[3]);
		ANKI_ASSERT(minx + width <= fbWidth);
		ANKI_ASSERT(miny + height <= fbHeight);

		VkRect2D out = {};
		out.extent.width = width;
		out.extent.height = height;
		out.offset.x = minx;
		out.offset.y = (flipvp) ? (fbHeight - (miny + height)) : miny;

		return out;
	}

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
};
/// @}

} // end namespace anki
