// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize volumetric pass");
	}

	return err;
}

Error Volumetric::initInternal(const ConfigSet& config)
{
	m_width = m_r->getWidth() / VOLUMETRIC_FRACTION;
	m_height = m_r->getHeight() / VOLUMETRIC_FRACTION;

	ANKI_R_LOGI("Initializing volumetric pass. Size %ux%u", m_width, m_height);

	// Misc
	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_noiseTex));

	// Create RTs
	m_r->createRenderTarget(m_width,
		m_height,
		IS_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE | TextureUsageBit::CLEAR,
		SamplingFilter::LINEAR,
		1,
		m_rt);

	m_r->clearRenderTarget(m_rt, ClearValue(), TextureUsageBit::SAMPLED_FRAGMENT);

	m_r->createRenderTarget(m_width,
		m_height,
		IS_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::LINEAR,
		1,
		m_hRt);

	// Create FBs
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::LOAD;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	m_vFb = getGrManager().newInstance<Framebuffer>(fbInit);

	fbInit.m_colorAttachments[0].m_texture = m_hRt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	m_hFb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Create shaders and progs
	ANKI_CHECK(m_r->createShaderf("shaders/Volumetric.frag.glsl",
		m_frag,
		"#define FB_SIZE uvec2(%uu, %uu)\n"
		"#define CLUSTER_COUNT uvec3(%uu, %uu, %uu)\n"
		"#define NOISE_MAP_SIZE %u\n",
		m_width,
		m_height,
		m_r->getIs().getLightBin().getClusterer().getClusterCountX(),
		m_r->getIs().getLightBin().getClusterer().getClusterCountY(),
		m_r->getIs().getLightBin().getClusterer().getClusterCountZ(),
		m_noiseTex->getWidth()));

	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	// const char* shader = "shaders/DepthAwareBlurGeneric.frag.glsl";
	// const char* shader = "shaders/GaussianBlurGeneric.frag.glsl";
	const char* shader = "shaders/LumaAwareBlurGeneric.frag.glsl";
	ANKI_CHECK(m_r->createShaderf(shader,
		m_hFrag,
		"#define HPASS\n"
		"#define COL_RGB\n"
		"#define TEXTURE_SIZE vec2(%f, %f)\n"
		"#define KERNEL_SIZE 11\n",
		F32(m_width),
		F32(m_height)));

	m_r->createDrawQuadShaderProgram(m_hFrag->getGrShader(), m_hProg);

	ANKI_CHECK(m_r->createShaderf(shader,
		m_vFrag,
		"#define VPASS\n"
		"#define COL_RGB\n"
		"#define TEXTURE_SIZE vec2(%f, %f)\n"
		"#define KERNEL_SIZE 11\n",
		F32(m_width),
		F32(m_height)));

	m_r->createDrawQuadShaderProgram(m_vFrag->getGrShader(), m_vProg);

	return ErrorCode::NONE;
}

void Volumetric::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void Volumetric::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void Volumetric::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	const Frustum& frc = ctx.m_frustumComponent->getFrustum();

	//
	// Main pass
	//
	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->setBlendFactors(0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA);

	cmdb->bindTexture(0, 0, m_r->getDepthDownscale().m_qd.m_depthRt);
	cmdb->bindTexture(0, 1, m_noiseTex->getGrTexture());
	cmdb->bindTexture(0, 2, m_r->getSm().m_spotTexArray);
	cmdb->bindTexture(0, 3, m_r->getSm().m_omniTexArray);

	bindUniforms(cmdb, 0, 0, ctx.m_is.m_commonToken);
	bindUniforms(cmdb, 0, 1, ctx.m_is.m_pointLightsToken);
	bindUniforms(cmdb, 0, 2, ctx.m_is.m_spotLightsToken);

	Vec4* uniforms = allocateAndBindUniforms<Vec4*>(sizeof(Vec4) * 2, cmdb, 0, 3);
	computeLinearizeDepthOptimal(frc.getNear(), frc.getFar(), uniforms[0].x(), uniforms[0].y());

	F32 texelOffset = 1.0 / m_noiseTex->getWidth();
	uniforms[0].z() = m_r->getFrameCount() * texelOffset;
	uniforms[0].w() = m_r->getFrameCount() & (m_noiseTex->getLayerCount() - 1);

	// compute the blend factor. If the camera rotated don't blend with previous frames
	F32 dotZ = ctx.m_frustumComponent->getFrustum().getTransform().getRotation().getZAxis().xyz().dot(
		ctx.m_prevCamTransform.getZAxis().xyz());
	F32 dotY = ctx.m_frustumComponent->getFrustum().getTransform().getRotation().getYAxis().xyz().dot(
		ctx.m_prevCamTransform.getYAxis().xyz());

	const F32 TOLERANCE = cos(toRad(1.0f / 8.0f));
	F32 blendFactor;
	if(clamp(dotZ, 0.0f, 1.0f) > TOLERANCE && clamp(dotY, 0.0f, 1.0f) > TOLERANCE)
	{
		blendFactor = 1.0 / 4.0;
	}
	else
	{
		blendFactor = 1.0 / 2.0;
	}

	uniforms[1] = Vec4(m_fogParticleColor, blendFactor);

	bindStorage(cmdb, 0, 0, ctx.m_is.m_clustersToken);
	bindStorage(cmdb, 0, 1, ctx.m_is.m_lightIndicesToken);

	cmdb->bindShaderProgram(m_prog);

	cmdb->beginRenderPass(m_fb);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);

	cmdb->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
	cmdb->setTextureSurfaceBarrier(
		m_hRt, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));

	//
	// H Blur pass
	//
	Vec4* buniforms = allocateAndBindUniforms<Vec4*>(sizeof(Vec4), cmdb, 0, 0);
	computeLinearizeDepthOptimal(frc.getNear(), frc.getFar(), buniforms[0].x(), buniforms[0].y());

	cmdb->bindTexture(0, 0, m_rt);
	cmdb->bindTexture(0, 1, m_r->getDepthDownscale().m_qd.m_depthRt);

	cmdb->bindShaderProgram(m_hProg);

	cmdb->beginRenderPass(m_hFb);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();

	cmdb->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
	cmdb->setTextureSurfaceBarrier(m_hRt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));

	//
	// V Blur pass
	//
	cmdb->bindTexture(0, 0, m_hRt);
	cmdb->bindShaderProgram(m_vProg);
	cmdb->beginRenderPass(m_vFb);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

} // end namespace anki
