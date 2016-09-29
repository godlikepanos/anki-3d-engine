// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_vert, "shaders/SmaaEdge.vert.glsl", pps.toCString(), "r_"));
	ANKI_CHECK(getResourceManager().loadResourceToCache(m_frag, "shaders/SmaaEdge.frag.glsl", pps.toCString(), "r_"));

	// Create ppline
	PipelineInitInfo ppinit;

	ppinit.m_inputAssembler.m_topology = PrimitiveTopology::TRIANGLE_STRIP;

	ppinit.m_depthStencil.m_depthWriteEnabled = false;
	ppinit.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;
	ppinit.m_depthStencil.m_stencilFront.m_stencilPassDepthPassOperation = StencilOperation::REPLACE;

	ppinit.m_color.m_attachmentCount = 1;
	ppinit.m_color.m_attachments[0].m_format = EDGE_PIXEL_FORMAT;

	ppinit.m_shaders[ShaderType::VERTEX] = m_vert->getGrShader();
	ppinit.m_shaders[ShaderType::FRAGMENT] = m_frag->getGrShader();

	m_ppline = gr.newInstance<Pipeline>(ppinit);

	// Create RT
	m_r->createRenderTarget(m_r->getWidth(),
		m_r->getHeight(),
		EDGE_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::LINEAR,
		1,
		m_rt);

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_colorAttachments[0].m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
	fbInit.m_depthStencilAttachment.m_texture = m_r->getSmaa().m_stencilTex;
	fbInit.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
	m_fb = gr.newInstance<Framebuffer>(fbInit);

	// Create RC group
	ResourceGroupInitInfo rcinit;
	rcinit.m_textures[0].m_texture = m_r->getIs().getRt();
	m_rcgroup = gr.newInstance<ResourceGroup>(rcinit);

	return ErrorCode::NONE;
}

void SmaaEdge::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(
		m_rt, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));
}

void SmaaEdge::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));

	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_r->getSmaa().m_stencilTex,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void SmaaEdge::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindResourceGroup(m_rcgroup, 0, nullptr);
	cmdb->bindPipeline(m_ppline);
	cmdb->setStencilCompareMask(FaceSelectionMask::FRONT, 0xF);
	cmdb->setStencilWriteMask(FaceSelectionMask::FRONT, 0xF);
	cmdb->setStencilReference(FaceSelectionMask::FRONT, 0xF);

	cmdb->beginRenderPass(m_fb);
	cmdb->drawArrays(3);
	cmdb->endRenderPass();
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

	ANKI_CHECK(
		getResourceManager().loadResourceToCache(m_vert, "shaders/SmaaWeights.vert.glsl", pps.toCString(), "r_"));
	ANKI_CHECK(
		getResourceManager().loadResourceToCache(m_frag, "shaders/SmaaWeights.frag.glsl", pps.toCString(), "r_"));

	// Create ppline
	PipelineInitInfo ppinit;

	ppinit.m_inputAssembler.m_topology = PrimitiveTopology::TRIANGLE_STRIP;

	ppinit.m_depthStencil.m_depthWriteEnabled = false;
	ppinit.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;

	ppinit.m_color.m_attachmentCount = 1;
	ppinit.m_color.m_attachments[0].m_format = WEIGHTS_PIXEL_FORMAT;
	ppinit.m_depthStencil.m_stencilFront.m_compareFunction = CompareOperation::EQUAL;

	ppinit.m_shaders[ShaderType::VERTEX] = m_vert->getGrShader();
	ppinit.m_shaders[ShaderType::FRAGMENT] = m_frag->getGrShader();

	m_ppline = gr.newInstance<Pipeline>(ppinit);

	// Create RT
	m_r->createRenderTarget(m_r->getWidth(),
		m_r->getHeight(),
		WEIGHTS_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::LINEAR,
		1,
		m_rt);

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_colorAttachments[0].m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
	fbInit.m_depthStencilAttachment.m_texture = m_r->getSmaa().m_stencilTex;
	fbInit.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::LOAD;
	fbInit.m_depthStencilAttachment.m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;
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
		texinit.m_usage = TextureUsageBit::UPLOAD | TextureUsageBit::SAMPLED_FRAGMENT;
		texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
		texinit.m_sampling.m_repeat = false;
		m_areaTex = gr.newInstance<Texture>(texinit);

		const TextureSurfaceInfo surf(0, 0, 0, 0);
		cmdb->setTextureSurfaceBarrier(m_areaTex, TextureUsageBit::NONE, TextureUsageBit::UPLOAD, surf);
		cmdb->uploadTextureSurfaceCopyData(m_areaTex, surf, &areaTexBytes[0], sizeof(areaTexBytes));
		cmdb->setTextureSurfaceBarrier(m_areaTex, TextureUsageBit::UPLOAD, TextureUsageBit::SAMPLED_FRAGMENT, surf);
	}

	// Create search texture
	{
		TextureInitInfo texinit;
		texinit.m_width = SEARCHTEX_WIDTH;
		texinit.m_height = SEARCHTEX_HEIGHT;
		texinit.m_format = PixelFormat(ComponentFormat::R8, TransformFormat::UNORM);
		texinit.m_usage = TextureUsageBit::UPLOAD | TextureUsageBit::SAMPLED_FRAGMENT;
		texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
		texinit.m_sampling.m_repeat = false;
		m_searchTex = gr.newInstance<Texture>(texinit);

		const TextureSurfaceInfo surf(0, 0, 0, 0);
		cmdb->setTextureSurfaceBarrier(m_searchTex, TextureUsageBit::NONE, TextureUsageBit::UPLOAD, surf);
		cmdb->uploadTextureSurfaceCopyData(m_searchTex, surf, &searchTexBytes[0], sizeof(searchTexBytes));
		cmdb->setTextureSurfaceBarrier(m_searchTex, TextureUsageBit::UPLOAD, TextureUsageBit::SAMPLED_FRAGMENT, surf);
	}
	cmdb->flush();

	// Create RC group
	ResourceGroupInitInfo rcinit;
	rcinit.m_textures[0].m_texture = m_r->getSmaa().m_edge.m_rt;
	rcinit.m_textures[1].m_texture = m_areaTex;
	rcinit.m_textures[2].m_texture = m_searchTex;
	m_rcgroup = gr.newInstance<ResourceGroup>(rcinit);

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
	cmdb->bindResourceGroup(m_rcgroup, 0, nullptr);
	cmdb->bindPipeline(m_ppline);
	cmdb->setStencilCompareMask(FaceSelectionMask::FRONT, 0xF);
	cmdb->setStencilWriteMask(FaceSelectionMask::FRONT, 0x0);
	cmdb->setStencilReference(FaceSelectionMask::FRONT, 0xF);

	cmdb->beginRenderPass(m_fb);
	cmdb->drawArrays(3);
	cmdb->endRenderPass();
}

Error Smaa::init(const ConfigSet& cfg)
{
	TextureInitInfo texinit;
	texinit.m_format = PixelFormat(ComponentFormat::S8, TransformFormat::UINT);
	texinit.m_width = m_r->getWidth();
	texinit.m_height = m_r->getHeight();
	texinit.m_usage = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	m_stencilTex = getGrManager().newInstance<Texture>(texinit);

	m_qualityPerset = "ULTRA";
	ANKI_CHECK(m_edge.init(cfg));
	ANKI_CHECK(m_weights.init(cfg));
	return ErrorCode::NONE;
}

} // end namespace anki
