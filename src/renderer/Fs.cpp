// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Fs.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Sm.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/Camera.h>

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
		Is::RT_PIXEL_FORMAT,
		1,
		SamplingFilter::NEAREST,
		1,
		m_rt);

	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_texture = m_r->getMs().getDepthRt();
	fbInit.m_depthStencilAttachment.m_loadOperation =
		AttachmentLoadOperation::LOAD;
	fbInit.m_depthStencilAttachment.m_mipmap = 1;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Init the global resources
	{
		ResourceGroupInitInfo init;
		init.m_textures[0].m_texture = m_r->getMs().getDepthRt();

		if(m_r->getSmEnabled())
		{
			init.m_textures[1].m_texture = m_r->getSm().getSpotTextureArray();
			init.m_textures[2].m_texture = m_r->getSm().getOmniTextureArray();
		}

		init.m_storageBuffers[0].m_dynamic = true;
		init.m_storageBuffers[1].m_dynamic = true;
		init.m_storageBuffers[2].m_dynamic = true;
		init.m_storageBuffers[3].m_dynamic = true;
		init.m_storageBuffers[4].m_dynamic = true;

		m_globalResources = getGrManager().newInstance<ResourceGroup>(init);
	}

	getGrManager().finish();
	return ErrorCode::NONE;
}

//==============================================================================
void Fs::prepareBuildCommandBuffers(RenderingContext& ctx)
{
	DynamicBufferInfo& dyn = ctx.m_fs.m_set1DynInfo;
	dyn.m_storageBuffers[0] = m_r->getIs().getCommonVarsToken();
	dyn.m_storageBuffers[1] = m_r->getIs().getPointLightsToken();
	dyn.m_storageBuffers[2] = m_r->getIs().getSpotLightsToken();
	dyn.m_storageBuffers[3] = m_r->getIs().getClustersToken();
	dyn.m_storageBuffers[4] = m_r->getIs().getLightIndicesToken();
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
	ThreadPool::Task::choseStartEnd(
		threadId, threadCount, problemSize, start, end);

	if(start == end)
	{
		// Early exit
		return ErrorCode::NONE;
	}

	// Create the command buffer and set some state
	CommandBufferPtr cmdb = m_r->getGrManager().newInstance<CommandBuffer>();
	ctx.m_fs.m_commandBuffers[threadId] = cmdb;

	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->setPolygonOffset(0.0, 0.0);
	cmdb->bindResourceGroup(m_globalResources, 1, &ctx.m_fs.m_set1DynInfo);

	// Start drawing
	Error err = m_r->getSceneDrawer().drawRange(Pass::MS_FS,
		*ctx.m_frustumComponent,
		cmdb,
		vis.getBegin(VisibilityGroupType::RENDERABLES_FS) + start,
		vis.getBegin(VisibilityGroupType::RENDERABLES_FS) + end);

	return err;
}

//==============================================================================
void Fs::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->bindFramebuffer(m_fb);
	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->setPolygonOffset(0.0, 0.0);

	for(U i = 0; i < m_r->getThreadPool().getThreadsCount(); ++i)
	{
		if(ctx.m_fs.m_commandBuffers[i].isCreated())
		{
			cmdb->pushSecondLevelCommandBuffer(ctx.m_fs.m_commandBuffers[i]);
		}
	}
}

} // end namespace anki
