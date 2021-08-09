// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Scale.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/TemporalAA.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki
{

Scale::~Scale()
{
}

Error Scale::init(const ConfigSet& cfg)
{
	const Bool needsScale = m_r->getPostProcessResolution() != m_r->getInternalResolution();
	if(!needsScale)
	{
		return Error::NONE;
	}

	ANKI_R_LOGI("Initializing (up|down)scale pass");

	// Program
	ANKI_CHECK(getResourceManager().loadResource("Shaders/Blit.ankiprog", m_blitProg));
	const ShaderProgramResourceVariant* variant;
	m_blitProg->getOrCreateVariant(variant);
	m_blitGrProg = variant->getProgram();

	// The RT desc
	m_rtDesc =
		m_r->create2DRenderTargetDescription(m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y(),
											 LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT, "Scaled");
	m_rtDesc.bake();

	// FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fbDescr.bake();

	return Error::NONE;
}

void Scale::populateRenderGraph(RenderingContext& ctx)
{
	const Bool needsScale = m_blitGrProg.isCreated();
	if(!needsScale)
	{
		m_runCtx.m_upscaledRt = m_r->getTemporalAA().getRt();
	}
	else
	{
		RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

		m_runCtx.m_upscaledRt = rgraph.newRenderTarget(m_rtDesc);

		GraphicsRenderPassDescription& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Scale");
		pass.newDependency(RenderPassDependency(m_r->getTemporalAA().getRt(), TextureUsageBit::SAMPLED_FRAGMENT));
		pass.newDependency(RenderPassDependency(m_runCtx.m_upscaledRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE));

		pass.setFramebufferInfo(m_fbDescr, {{m_runCtx.m_upscaledRt}}, {});

		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				Scale* const self = static_cast<Scale*>(rgraphCtx.m_userData);
				self->run(rgraphCtx);
			},
			this, 0);
	}
}

void Scale::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_blitGrProg);

	cmdb->setViewport(0, 0, m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y());

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, m_r->getTemporalAA().getRt());

	cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3, 1);
}

} // end namespace anki
