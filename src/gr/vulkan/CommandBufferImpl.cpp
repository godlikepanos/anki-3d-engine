// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

#include <anki/gr/Pipeline.h>
#include <anki/gr/vulkan/PipelineImpl.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/vulkan/FramebufferImpl.h>

namespace anki
{

//==============================================================================
CommandBufferImpl::CommandBufferImpl(GrManager* manager)
	: VulkanObject(manager)
{
}

//==============================================================================
CommandBufferImpl::~CommandBufferImpl()
{
	if(m_empty)
	{
		ANKI_LOGW("Command buffer was empty");
	}

	if(!m_flushed)
	{
		ANKI_LOGW("Command buffer was not flushed");
	}

	if(m_handle)
	{
		getGrManagerImpl().deleteCommandBuffer(m_handle, m_secondLevel, m_tid);
	}

	m_pplineList.destroy(m_alloc);
	m_fbList.destroy(m_alloc);
}

//==============================================================================
Error CommandBufferImpl::init(const CommandBufferInitInfo& init)
{
	auto& pool = getGrManagerImpl().getAllocator().getMemoryPool();
	m_alloc = StackAllocator<U8>(pool.getAllocationCallback(),
		pool.getAllocationCallbackUserData(),
		init.m_hints.m_chunkSize,
		1.0,
		0,
		false);

	m_secondLevel = init.m_secondLevel;
	m_frameLast = init.m_frameLastCommandBuffer;
	m_frameFirst = init.m_frameFirstCommandBuffer;
	m_tid = Thread::getCurrentThreadId();

	m_handle = getGrManagerImpl().newCommandBuffer(m_tid, m_secondLevel);
	ANKI_ASSERT(m_handle);

	// Begin recording
	VkCommandBufferInheritanceInfo inheritance = {};
	inheritance.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

	if(init.m_secondLevel)
	{
		ANKI_ASSERT(0 && "TODO");
	}

	VkCommandBufferBeginInfo begin = {};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin.pInheritanceInfo = &inheritance;

	vkBeginCommandBuffer(m_handle, &begin);

	// If it's the frame's first command buffer then do the default fb image
	// transition
	if(init.m_frameFirstCommandBuffer)
	{
		// Default FB barrier/transition
		setImageBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			getGrManagerImpl().getDefaultSurfaceImage(
				getGrManagerImpl().getFrame() % MAX_FRAMES_IN_FLIGHT),
			VK_IMAGE_ASPECT_COLOR_BIT,
			TextureType::_2D,
			TextureSurfaceInfo(0, 0, 0));
	}

	return ErrorCode::NONE;
}

//==============================================================================
void CommandBufferImpl::commandCommon()
{
	ANKI_ASSERT(Thread::getCurrentThreadId() == m_tid
		&& "Commands must be recorder by the thread this command buffer was "
		   "created");

	ANKI_ASSERT(!m_flushed);
	ANKI_ASSERT(m_handle);
}

//==============================================================================
void CommandBufferImpl::bindPipeline(PipelinePtr ppline)
{
	commandCommon();
	vkCmdBindPipeline(m_handle,
		ppline->getImplementation().getBindPoint(),
		ppline->getImplementation().getHandle());

	m_pplineList.pushBack(m_alloc, ppline);
}

//==============================================================================
void CommandBufferImpl::beginRenderPass(FramebufferPtr fb)
{
	commandCommon();
	ANKI_ASSERT(!insideRenderPass());
	m_empty = false;

	m_firstRpassDrawcall = true;
	m_activeFb = fb;

	m_fbList.pushBack(m_alloc, fb);

#if ANKI_ASSERTIONS
	m_subpassContents = VK_SUBPASS_CONTENTS_MAX_ENUM;
#endif
}

//==============================================================================
void CommandBufferImpl::endRenderPass()
{
	commandCommon();
	ANKI_ASSERT(insideRenderPass());

	vkCmdEndRenderPass(m_handle);
	m_activeFb.reset(nullptr);
}

//==============================================================================
void CommandBufferImpl::drawcallCommon()
{
	// Preconditions
	commandCommon();
	ANKI_ASSERT(insideRenderPass());
	ANKI_ASSERT(m_subpassContents == VK_SUBPASS_CONTENTS_MAX_ENUM
		|| m_subpassContents == VK_SUBPASS_CONTENTS_INLINE);
#if ANKI_ASSERTIONS
	m_subpassContents = VK_SUBPASS_CONTENTS_INLINE;
#endif

	if(ANKI_UNLIKELY(m_firstRpassDrawcall))
	{
		m_firstRpassDrawcall = false;

		// Bind the framebuffer
		VkRenderPassBeginInfo bi = {};
		bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		FramebufferImpl& impl = m_activeFb->getImplementation();
		bi.renderPass = impl.getRenderPassHandle();
		bi.clearValueCount = impl.getAttachmentCount();
		bi.pClearValues = impl.getClearValues();

		if(!impl.isDefaultFramebuffer())
		{
			bi.framebuffer = impl.getFramebufferHandle(0);

			ANKI_ASSERT(0);
			// TODO Get the render arrea from one of the attachments
		}
		else
		{
			// Bind the default FB
			m_renderedToDefaultFb = true;

			bi.framebuffer = impl.getFramebufferHandle(
				getGrManagerImpl().getFrame() % MAX_FRAMES_IN_FLIGHT);

			bi.renderArea.extent.width =
				getGrManagerImpl().getDefaultSurfaceWidth();
			bi.renderArea.extent.height =
				getGrManagerImpl().getDefaultSurfaceHeight();
		}

		vkCmdBeginRenderPass(m_handle, &bi, VK_SUBPASS_CONTENTS_INLINE);
	}
}

//==============================================================================
void CommandBufferImpl::flush(CommandBuffer* cmdb)
{
	ANKI_ASSERT(cmdb);
	ANKI_ASSERT(!m_flushed);
	ANKI_ASSERT(!m_empty);

	if(m_frameLast)
	{
		// Default FB barrier/transition
		setImageBarrier(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			getGrManagerImpl().getDefaultSurfaceImage(
				getGrManagerImpl().getFrame() % MAX_FRAMES_IN_FLIGHT),
			VK_IMAGE_ASPECT_COLOR_BIT,
			TextureType::_2D,
			TextureSurfaceInfo(0, 0, 0));
	}

	ANKI_VK_CHECKF(vkEndCommandBuffer(m_handle));
	getGrManagerImpl().flushCommandBuffer(*this, CommandBufferPtr(cmdb));
	m_flushed = true;
}

} // end namespace anki