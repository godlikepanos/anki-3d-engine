// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Volumetric.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/Sm.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/LightBin.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

Volumetric::~Volumetric()
{
}

Error Volumetric::init(const ConfigSet& config)
{
	ANKI_LOGI("Initializing volumetric pass");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to initialize volumetric pass");
	}

	return err;
}

Error Volumetric::initInternal(const ConfigSet& config)
{
	U width = m_r->getWidth() / VOLUMETRIC_FRACTION;
	U height = m_r->getHeight() / VOLUMETRIC_FRACTION;

	// Create RTs
	m_r->createRenderTarget(width,
		height,
		IS_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE | TextureUsageBit::CLEAR,
		SamplingFilter::LINEAR,
		1,
		m_rt);

	m_r->clearRenderTarget(m_rt, ClearValue(), TextureUsageBit::SAMPLED_FRAGMENT);

	// Create shaders
	ANKI_CHECK(m_r->createShaderf("shaders/Volumetric.frag.glsl",
		m_frag,
		"#define RPASS_SIZE uvec2(%uu, %uu)\n"
		"#define CLUSTER_COUNT uvec3(%uu, %uu, %uu)\n",
		width,
		height,
		m_r->getIs().getLightBin().getClusterer().getClusterCountX(),
		m_r->getIs().getLightBin().getClusterer().getClusterCountY(),
		m_r->getIs().getLightBin().getClusterer().getClusterCountZ()));

	// Create prog
	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	// Create FBs
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::LOAD;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

void Volumetric::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void Volumetric::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void Volumetric::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	const Frustum& frc = ctx.m_frustumComponent->getFrustum();

	// Update uniforms
	TransientMemoryToken token;
	Vec4* uniforms = static_cast<Vec4*>(
		getGrManager().allocateFrameTransientMemory(sizeof(Vec4) * 2, BufferUsageBit::UNIFORM_ALL, token));
	computeLinearizeDepthOptimal(frc.getNear(), frc.getFar(), uniforms[0].x(), uniforms[0].y());

	uniforms[1] = Vec4(m_fogColor, m_fogFactor);

	// pass
	cmdb->setViewport(0, 0, m_r->getWidth() / VOLUMETRIC_FRACTION, m_r->getHeight() / VOLUMETRIC_FRACTION);
	cmdb->setBlendFactors(0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA);

	cmdb->bindTexture(0, 0, m_r->getDepthDownscale().m_qd.m_depthRt);
	cmdb->bindTexture(0, 1, m_r->getSm().m_spotTexArray);
	cmdb->bindTexture(0, 2, m_r->getSm().m_omniTexArray);

	cmdb->bindUniformBuffer(0, 0, ctx.m_is.m_commonToken);
	cmdb->bindUniformBuffer(0, 1, ctx.m_is.m_pointLightsToken);
	cmdb->bindUniformBuffer(0, 2, ctx.m_is.m_spotLightsToken);
	cmdb->bindUniformBuffer(0, 3, token);

	cmdb->bindStorageBuffer(0, 0, ctx.m_is.m_clustersToken);
	cmdb->bindStorageBuffer(0, 1, ctx.m_is.m_lightIndicesToken);

	cmdb->bindShaderProgram(m_prog);

	cmdb->beginRenderPass(m_fb);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
}

} // end namespace anki
