// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Fs.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Sm.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

//==============================================================================
Fs::~Fs()
{
}

//==============================================================================
Error Fs::init(const ConfigSet&)
{
	m_width = m_r->getWidth() / FS_FRACTION;
	m_height = m_r->getHeight() / FS_FRACTION;

	// Create RT
	m_r->createRenderTarget(m_width,
		m_height,
		IS_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT
			| TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		SamplingFilter::NEAREST,
		1,
		m_rt);

	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::CLEAR;
	fbInit.m_colorAttachments[0].m_usageInsideRenderPass =
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	fbInit.m_depthStencilAttachment.m_texture = m_r->getMs().getDepthRt();
	fbInit.m_depthStencilAttachment.m_loadOperation =
		AttachmentLoadOperation::LOAD;
	fbInit.m_depthStencilAttachment.m_usageInsideRenderPass =
		TextureUsageBit::SAMPLED_FRAGMENT
		| TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;
	fbInit.m_depthStencilAttachment.m_surface.m_level = 1;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Init the global resources
	{
		ResourceGroupInitInfo init;
		init.m_textures[0].m_texture = m_r->getMs().getDepthRt();
		init.m_textures[0].m_usage = TextureUsageBit::SAMPLED_FRAGMENT
			| TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;
		init.m_textures[1].m_texture = m_r->getSm().getSpotTextureArray();
		init.m_textures[2].m_texture = m_r->getSm().getOmniTextureArray();

		init.m_uniformBuffers[0].m_uploadedMemory = true;
		init.m_uniformBuffers[0].m_usage =
			BufferUsageBit::UNIFORM_FRAGMENT | BufferUsageBit::UNIFORM_VERTEX;
		init.m_uniformBuffers[1].m_uploadedMemory = true;
		init.m_uniformBuffers[1].m_usage =
			BufferUsageBit::UNIFORM_FRAGMENT | BufferUsageBit::UNIFORM_VERTEX;
		init.m_uniformBuffers[2].m_uploadedMemory = true;
		init.m_uniformBuffers[2].m_usage =
			BufferUsageBit::UNIFORM_FRAGMENT | BufferUsageBit::UNIFORM_VERTEX;

		init.m_storageBuffers[0].m_uploadedMemory = true;
		init.m_storageBuffers[0].m_usage =
			BufferUsageBit::STORAGE_FRAGMENT | BufferUsageBit::STORAGE_VERTEX;
		init.m_storageBuffers[1].m_uploadedMemory = true;
		init.m_storageBuffers[1].m_usage =
			BufferUsageBit::STORAGE_FRAGMENT | BufferUsageBit::STORAGE_VERTEX;

		m_globalResources = getGrManager().newInstance<ResourceGroup>(init);
	}

	getGrManager().finish();
	return ErrorCode::NONE;
}

//==============================================================================
Error Fs::buildCommandBuffers(
	RenderingContext& ctx, U threadId, U threadCount) const
{
	// Get some stuff
	VisibilityTestResults& vis =
		ctx.m_frustumComponent->getVisibilityTestResults();

	U problemSize = vis.getCount(VisibilityGroupType::RENDERABLES_FS);
	PtrSize start, end;
	ThreadPoolTask::choseStartEnd(
		threadId, threadCount, problemSize, start, end);

	if(start == end)
	{
		// Early exit
		return ErrorCode::NONE;
	}

	// Create the command buffer and set some state
	CommandBufferInitInfo cinf;
	cinf.m_flags = CommandBufferFlag::SECOND_LEVEL;
	cinf.m_framebuffer = m_fb;
	CommandBufferPtr cmdb =
		m_r->getGrManager().newInstance<CommandBuffer>(cinf);
	ctx.m_fs.m_commandBuffers[threadId] = cmdb;

	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->setPolygonOffset(0.0, 0.0);
	cmdb->bindResourceGroup(m_globalResources, 1, &ctx.m_is.m_dynBufferInfo);

	// Start drawing
	Error err = m_r->getSceneDrawer().drawRange(Pass::MS_FS,
		*ctx.m_frustumComponent,
		cmdb,
		vis.getBegin(VisibilityGroupType::RENDERABLES_FS) + start,
		vis.getBegin(VisibilityGroupType::RENDERABLES_FS) + end);

	return err;
}

//==============================================================================
void Fs::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureBarrier(m_rt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

//==============================================================================
void Fs::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

//==============================================================================
void Fs::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->beginRenderPass(m_fb);
	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->setPolygonOffset(0.0, 0.0);

	for(U i = 0; i < m_r->getThreadPool().getThreadsCount(); ++i)
	{
		if(ctx.m_fs.m_commandBuffers[i].isCreated())
		{
			cmdb->pushSecondLevelCommandBuffer(ctx.m_fs.m_commandBuffers[i]);
		}
	}

	cmdb->endRenderPass();
}

} // end namespace anki
