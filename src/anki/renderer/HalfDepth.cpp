// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/HalfDepth.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>

namespace anki
{

HalfDepth::~HalfDepth()
{
}

Error HalfDepth::init(const ConfigSet&)
{
	GrManager& gr = getGrManager();
	U width = m_r->getWidth() / 2;
	U height = m_r->getHeight() / 2;

	// Create shader
	ANKI_CHECK(getResourceManager().loadResource("shaders/HalfDepth.frag.glsl", m_frag));

	// Create pipeline
	PipelineInitInfo pinit;

	pinit.m_inputAssembler.m_topology = PrimitiveTopology::TRIANGLE_STRIP;

	pinit.m_depthStencil.m_depthWriteEnabled = true;
	pinit.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;
	pinit.m_depthStencil.m_format = MS_DEPTH_ATTACHMENT_PIXEL_FORMAT;

	pinit.m_shaders[ShaderType::VERTEX] = m_r->getDrawQuadVertexShader();
	pinit.m_shaders[ShaderType::FRAGMENT] = m_frag->getGrShader();
	m_ppline = gr.newInstance<Pipeline>(pinit);

	// Create RT
	m_r->createRenderTarget(width,
		height,
		MS_DEPTH_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		SamplingFilter::LINEAR,
		1,
		m_depthRt);

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_depthStencilAttachment.m_texture = m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_depthStencilAttachment.m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectMask::DEPTH;

	m_fb = gr.newInstance<Framebuffer>(fbInit);

	// Create RC group
	ResourceGroupInitInfo rcinit;
	rcinit.m_textures[0].m_texture = m_r->getMs().getDepthRt();
	rcinit.m_textures[0].m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;

	m_rcgroup = gr.newInstance<ResourceGroup>(rcinit);

	return ErrorCode::NONE;
}

void HalfDepth::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fb);
	cmdb->setViewport(0, 0, m_r->getWidth() / 2, m_r->getHeight() / 2);
	cmdb->bindPipeline(m_ppline);
	cmdb->bindResourceGroup(m_rcgroup, 0, nullptr);

	m_r->drawQuad(cmdb);

	cmdb->endRenderPass();
}

} // end namespace anki
