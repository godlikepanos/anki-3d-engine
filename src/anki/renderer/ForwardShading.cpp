// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/ForwardShading.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/Volumetric.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/LensFlare.h>
#include <anki/renderer/VolumetricLightingAccumulation.h>

namespace anki
{

ForwardShading::~ForwardShading()
{
}

Error ForwardShading::init(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing forward shading");

	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize forward shading");
	}

	return err;
}

Error ForwardShading::initInternal(const ConfigSet&)
{
	ANKI_CHECK(initVol());

	return Error::NONE;
}

Error ForwardShading::initVol()
{
	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_vol.m_noiseTex));

	ANKI_CHECK(getResourceManager().loadResource("shaders/ForwardShadingVolumetricUpscale.glslp", m_vol.m_prog));

	ShaderProgramResourceConstantValueInitList<3> consts(m_vol.m_prog);
	consts.add("NOISE_TEX_SIZE", U32(m_vol.m_noiseTex->getWidth()))
		.add("SRC_SIZE", Vec2(m_r->getWidth() / VOLUMETRIC_FRACTION, m_r->getHeight() / VOLUMETRIC_FRACTION))
		.add("FB_SIZE", Vec2(m_r->getWidth(), m_r->getHeight()));

	const ShaderProgramResourceVariant* variant;
	m_vol.m_prog->getOrCreateVariant(consts.get(), variant);
	m_vol.m_grProg = variant->getProgram();

	return Error::NONE;
}

void ForwardShading::drawVolumetric(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);
	cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);
	cmdb->setDepthWrite(false);

	cmdb->bindShaderProgram(m_vol.m_grProg);
	Vec4* unis = allocateAndBindUniforms<Vec4*>(sizeof(Vec4), cmdb, 0, 0);
	computeLinearizeDepthOptimal(ctx.m_renderQueue->m_cameraNear, ctx.m_renderQueue->m_cameraFar, unis->x(), unis->y());

	rgraphCtx.bindTextureAndSampler(
		0, 0, m_r->getDepthDownscale().getHiZRt(), HIZ_HALF_DEPTH, m_r->getNearestSampler());
	rgraphCtx.bindTextureAndSampler(
		0, 1, m_r->getDepthDownscale().getHiZRt(), HIZ_QUARTER_DEPTH, m_r->getNearestSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 2, m_r->getVolumetric().getRt(), m_r->getLinearSampler());
	cmdb->bindTextureAndSampler(0,
		3,
		m_vol.m_noiseTex->getGrTextureView(),
		m_r->getTrilinearRepeatSampler(),
		TextureUsageBit::SAMPLED_FRAGMENT);

	drawQuad(cmdb);

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setDepthCompareOperation(CompareOperation::LESS);
	cmdb->setDepthWrite(true);
}

void ForwardShading::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const U threadId = rgraphCtx.m_currentSecondLevelCommandBufferIndex;
	const U threadCount = rgraphCtx.m_secondLevelCommandBufferCount;
	const U problemSize = ctx.m_renderQueue->m_forwardShadingRenderables.getSize();
	PtrSize start, end;
	splitThreadedProblem(threadId, threadCount, problemSize, start, end);

	if(start != end)
	{
		cmdb->setDepthWrite(false);
		cmdb->setBlendFactors(0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA);

		const ClusterBinOut& rsrc = ctx.m_clusterBinOut;
		rgraphCtx.bindTextureAndSampler(
			0, 0, m_r->getDepthDownscale().getHiZRt(), HIZ_HALF_DEPTH, m_r->getLinearSampler());
		rgraphCtx.bindColorTextureAndSampler(
			0, 1, m_r->getVolumetricLightingAccumulation().getRt(), m_r->getLinearSampler());
		rgraphCtx.bindColorTextureAndSampler(0, 2, m_r->getShadowMapping().getShadowmapRt(), m_r->getLinearSampler());
		bindUniforms(cmdb, 0, 0, ctx.m_lightShadingUniformsToken);
		bindUniforms(cmdb, 0, 1, rsrc.m_pointLightsToken);
		bindUniforms(cmdb, 0, 2, rsrc.m_spotLightsToken);
		bindStorage(cmdb, 0, 0, rsrc.m_clustersToken);
		bindStorage(cmdb, 0, 1, rsrc.m_indicesToken);

		// Start drawing
		m_r->getSceneDrawer().drawRange(Pass::FS,
			ctx.m_matrices.m_view,
			ctx.m_matrices.m_viewProjectionJitter,
			ctx.m_prevMatrices.m_viewProjectionJitter,
			cmdb,
			ctx.m_renderQueue->m_forwardShadingRenderables.getBegin() + start,
			ctx.m_renderQueue->m_forwardShadingRenderables.getBegin() + end);

		// Restore state
		cmdb->setDepthWrite(true);
		cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	}

	if(threadId == threadCount - 1)
	{
		drawVolumetric(ctx, rgraphCtx);

		if(ctx.m_renderQueue->m_lensFlares.getSize())
		{
			m_r->getLensFlare().runDrawFlares(ctx, cmdb);
		}
	}
}

void ForwardShading::setDependencies(const RenderingContext& ctx, GraphicsRenderPassDescription& pass)
{
	pass.newDependency({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_FRAGMENT, HIZ_HALF_DEPTH});
	pass.newDependency({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_FRAGMENT, HIZ_QUARTER_DEPTH});
	pass.newDependency({m_r->getVolumetric().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newDependency({m_r->getVolumetricLightingAccumulation().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});

	if(ctx.m_renderQueue->m_lensFlares.getSize())
	{
		pass.newDependency({m_r->getLensFlare().getIndirectDrawBuffer(), BufferUsageBit::INDIRECT_GRAPHICS});
	}
}

} // end namespace anki
