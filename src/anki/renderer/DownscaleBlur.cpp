// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/TemporalAA.h>

namespace anki
{

DownscaleBlur::~DownscaleBlur()
{
	m_passes.destroy(getAllocator());
}

Error DownscaleBlur::initSubpass(U idx, const UVec2& inputTexSize)
{
	Subpass& pass = m_passes[idx];

	pass.m_width = inputTexSize.x() / 2;
	pass.m_height = inputTexSize.y() / 2;

	// RT
	pass.m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(pass.m_width,
		pass.m_height,
		LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
			| TextureUsageBit::SAMPLED_COMPUTE,
		SamplingFilter::LINEAR,
		1,
		"downblur"));

	// FB
	FramebufferInitInfo fbInit("downblur");
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = pass.m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	pass.m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

Error DownscaleBlur::init(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing dowscale blur");

	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize downscale blur");
	}

	return err;
}

Error DownscaleBlur::initInternal(const ConfigSet&)
{
	const U passCount = computeMaxMipmapCount2d(m_r->getWidth(), m_r->getHeight(), DOWNSCALE_BLUR_DOWN_TO) - 1;
	m_passes.create(getAllocator(), passCount);

	UVec2 size(m_r->getWidth(), m_r->getHeight());
	for(U i = 0; i < m_passes.getSize(); ++i)
	{
		ANKI_CHECK(initSubpass(i, size));
		size /= 2;
	}

	ANKI_CHECK(getResourceManager().loadResource("programs/DownscaleBlur.ankiprog", m_prog));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variant);
	m_grProg = variant->getProgram();

	return ErrorCode::NONE;
}

void DownscaleBlur::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_passes[0].m_rt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void DownscaleBlur::run(RenderingContext& ctx)
{
	CommandBufferPtr cmdb = ctx.m_commandBuffer;

	cmdb->bindTexture(0, 0, m_r->getTemporalAA().getRt());

	for(U i = 0; i < m_passes.getSize(); ++i)
	{
		Subpass& pass = m_passes[i];

		if(i > 0u)
		{
			cmdb->setTextureSurfaceBarrier(m_passes[i - 1].m_rt,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
				TextureUsageBit::SAMPLED_FRAGMENT,
				TextureSurfaceInfo(0, 0, 0, 0));

			cmdb->setTextureSurfaceBarrier(pass.m_rt,
				TextureUsageBit::NONE,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
				TextureSurfaceInfo(0, 0, 0, 0));

			cmdb->bindTexture(0, 0, m_passes[i - 1].m_rt);
		}

		cmdb->setViewport(0, 0, pass.m_width, pass.m_height);
		cmdb->bindShaderProgram(m_grProg);

		cmdb->beginRenderPass(pass.m_fb);
		m_r->drawQuad(cmdb);
		cmdb->endRenderPass();
	}
}

void DownscaleBlur::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_passes.getBack().m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_COMPUTE | TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

} // end namespace anki
