// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Velocity.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

Velocity::~Velocity()
{
}

Error Velocity::init(const ConfigSet& cfg)
{
	ANKI_LOGI("Initializing velocity pass");

	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_LOGE("Failed to initialize velocity pass");
	}

	return err;
}

Error Velocity::initInternal(const ConfigSet& cfg)
{
	U width = m_r->getWidth() / 2, height = m_r->getHeight() / 2;
	ANKI_CHECK(m_r->createShaderf("shaders/Velocity.frag.glsl", m_frag, "#define FB_SIZE uvec2(%uu, %uu)", 
		width, height));

	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	m_r->createRenderTarget(width,
		height,
		PixelFormat(ComponentFormat::R8G8, TransformFormat::UNORM),
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		SamplingFilter::LINEAR,
		1,
		m_rt);

	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;

	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

void Velocity::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	struct Uniforms
	{
		Mat4 m_prevInvViewProjMat;
		Mat4 m_crntViewProjMat;
	};

	TransientMemoryToken token;
	Uniforms* unis = static_cast<Uniforms*>(getGrManager().allocateFrameTransientMemory(sizeof(Uniforms), 
		BufferUsageBit::UNIFORM_ALL, token));

	unis->m_prevInvViewProjMat = ctx.m_prevFrameViewProjMatrix.getInverse();
	unis->m_crntViewProjMat = ctx.m_frustumComponent->getViewProjectionMatrix();

	cmdb->bindTexture(0, 0, m_r->getDepthDownscale().m_hd.m_depthRt);
	cmdb->bindUniformBuffer(0, 0, token);
	cmdb->setViewport(0, 0, m_r->getWidth() / 2, m_r->getHeight() / 2);
	cmdb->bindShaderProgram(m_prog);
	cmdb->beginRenderPass(m_fb);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

} // end namespace anki
