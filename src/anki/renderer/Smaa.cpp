// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Smaa.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Is.h>
#include <SMAA/AreaTex.h>
#include <SMAA/SearchTex.h>

namespace anki
{

static const PixelFormat EDGE_PIXEL_FORMAT(ComponentFormat::R8G8, TransformFormat::UNORM);
static const PixelFormat WEIGHTS_PIXEL_FORMAT(ComponentFormat::R8G8B8A8, TransformFormat::UNORM);
static const PixelFormat STENCIL_PIXEL_FORMAT(ComponentFormat::S8, TransformFormat::UINT);

SmaaEdge::~SmaaEdge()
{
}

Error SmaaEdge::init(const ConfigSet& initializer)
{
	GrManager& gr = getGrManager();

	// Create shaders
	StringAuto pps(getAllocator());
	pps.sprintf("#define SMAA_RT_METRICS vec4(%f, %f, %f, %f)\n"
				"#define SMAA_PRESET_%s\n",
		1.0 / m_r->getWidth(),
		1.0 / m_r->getHeight(),
		F32(m_r->getWidth()),
		F32(m_r->getHeight()),
		&m_r->getSmaa().m_qualityPerset[0]);

	ANKI_CHECK(m_r->createShader("shaders/SmaaEdge.vert.glsl", m_vert, pps.toCString()));
	ANKI_CHECK(m_r->createShader("shaders/SmaaEdge.frag.glsl", m_frag, pps.toCString()));

	// Create prog
	m_prog = getGrManager().newInstance<ShaderProgram>(m_vert->getGrShader(), m_frag->getGrShader());

	// Create RT
	m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		EDGE_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::LINEAR));

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_texture = m_r->getSmaa().m_stencilTex;
	fbInit.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::CLEAR;
	m_fb = gr.newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

void SmaaEdge::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(
		m_rt, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));

	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_r->getSmaa().m_stencilTex,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void SmaaEdge::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));

	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_r->getSmaa().m_stencilTex,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void SmaaEdge::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindShaderProgram(m_prog);
	cmdb->bindTexture(0, 0, m_r->getIs().getRt());

	cmdb->setStencilOperations(
		FaceSelectionBit::FRONT, StencilOperation::KEEP, StencilOperation::KEEP, StencilOperation::REPLACE);
	cmdb->setStencilCompareMask(FaceSelectionBit::FRONT, 0xF);
	cmdb->setStencilWriteMask(FaceSelectionBit::FRONT, 0xF);
	cmdb->setStencilReference(FaceSelectionBit::FRONT, 0xF);

	cmdb->beginRenderPass(m_fb);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();

	// Restore state
	cmdb->setStencilOperations(
		FaceSelectionBit::FRONT, StencilOperation::KEEP, StencilOperation::KEEP, StencilOperation::KEEP);
}

SmaaWeights::~SmaaWeights()
{
}

Error SmaaWeights::init(const ConfigSet& initializer)
{
	GrManager& gr = getGrManager();

	// Create shaders
	StringAuto pps(getAllocator());
	pps.sprintf("#define SMAA_RT_METRICS vec4(%f, %f, %f, %f)\n"
				"#define SMAA_PRESET_%s\n",
		1.0 / m_r->getWidth(),
		1.0 / m_r->getHeight(),
		F32(m_r->getWidth()),
		F32(m_r->getHeight()),
		&m_r->getSmaa().m_qualityPerset[0]);

	ANKI_CHECK(m_r->createShader("shaders/SmaaWeights.vert.glsl", m_vert, pps.toCString()));
	ANKI_CHECK(m_r->createShader("shaders/SmaaWeights.frag.glsl", m_frag, pps.toCString()));

	// Create prog
	m_prog = getGrManager().newInstance<ShaderProgram>(m_vert->getGrShader(), m_frag->getGrShader());

	// Create RT
	m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		WEIGHTS_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::LINEAR));

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_texture = m_r->getSmaa().m_stencilTex;
	fbInit.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::LOAD;
	fbInit.m_depthStencilAttachment.m_stencilStoreOperation = AttachmentStoreOperation::DONT_CARE;
	m_fb = gr.newInstance<Framebuffer>(fbInit);

	// Create Area texture
	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::SMALL_BATCH;
	CommandBufferPtr cmdb = gr.newInstance<CommandBuffer>(cmdbinit);

	{
		TextureInitInfo texinit;
		texinit.m_width = AREATEX_WIDTH;
		texinit.m_height = AREATEX_HEIGHT;
		texinit.m_format = PixelFormat(ComponentFormat::R8G8, TransformFormat::UNORM);
		texinit.m_usage = TextureUsageBit::TRANSFER_DESTINATION | TextureUsageBit::SAMPLED_FRAGMENT;
		texinit.m_usageWhenEncountered = TextureUsageBit::SAMPLED_FRAGMENT;
		texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
		texinit.m_sampling.m_repeat = false;
		m_areaTex = gr.newInstance<Texture>(texinit);

		StagingGpuMemoryToken token;
		void* stagingMem = m_r->getStagingGpuMemoryManager().allocateFrame(
			sizeof(areaTexBytes), StagingGpuMemoryType::TRANSFER, token);
		memcpy(stagingMem, &areaTexBytes[0], sizeof(areaTexBytes));

		const TextureSurfaceInfo surf(0, 0, 0, 0);
		cmdb->setTextureSurfaceBarrier(m_areaTex, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, surf);
		cmdb->copyBufferToTextureSurface(token.m_buffer, token.m_offset, token.m_range, m_areaTex, surf);
		cmdb->setTextureSurfaceBarrier(
			m_areaTex, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::SAMPLED_FRAGMENT, surf);
	}

	// Create search texture
	{
		TextureInitInfo texinit;
		texinit.m_width = SEARCHTEX_WIDTH;
		texinit.m_height = SEARCHTEX_HEIGHT;
		texinit.m_format = PixelFormat(ComponentFormat::R8, TransformFormat::UNORM);
		texinit.m_usage = TextureUsageBit::TRANSFER_DESTINATION | TextureUsageBit::SAMPLED_FRAGMENT;
		texinit.m_usageWhenEncountered = TextureUsageBit::SAMPLED_FRAGMENT;
		texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
		texinit.m_sampling.m_repeat = false;
		m_searchTex = gr.newInstance<Texture>(texinit);

		StagingGpuMemoryToken token;
		void* stagingMem = m_r->getStagingGpuMemoryManager().allocateFrame(
			sizeof(searchTexBytes), StagingGpuMemoryType::TRANSFER, token);
		memcpy(stagingMem, &searchTexBytes[0], sizeof(searchTexBytes));

		const TextureSurfaceInfo surf(0, 0, 0, 0);
		cmdb->setTextureSurfaceBarrier(m_searchTex, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, surf);
		cmdb->copyBufferToTextureSurface(token.m_buffer, token.m_offset, token.m_range, m_searchTex, surf);
		cmdb->setTextureSurfaceBarrier(
			m_searchTex, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::SAMPLED_FRAGMENT, surf);
	}
	cmdb->flush();

	return ErrorCode::NONE;
}

void SmaaWeights::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(
		m_rt, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));
}

void SmaaWeights::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void SmaaWeights::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindTexture(0, 0, m_r->getSmaa().m_edge.m_rt);
	cmdb->bindTexture(0, 1, m_areaTex);
	cmdb->bindTexture(0, 2, m_searchTex);
	cmdb->bindShaderProgram(m_prog);

	cmdb->setStencilCompareOperation(FaceSelectionBit::FRONT, CompareOperation::EQUAL);
	cmdb->setStencilCompareMask(FaceSelectionBit::FRONT, 0xF);
	cmdb->setStencilWriteMask(FaceSelectionBit::FRONT, 0x0);
	cmdb->setStencilReference(FaceSelectionBit::FRONT, 0xF);

	cmdb->beginRenderPass(m_fb);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();

	// Restore state
	cmdb->setStencilCompareOperation(FaceSelectionBit::FRONT, CompareOperation::ALWAYS);
}

Error Smaa::init(const ConfigSet& cfg)
{
	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize SMAA");
	}

	return err;
}

Error Smaa::initInternal(const ConfigSet& cfg)
{
	m_qualityPerset = "ULTRA";

	ANKI_R_LOGI("Initializing SMAA in %s perset", &m_qualityPerset[0]);

	TextureInitInfo texinit;
	texinit.m_format = STENCIL_PIXEL_FORMAT;
	texinit.m_width = m_r->getWidth();
	texinit.m_height = m_r->getHeight();
	texinit.m_usage = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	m_stencilTex = m_r->createAndClearRenderTarget(texinit);

	ANKI_CHECK(m_edge.init(cfg));
	ANKI_CHECK(m_weights.init(cfg));
	return ErrorCode::NONE;
}

} // end namespace anki
