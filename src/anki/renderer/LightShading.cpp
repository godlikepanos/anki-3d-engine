// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/LightShading.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/Indirect.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/ForwardShading.h>
#include <anki/renderer/VolumetricFog.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/Ssao.h>
#include <anki/renderer/Ssr.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/HighRezTimer.h>

namespace anki
{

LightShading::LightShading(Renderer* r)
	: RendererObject(r)
{
}

LightShading::~LightShading()
{
}

Error LightShading::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing light stage");
	Error err = initLightShading(config);

	if(!err)
	{
		err = initApplyFog(config);
	}

	if(err)
	{
		ANKI_R_LOGE("Failed to init light stage");
	}

	return err;
}

Error LightShading::initLightShading(const ConfigSet& config)
{
	// Load shaders and programs
	ANKI_CHECK(getResourceManager().loadResource("shaders/LightShading.glslp", m_lightShading.m_prog));

	ShaderProgramResourceConstantValueInitList<5> consts(m_lightShading.m_prog);
	consts.add("CLUSTER_COUNT_X", U32(m_r->getClusterCount()[0]))
		.add("CLUSTER_COUNT_Y", U32(m_r->getClusterCount()[1]))
		.add("CLUSTER_COUNT_Z", U32(m_r->getClusterCount()[2]))
		.add("CLUSTER_COUNT", U32(m_r->getClusterCount()[3]))
		.add("IR_MIPMAP_COUNT", U32(m_r->getIndirect().getReflectionTextureMipmapCount()));

	const ShaderProgramResourceVariant* variant;
	m_lightShading.m_prog->getOrCreateVariant(consts.get(), variant);
	m_lightShading.m_grProg = variant->getProgram();

	// Create RT descr
	m_lightShading.m_rtDescr = m_r->create2DRenderTargetDescription(
		m_r->getWidth(), m_r->getHeight(), LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT, "Light Shading");
	m_lightShading.m_rtDescr.bake();

	// Create FB descr
	m_lightShading.m_fbDescr.m_colorAttachmentCount = 1;
	m_lightShading.m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_lightShading.m_fbDescr.bake();

	return Error::NONE;
}

Error LightShading::initApplyFog(const ConfigSet& config)
{
	// Load shaders and programs
	ANKI_CHECK(getResourceManager().loadResource("shaders/LightShadingApplyFog.glslp", m_applyFog.m_prog));

	ShaderProgramResourceConstantValueInitList<1> consts(m_applyFog.m_prog);
	consts.add("FOG_LAST_CLASTER", U32(m_r->getVolumetricFog().getFinalClusterInZ()));

	const ShaderProgramResourceVariant* variant;
	m_applyFog.m_prog->getOrCreateVariant(consts.get(), variant);
	m_applyFog.m_grProg = variant->getProgram();

	return Error::NONE;
}

void LightShading::run(RenderPassWorkContext& rgraphCtx)
{
	const RenderingContext& ctx = *m_runCtx.m_ctx;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const ClusterBinOut& rsrc = ctx.m_clusterBinOut;

	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	// Do light shading first
	if(rgraphCtx.m_currentSecondLevelCommandBufferIndex == 0)
	{
		cmdb->bindShaderProgram(m_lightShading.m_grProg);
		cmdb->setDepthWrite(false);

		// Bind textures
		rgraphCtx.bindColorTextureAndSampler(0, 0, m_r->getGBuffer().getColorRt(0), m_r->getNearestSampler());
		rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getGBuffer().getColorRt(1), m_r->getNearestSampler());
		rgraphCtx.bindColorTextureAndSampler(0, 2, m_r->getGBuffer().getColorRt(2), m_r->getNearestSampler());
		rgraphCtx.bindTextureAndSampler(0,
			3,
			m_r->getGBuffer().getDepthRt(),
			TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
			m_r->getNearestSampler());
		rgraphCtx.bindColorTextureAndSampler(0, 4, m_r->getSsr().getRt(), m_r->getLinearSampler());
		rgraphCtx.bindColorTextureAndSampler(0, 5, m_r->getSsao().getRt(), m_r->getLinearSampler());

		rgraphCtx.bindColorTextureAndSampler(0, 6, m_r->getShadowMapping().getShadowmapRt(), m_r->getLinearSampler());
		rgraphCtx.bindColorTextureAndSampler(
			0, 7, m_r->getIndirect().getReflectionRt(), m_r->getTrilinearRepeatSampler());
		rgraphCtx.bindColorTextureAndSampler(
			0, 8, m_r->getIndirect().getIrradianceRt(), m_r->getTrilinearRepeatSampler());
		cmdb->bindTextureAndSampler(0,
			9,
			m_r->getIndirect().getIntegrationLut(),
			m_r->getIndirect().getIntegrationLutSampler(),
			TextureUsageBit::SAMPLED_FRAGMENT);

		// Bind uniforms
		bindUniforms(cmdb, 0, 0, ctx.m_lightShadingUniformsToken);
		bindUniforms(cmdb, 0, 1, rsrc.m_pointLightsToken);
		bindUniforms(cmdb, 0, 2, rsrc.m_spotLightsToken);
		bindUniforms(cmdb, 0, 3, rsrc.m_probesToken);

		// Bind storage
		bindStorage(cmdb, 0, 0, rsrc.m_clustersToken);
		bindStorage(cmdb, 0, 1, rsrc.m_indicesToken);

		drawQuad(cmdb);
	}

	// Do the fog apply
	if(rgraphCtx.m_currentSecondLevelCommandBufferIndex == rgraphCtx.m_secondLevelCommandBufferCount - 1u)
	{
		cmdb->bindShaderProgram(m_applyFog.m_grProg);

		// Bind textures
		rgraphCtx.bindTextureAndSampler(0,
			0,
			m_r->getGBuffer().getDepthRt(),
			TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
			m_r->getNearestSampler());
		rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getVolumetricFog().getRt(), m_r->getLinearSampler());

		// Uniforms
		struct PushConsts
		{
			ClustererMagicValues m_clustererMagic;
			Mat4 m_invViewProjMat;
		} regs;
		regs.m_clustererMagic = ctx.m_clusterBinOut.m_shaderMagicValues;
		regs.m_invViewProjMat = ctx.m_matrices.m_viewProjectionJitter.getInverse();

		cmdb->setPushConstants(&regs, sizeof(regs));

		// finalPixelColor = pixelWithoutFog * transmitance + inScattering (see the shader)
		cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::SRC_ALPHA);

		drawQuad(cmdb);

		// Reset state
		cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	}

	// Forward shading last
	m_r->getForwardShading().run(ctx, rgraphCtx);
}

void LightShading::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_lightShading.m_rtDescr);

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Light&FW Shad.");

	pass.setWork(
		[](RenderPassWorkContext& rgraphCtx) { static_cast<LightShading*>(rgraphCtx.m_userData)->run(rgraphCtx); },
		this,
		computeNumberOfSecondLevelCommandBuffers(ctx.m_renderQueue->m_forwardShadingRenderables.getSize()));
	pass.setFramebufferInfo(m_lightShading.m_fbDescr, {{m_runCtx.m_rt}}, {m_r->getGBuffer().getDepthRt()});

	// Light shading
	pass.newDependency({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	pass.newDependency({m_r->getGBuffer().getColorRt(0), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newDependency({m_r->getGBuffer().getColorRt(1), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newDependency({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newDependency({m_r->getGBuffer().getDepthRt(),
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ,
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});
	pass.newDependency({m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newDependency({m_r->getSsao().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});

	// Refl & indirect
	pass.newDependency({m_r->getSsr().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newDependency({m_r->getIndirect().getReflectionRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newDependency({m_r->getIndirect().getIrradianceRt(), TextureUsageBit::SAMPLED_FRAGMENT});

	// Fog
	pass.newDependency({m_r->getVolumetricFog().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});

	// For forward shading
	m_r->getForwardShading().setDependencies(ctx, pass);
}

} // end namespace anki
