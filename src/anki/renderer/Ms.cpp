// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ms.h>
#include <anki/renderer/Renderer.h>
#include <anki/util/Logger.h>
#include <anki/util/ThreadPool.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/misc/ConfigSet.h>
#include <anki/core/Trace.h>
#include <anki/scene/LightComponent.h>
#include <anki/scene/MoveComponent.h>

namespace anki
{

Ms::~Ms()
{
}

Error Ms::init(const ConfigSet& initializer)
{
	ANKI_R_LOGI("Initializing g-buffer pass");

	Error err = initInternal(initializer);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize g-buffer pass");
	}

	return err;
}

Error Ms::initInternal(const ConfigSet& initializer)
{
	m_depthRt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		MS_DEPTH_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE
			| TextureUsageBit::GENERATE_MIPMAPS,
		SamplingFilter::NEAREST,
		1,
		"gbuffdepth"));

	m_rt0 = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		MS_COLOR_ATTACHMENT_PIXEL_FORMATS[0],
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::NEAREST,
		1,
		"gbuffrt0"));

	m_rt1 = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		MS_COLOR_ATTACHMENT_PIXEL_FORMATS[1],
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::NEAREST,
		1,
		"gbuffrt1"));

	m_rt2 = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		MS_COLOR_ATTACHMENT_PIXEL_FORMATS[2],
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
			| TextureUsageBit::GENERATE_MIPMAPS,
		SamplingFilter::NEAREST,
		1,
		"gbuffrt2"));

	AttachmentLoadOperation loadop = AttachmentLoadOperation::DONT_CARE;
#if ANKI_EXTRA_CHECKS
	loadop = AttachmentLoadOperation::CLEAR;
#endif

	FramebufferInitInfo fbInit("gbuffer");
	fbInit.m_colorAttachmentCount = MS_COLOR_ATTACHMENT_COUNT;
	fbInit.m_colorAttachments[0].m_texture = m_rt0;
	fbInit.m_colorAttachments[0].m_loadOperation = loadop;
	fbInit.m_colorAttachments[0].m_clearValue.m_colorf = {{1.0, 0.0, 0.0, 0.0}};
	fbInit.m_colorAttachments[1].m_texture = m_rt1;
	fbInit.m_colorAttachments[1].m_loadOperation = loadop;
	fbInit.m_colorAttachments[1].m_clearValue.m_colorf = {{0.0, 1.0, 0.0, 0.0}};
	fbInit.m_colorAttachments[2].m_texture = m_rt2;
	fbInit.m_colorAttachments[2].m_loadOperation = loadop;
	fbInit.m_colorAttachments[2].m_clearValue.m_colorf = {{0.0, 0.0, 1.0, 0.0}};
	fbInit.m_depthStencilAttachment.m_texture = m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH_STENCIL;

	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	ANKI_CHECK(m_r->getResourceManager().loadResource(
		"shaders/MsStencilBufferIsPermutations.vert.glsl", m_isPermutationsVert));
	ANKI_CHECK(m_r->getResourceManager().loadResource(
		"shaders/MsStencilBufferIsPermutations.frag.glsl", m_isPermutationsFrag));
	m_isPermutationsProg = m_r->getGrManager().newInstance<ShaderProgram>(
		m_isPermutationsVert->getGrShader(), m_isPermutationsFrag->getGrShader());

	return ErrorCode::NONE;
}

Error Ms::buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount) const
{
	ANKI_TRACE_START_EVENT(RENDER_MS);

	// Get some stuff
	const VisibilityTestResults& vis = *ctx.m_visResults;

	U problemSize = vis.getCount(VisibilityGroupType::RENDERABLES_MS);
	PtrSize start, end;
	ThreadPoolTask::choseStartEnd(threadId, threadCount, problemSize, start, end);

	if(start != end)
	{
		// Create the command buffer
		CommandBufferInitInfo cinf;
		cinf.m_flags = CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::GRAPHICS_WORK;
		if(end - start < COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS)
		{
			cinf.m_flags |= CommandBufferFlag::SMALL_BATCH;
		}
		cinf.m_framebuffer = m_fb;
		CommandBufferPtr cmdb = m_r->getGrManager().newInstance<CommandBuffer>(cinf);
		ctx.m_ms.m_commandBuffers[threadId] = cmdb;

		// Inform on RTs
		TextureSurfaceInfo surf(0, 0, 0, 0);
		cmdb->informTextureSurfaceCurrentUsage(m_rt0, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE);
		cmdb->informTextureSurfaceCurrentUsage(m_rt1, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE);
		cmdb->informTextureSurfaceCurrentUsage(m_rt2, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE);
		cmdb->informTextureSurfaceCurrentUsage(m_depthRt, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);

		// Set some state, leave the rest to default
		cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

		// Start drawing
		ANKI_CHECK(m_r->getSceneDrawer().drawRange(Pass::MS_FS,
			ctx.m_viewMat,
			ctx.m_viewProjMatJitter,
			cmdb,
			vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + start,
			vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + end));

		// If last fill stencil buffer
		if(end == problemSize)
		{
			fillStencilBufferWithIsPermutations(ctx, cmdb);
		}
	}

	ANKI_TRACE_STOP_EVENT(RENDER_MS);
	return ErrorCode::NONE;
}

void Ms::run(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_MS);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->beginRenderPass(m_fb);

	// Set some state anyway because other stages may depend on it
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	for(U i = 0; i < m_r->getThreadPool().getThreadsCount(); ++i)
	{
		if(ctx.m_ms.m_commandBuffers[i].isCreated())
		{
			cmdb->pushSecondLevelCommandBuffer(ctx.m_ms.m_commandBuffers[i]);
		}
	}

	cmdb->endRenderPass();

	ANKI_TRACE_STOP_EVENT(RENDER_MS);
}

void Ms::setPreRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_MS);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	TextureSurfaceInfo surf(0, 0, 0, 0);

	cmdb->setTextureSurfaceBarrier(m_rt0, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf);
	cmdb->setTextureSurfaceBarrier(m_rt1, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf);
	cmdb->setTextureSurfaceBarrier(m_rt2, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf);
	cmdb->setTextureSurfaceBarrier(
		m_depthRt, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, surf);

	ANKI_TRACE_STOP_EVENT(RENDER_MS);
}

void Ms::setPostRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_MS);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	TextureSurfaceInfo surf(0, 0, 0, 0);

	cmdb->setTextureSurfaceBarrier(
		m_rt0, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	cmdb->setTextureSurfaceBarrier(
		m_rt1, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	cmdb->setTextureSurfaceBarrier(
		m_rt2, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	cmdb->setTextureSurfaceBarrier(
		m_depthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	ANKI_TRACE_STOP_EVENT(RENDER_MS);
}

void Ms::fillStencilBufferWithIsPermutations(RenderingContext& ctx, CommandBufferPtr& cmdb) const
{
	// Set state
	for(U i = 0; i < MS_COLOR_ATTACHMENT_COUNT; ++i)
	{
		cmdb->setColorChannelWriteMask(i, ColorBit::NONE);
	}

	cmdb->setDepthWrite(false);
	cmdb->setDepthCompareOperation(CompareOperation::GREATER);
	cmdb->setCullMode(FaceSelectionBit::FRONT);

	cmdb->setStencilReference(FaceSelectionBit::FRONT_AND_BACK, 0xF);
	cmdb->setStencilCompareMask(FaceSelectionBit::FRONT_AND_BACK, 0xF);
	cmdb->setStencilOperations(
		FaceSelectionBit::FRONT_AND_BACK, StencilOperation::KEEP, StencilOperation::KEEP, StencilOperation::REPLACE);
	cmdb->setStencilCompareOperation(FaceSelectionBit::FRONT_AND_BACK, CompareOperation::ALWAYS);

	cmdb->bindShaderProgram(m_isPermutationsProg);

	cmdb->setVertexAttribute(0, 0, PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT), 0);
	cmdb->setVertexAttribute(1, 1, PixelFormat(ComponentFormat::R32G32B32A32, TransformFormat::FLOAT), 0);
	cmdb->setVertexAttribute(2, 2, PixelFormat(ComponentFormat::R32G32B32A32, TransformFormat::FLOAT), 0);
	cmdb->setVertexAttribute(3, 3, PixelFormat(ComponentFormat::R32G32B32A32, TransformFormat::FLOAT), 0);
	cmdb->setVertexAttribute(4, 4, PixelFormat(ComponentFormat::R32G32B32A32, TransformFormat::FLOAT), 0);

	// Do point lights
	const Mat4& viewProjMat = ctx.m_viewProjMatJitter;
	U plightCount = ctx.m_visResults->getCount(VisibilityGroupType::LIGHTS_POINT);
	StagingGpuMemoryToken plightMvpsToken;
	if(plightCount)
	{
		// Allocate MVP vert buffer
		Vec4* mvps = static_cast<Vec4*>(m_r->getStagingGpuMemoryManager().allocateFrame(
			plightCount * sizeof(Mat4), StagingGpuMemoryType::VERTEX, plightMvpsToken));

		// Fill the MVPs
		const VisibleNode* it = ctx.m_visResults->getBegin(VisibilityGroupType::LIGHTS_POINT);
		const VisibleNode* end = ctx.m_visResults->getEnd(VisibilityGroupType::LIGHTS_POINT);
		while(it != end)
		{
			const LightComponent& lightc = it->m_node->getComponent<LightComponent>();
			const MoveComponent& movec = it->m_node->getComponent<MoveComponent>();

			Mat4 modelMat(movec.getWorldTransform().getOrigin().xyz1(),
				movec.getWorldTransform().getRotation().getRotationPart(),
				lightc.getRadius());

			Mat4 mvp = viewProjMat * modelMat;

			mvps[0] = mvp.getColumn(0);
			mvps[1] = mvp.getColumn(1);
			mvps[2] = mvp.getColumn(2);
			mvps[3] = mvp.getColumn(3);

			mvps += 4;
			++it;
		}

		// Set state and draw
		cmdb->setStencilWriteMask(FaceSelectionBit::FRONT_AND_BACK, 1 << U(IsShaderPermutationOption::POINT_LIGHTS));

		cmdb->bindVertexBuffer(0, m_r->getLightVolumePrimitives().m_pointLightPositions, 0, sizeof(F32) * 3);
		cmdb->bindVertexBuffer(1,
			plightMvpsToken.m_buffer,
			plightMvpsToken.m_offset + 0 * sizeof(Vec4),
			sizeof(Mat4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(2,
			plightMvpsToken.m_buffer,
			plightMvpsToken.m_offset + 1 * sizeof(Vec4),
			sizeof(Mat4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(3,
			plightMvpsToken.m_buffer,
			plightMvpsToken.m_offset + 2 * sizeof(Vec4),
			sizeof(Mat4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(4,
			plightMvpsToken.m_buffer,
			plightMvpsToken.m_offset + 3 * sizeof(Vec4),
			sizeof(Mat4),
			VertexStepRate::INSTANCE);
		cmdb->bindIndexBuffer(m_r->getLightVolumePrimitives().m_pointLightIndices, 0, IndexType::U16);

		cmdb->drawElements(
			PrimitiveTopology::TRIANGLES, m_r->getLightVolumePrimitives().m_pointLightIndexCount, plightCount);
	}

	// Do spot lights
	U slightCount = ctx.m_visResults->getCount(VisibilityGroupType::LIGHTS_SPOT);
	StagingGpuMemoryToken slightMvpsToken;
	if(slightCount)
	{
		// Allocate MVP vert buffer
		Vec4* mvps = static_cast<Vec4*>(m_r->getStagingGpuMemoryManager().allocateFrame(
			slightCount * sizeof(Mat4), StagingGpuMemoryType::VERTEX, slightMvpsToken));

		// Fill the MVPs
		const VisibleNode* it = ctx.m_visResults->getBegin(VisibilityGroupType::LIGHTS_SPOT);
		const VisibleNode* end = ctx.m_visResults->getEnd(VisibilityGroupType::LIGHTS_SPOT);
		while(it != end)
		{
			const LightComponent& lightc = it->m_node->getComponent<LightComponent>();
			const MoveComponent& movec = it->m_node->getComponent<MoveComponent>();

			Mat4 modelMat(movec.getWorldTransform().getOrigin().xyz1(),
				movec.getWorldTransform().getRotation().getRotationPart(),
				1.0);

			// Calc the scale of the cone
			Mat4 scaleMat(Mat4::getIdentity());
			scaleMat(0, 0) = tan(lightc.getOuterAngle() / 2.0f) * lightc.getDistance();
			scaleMat(1, 1) = scaleMat(0, 0);
			scaleMat(2, 2) = lightc.getDistance();

			modelMat = modelMat * scaleMat;

			Mat4 mvp = viewProjMat * modelMat;

			mvps[0] = mvp.getColumn(0);
			mvps[1] = mvp.getColumn(1);
			mvps[2] = mvp.getColumn(2);
			mvps[3] = mvp.getColumn(3);

			mvps += 4;
			++it;
		}

		// Set state and draw
		cmdb->setStencilWriteMask(FaceSelectionBit::FRONT_AND_BACK, 1 << U(IsShaderPermutationOption::SPOT_LIGHTS));

		cmdb->bindVertexBuffer(0, m_r->getLightVolumePrimitives().m_spotLightPositions, 0, sizeof(F32) * 3);
		cmdb->bindVertexBuffer(1,
			slightMvpsToken.m_buffer,
			slightMvpsToken.m_offset + 0 * sizeof(Vec4),
			sizeof(Mat4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(2,
			slightMvpsToken.m_buffer,
			slightMvpsToken.m_offset + 1 * sizeof(Vec4),
			sizeof(Mat4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(3,
			slightMvpsToken.m_buffer,
			slightMvpsToken.m_offset + 2 * sizeof(Vec4),
			sizeof(Mat4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(4,
			slightMvpsToken.m_buffer,
			slightMvpsToken.m_offset + 3 * sizeof(Vec4),
			sizeof(Mat4),
			VertexStepRate::INSTANCE);
		cmdb->bindIndexBuffer(m_r->getLightVolumePrimitives().m_spotLightIndices, 0, IndexType::U16);

		cmdb->drawElements(
			PrimitiveTopology::TRIANGLES, m_r->getLightVolumePrimitives().m_spotLightIndexCount, slightCount);
	}

	// Draw again to remove the areas where the stencil buffer has over exposure
	cmdb->setDepthCompareOperation(CompareOperation::LESS);
	cmdb->setCullMode(FaceSelectionBit::BACK);

#if 0
	cmdb->setStencilOperations(
		FaceSelectionBit::FRONT_AND_BACK, StencilOperation::KEEP, StencilOperation::ZERO, StencilOperation::KEEP);
	
	if(plightCount)
	{
		cmdb->setStencilWriteMask(FaceSelectionBit::FRONT_AND_BACK, 1 << U(IsShaderPermutationOption::POINT_LIGHTS));
	
		cmdb->bindVertexBuffer(0, m_r->getLightVolumePrimitives().m_pointLightPositions, 0, sizeof(F32) * 3);
		cmdb->bindVertexBuffer(1,
			plightMvpsToken.m_buffer,
			plightMvpsToken.m_offset + 0 * sizeof(Vec4),
			sizeof(Vec4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(2,
			plightMvpsToken.m_buffer,
			plightMvpsToken.m_offset + 1 * sizeof(Vec4),
			sizeof(Vec4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(3,
			plightMvpsToken.m_buffer,
			plightMvpsToken.m_offset + 2 * sizeof(Vec4),
			sizeof(Vec4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(4,
			plightMvpsToken.m_buffer, 
			plightMvpsToken.m_offset + 3 * sizeof(Vec4),
			sizeof(Vec4),
			VertexStepRate::INSTANCE);
		cmdb->bindIndexBuffer(m_r->getLightVolumePrimitives().m_pointLightIndices, 0, IndexType::U16);

		cmdb->drawElements(
			PrimitiveTopology::TRIANGLES, m_r->getLightVolumePrimitives().m_pointLightIndexCount, plightCount);
	}
	
	if(slightCount)
	{
		cmdb->setStencilWriteMask(FaceSelectionBit::FRONT_AND_BACK, 1 << U(IsShaderPermutationOption::SPOT_LIGHTS));

		cmdb->bindVertexBuffer(0, m_r->getLightVolumePrimitives().m_spotLightPositions, 0, sizeof(F32) * 3);
		cmdb->bindVertexBuffer(1,
			slightMvpsToken.m_buffer,
			slightMvpsToken.m_offset + 0 * sizeof(Vec4),
			sizeof(Vec4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(2,
			slightMvpsToken.m_buffer,
			slightMvpsToken.m_offset + 1 * sizeof(Vec4),
			sizeof(Vec4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(3,
			slightMvpsToken.m_buffer,
			slightMvpsToken.m_offset + 2 * sizeof(Vec4),
			sizeof(Vec4),
			VertexStepRate::INSTANCE);
		cmdb->bindVertexBuffer(4,
			slightMvpsToken.m_buffer,
			slightMvpsToken.m_offset + 3 * sizeof(Vec4),
			sizeof(Vec4),
			VertexStepRate::INSTANCE);
		cmdb->bindIndexBuffer(m_r->getLightVolumePrimitives().m_spotLightIndices, 0, IndexType::U16);

		cmdb->drawElements(
			PrimitiveTopology::TRIANGLES, m_r->getLightVolumePrimitives().m_spotLightIndexCount, slightCount);
	}
#endif

	// Restore state
	cmdb->setStencilWriteMask(FaceSelectionBit::FRONT_AND_BACK, 0);
}

} // end namespace anki
