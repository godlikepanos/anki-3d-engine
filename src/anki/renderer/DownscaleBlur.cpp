// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Is.h>

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

	// frag shader
	ANKI_CHECK(m_r->createShaderf("shaders/DownscaleBlur.frag.glsl",
		pass.m_frag,
		"#define TEXTURE_SIZE vec2(%f, %f)\n",
		F32(inputTexSize.x()),
		F32(inputTexSize.y())));

	// prog
	m_r->createDrawQuadShaderProgram(pass.m_frag->getGrShader(), pass.m_prog);

	// RT
	pass.m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(pass.m_width,
		pass.m_height,
		IS_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
			| TextureUsageBit::SAMPLED_COMPUTE,
		SamplingFilter::LINEAR));

	// FB
	FramebufferInitInfo fbInit;
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

	cmdb->bindTexture(0, 0, m_r->getIs().getRt());

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
		cmdb->bindShaderProgram(pass.m_prog);

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
