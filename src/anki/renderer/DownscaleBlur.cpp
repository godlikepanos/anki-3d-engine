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

	// frag shader
	ANKI_CHECK(m_r->createShaderf("shaders/DownscaleBlur.frag.glsl",
		pass.m_frag,
		"#define TEXTURE_SIZE vec2(%f, %f)\n"
		"#define TEXTURE_MIPMAP float(%u)\n",
		F32(inputTexSize.x()),
		F32(inputTexSize.y()),
		idx));

	// prog
	m_r->createDrawQuadShaderProgram(pass.m_frag->getGrShader(), pass.m_prog);

	// FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_r->getIs().getRt();
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_colorAttachments[0].m_surface.m_level = idx + 1;
	pass.m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

Error DownscaleBlur::init(const ConfigSet& cfg)
{
	ANKI_LOGI("Initializing dowscale blur");

	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_LOGE("Failed to initialize downscale blur");
	}

	return err;
}

Error DownscaleBlur::initInternal(const ConfigSet&)
{
	m_passes.create(getAllocator(), m_r->getIs().getRtMipmapCount() - 1);

	UVec2 size(m_r->getWidth(), m_r->getHeight());
	for(U i = 0; i < m_passes.getSize(); ++i)
	{
		ANKI_CHECK(initSubpass(i, size));
		size /= 2;
	}

	return ErrorCode::NONE;
}

void DownscaleBlur::run(RenderingContext& ctx)
{
	CommandBufferPtr cmdb = ctx.m_commandBuffer;

	cmdb->bindTexture(0, 0, m_r->getIs().getRt());

	UVec2 size(m_r->getWidth(), m_r->getHeight());
	for(U i = 0; i < m_passes.getSize(); ++i)
	{
		size /= 2;
		Subpass& pass = m_passes[i];

		if(i > 0)
		{
			cmdb->setTextureSurfaceBarrier(m_r->getIs().getRt(),
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
				TextureUsageBit::SAMPLED_FRAGMENT,
				TextureSurfaceInfo(i, 0, 0, 0));
		}

		cmdb->setTextureSurfaceBarrier(m_r->getIs().getRt(),
			TextureUsageBit::NONE,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureSurfaceInfo(i + 1, 0, 0, 0));

		cmdb->beginRenderPass(pass.m_fb);
		cmdb->setViewport(0, 0, size.x(), size.y());
		cmdb->bindShaderProgram(pass.m_prog);

		m_r->drawQuad(cmdb);
		cmdb->endRenderPass();
	}
}

} // end namespace anki
