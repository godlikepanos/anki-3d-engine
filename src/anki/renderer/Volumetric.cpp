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

Error VolumetricMain::init(const ConfigSet& config)
{
	// Misc
	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_noiseTex));

	for(U i = 0; i < 2; ++i)
	{
		// RT
		TextureInitInfo rtInit = m_r->create2DRenderTargetInitInfo(m_vol->m_width,
			m_vol->m_height,
			IS_COLOR_ATTACHMENT_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			SamplingFilter::LINEAR,
			1,
			"volmain");
		rtInit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		m_rt[i] = m_r->createAndClearRenderTarget(rtInit);

		// FB
		FramebufferInitInfo fbInit("volmain");
		fbInit.m_colorAttachmentCount = 1;
		fbInit.m_colorAttachments[0].m_texture = m_rt[i];
		fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		m_fb[i] = getGrManager().newInstance<Framebuffer>(fbInit);
	}

	// Shaders
	ANKI_CHECK(getResourceManager().loadResource("programs/VolumetricFog.ankiprog", m_prog));

	ShaderProgramResourceMutationInitList<1> mutators(m_prog);
	mutators.add("ENABLE_SHADOWS", 1);

	ShaderProgramResourceConstantValueInitList<3> consts(m_prog);
	consts.add("FB_SIZE", UVec2(m_vol->m_width, m_vol->m_height))
		.add("CLUSTER_COUNT",
			UVec3(m_r->getIs().getLightBin().getClusterer().getClusterCountX(),
				m_r->getIs().getLightBin().getClusterer().getClusterCountY(),
				m_r->getIs().getLightBin().getClusterer().getClusterCountZ()))
		.add("NOISE_MAP_SIZE", U32(m_noiseTex->getWidth()));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutators.get(), consts.get(), variant);
	m_grProg = variant->getProgram();

	return ErrorCode::NONE;
}

TexturePtr VolumetricMain::getRt() const
{
	return m_rt[m_r->getFrameCount() & 1];
}

void VolumetricMain::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt[m_r->getFrameCount() & 1],
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void VolumetricMain::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	//
	// Main pass
	//
	cmdb->setViewport(0, 0, m_vol->m_width, m_vol->m_height);

	cmdb->bindTexture(0, 0, m_r->getDepthDownscale().m_qd.m_depthRt);
	cmdb->bindTexture(0, 1, m_noiseTex->getGrTexture());
	TexturePtr& history = m_rt[(m_r->getFrameCount() + 1) & 1];
	cmdb->informTextureCurrentUsage(history, TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->bindTexture(0, 2, history);
	cmdb->bindTexture(0, 3, m_r->getSm().m_spotTexArray);
	cmdb->bindTexture(0, 4, m_r->getSm().m_omniTexArray);

	bindUniforms(cmdb, 0, 0, ctx.m_is.m_commonToken);
	bindUniforms(cmdb, 0, 1, ctx.m_is.m_pointLightsToken);
	bindUniforms(cmdb, 0, 2, ctx.m_is.m_spotLightsToken);

	struct Unis
	{
		Vec4 m_linearizeNoiseTexOffsetLayer;
		Vec4 m_fogParticleColorPad1;
		Mat4 m_prevViewProjMatMulInvViewProjMat;
	};

	Unis* uniforms = allocateAndBindUniforms<Unis*>(sizeof(Unis), cmdb, 0, 3);
	computeLinearizeDepthOptimal(ctx.m_near,
		ctx.m_far,
		uniforms->m_linearizeNoiseTexOffsetLayer.x(),
		uniforms->m_linearizeNoiseTexOffsetLayer.y());
	F32 texelOffset = 1.0 / m_noiseTex->getWidth();
	uniforms->m_linearizeNoiseTexOffsetLayer.z() = m_r->getFrameCount() * texelOffset;
	uniforms->m_linearizeNoiseTexOffsetLayer.w() = m_r->getFrameCount() & (m_noiseTex->getLayerCount() - 1);
	uniforms->m_fogParticleColorPad1 = Vec4(m_fogParticleColor, 0.0);
	uniforms->m_prevViewProjMatMulInvViewProjMat = ctx.m_prevViewProjMat * ctx.m_viewProjMat.getInverse();

	bindStorage(cmdb, 0, 0, ctx.m_is.m_clustersToken);
	bindStorage(cmdb, 0, 1, ctx.m_is.m_lightIndicesToken);

	cmdb->bindShaderProgram(m_grProg);

	cmdb->beginRenderPass(m_fb[m_r->getFrameCount() & 1]);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

void VolumetricMain::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt[m_r->getFrameCount() & 1],
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

Error VolumetricHBlur::init(const ConfigSet& config)
{
	// Create RTs
	m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_vol->m_width,
		m_vol->m_height,
		IS_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::LINEAR,
		1,
		"volblur"));

	// Create FBs
	FramebufferInitInfo fbInit("volblur");
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Progs
	ANKI_CHECK(m_r->getResourceManager().loadResource("programs/LumaAwareBlur.ankiprog", m_prog));

	ShaderProgramResourceMutationInitList<3> mutators(m_prog);
	mutators.add("HORIZONTAL", 1).add("KERNEL_SIZE", 11).add("COLOR_COMPONENTS", 3);
	ShaderProgramResourceConstantValueInitList<1> consts(m_prog);
	consts.add("TEXTURE_SIZE", UVec2(m_vol->m_width, m_vol->m_height));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutators.get(), consts.get(), variant);
	m_grProg = variant->getProgram();

	return ErrorCode::NONE;
}

void VolumetricHBlur::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(
		m_rt, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));
}

void VolumetricHBlur::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->bindTexture(0, 0, m_vol->m_main.m_rt[m_r->getFrameCount() & 1]);
	cmdb->bindShaderProgram(m_grProg);
	cmdb->setViewport(0, 0, m_vol->m_width, m_vol->m_height);

	cmdb->beginRenderPass(m_fb);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

void VolumetricHBlur::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

Error VolumetricVBlur::init(const ConfigSet& config)
{
	// Create FBs
	for(U i = 0; i < 2; ++i)
	{
		FramebufferInitInfo fbInit;
		fbInit.m_colorAttachmentCount = 1;
		fbInit.m_colorAttachments[0].m_texture = m_vol->m_main.m_rt[i];
		fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		m_fb[i] = getGrManager().newInstance<Framebuffer>(fbInit);
	}

	// Progs
	ANKI_CHECK(m_r->getResourceManager().loadResource("programs/LumaAwareBlur.ankiprog", m_prog));

	ShaderProgramResourceMutationInitList<3> mutators(m_prog);
	mutators.add("HORIZONTAL", 0).add("KERNEL_SIZE", 11).add("COLOR_COMPONENTS", 3);
	ShaderProgramResourceConstantValueInitList<1> consts(m_prog);
	consts.add("TEXTURE_SIZE", UVec2(m_vol->m_width, m_vol->m_height));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutators.get(), consts.get(), variant);
	m_grProg = variant->getProgram();

	return ErrorCode::NONE;
}

void VolumetricVBlur::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_vol->m_main.m_rt[m_r->getFrameCount() & 1],
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void VolumetricVBlur::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->bindTexture(0, 0, m_vol->m_hblur.m_rt);
	cmdb->bindShaderProgram(m_grProg);
	cmdb->setViewport(0, 0, m_vol->m_width, m_vol->m_height);

	cmdb->beginRenderPass(m_fb[m_r->getFrameCount() & 1]);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

void VolumetricVBlur::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_vol->m_main.m_rt[m_r->getFrameCount() & 1],
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

Error Volumetric::init(const ConfigSet& config)
{
	m_width = m_r->getWidth() / VOLUMETRIC_FRACTION;
	m_height = m_r->getHeight() / VOLUMETRIC_FRACTION;

	ANKI_R_LOGI("Initializing volumetric pass. Size %ux%u", m_width, m_height);

	Error err = m_main.init(config);

	if(!err)
	{
		err = m_hblur.init(config);
	}

	if(!err)
	{
		err = m_vblur.init(config);
	}

	if(err)
	{
		ANKI_R_LOGE("Failed to initialize volumetric pass");
	}

	return err;
}

} // end namespace anki
