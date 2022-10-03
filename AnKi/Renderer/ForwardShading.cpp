// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ForwardShading.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/LensFlare.h>
#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>

namespace anki {

ForwardShading::~ForwardShading()
{
}

void ForwardShading::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const U32 threadId = rgraphCtx.m_currentSecondLevelCommandBufferIndex;
	const U32 threadCount = rgraphCtx.m_secondLevelCommandBufferCount;
	const U32 problemSize = ctx.m_renderQueue->m_forwardShadingRenderables.getSize();
	U32 start, end;
	splitThreadedProblem(threadId, threadCount, problemSize, start, end);

	if(start != end)
	{
		cmdb->setDepthWrite(false);
		cmdb->setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);

		const ClusteredShadingContext& rsrc = ctx.m_clusteredShading;
		const U32 set = kMaterialSetGlobal;
		cmdb->bindSampler(set, kMaterialBindingLinearClampSampler, m_r->getSamplers().m_trilinearClamp);

		rgraphCtx.bindTexture(set, kMaterialBindingDepthRt, m_r->getDepthDownscale().getHiZRt(), kHiZHalfSurface);
		rgraphCtx.bindColorTexture(set, kMaterialBindingLightVolume, m_r->getVolumetricLightingAccumulation().getRt());

		bindUniforms(cmdb, set, kMaterialBindingClusterShadingUniforms, rsrc.m_clusteredShadingUniformsToken);
		bindUniforms(cmdb, set, kMaterialBindingClusterShadingLights, rsrc.m_pointLightsToken);
		bindUniforms(cmdb, set, kMaterialBindingClusterShadingLights + 1, rsrc.m_spotLightsToken);
		rgraphCtx.bindColorTexture(set, kMaterialBindingClusterShadingLights + 2,
								   m_r->getShadowMapping().getShadowmapRt());
		bindStorage(cmdb, set, kMaterialBindingClusters, rsrc.m_clustersToken);

		RenderableDrawerArguments args;
		args.m_viewMatrix = ctx.m_matrices.m_view;
		args.m_cameraTransform = ctx.m_matrices.m_cameraTransform;
		args.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjectionJitter;
		args.m_previousViewProjectionMatrix = ctx.m_prevMatrices.m_viewProjectionJitter; // Not sure about that
		args.m_sampler = m_r->getSamplers().m_trilinearRepeatAnisoResolutionScalingBias;

		// Start drawing
		m_r->getSceneDrawer().drawRange(RenderingTechnique::kForward, args,
										ctx.m_renderQueue->m_forwardShadingRenderables.getBegin() + start,
										ctx.m_renderQueue->m_forwardShadingRenderables.getBegin() + end, cmdb);

		// Restore state
		cmdb->setDepthWrite(true);
		cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	}

	if(threadId == threadCount - 1 && ctx.m_renderQueue->m_lensFlares.getSize())
	{
		m_r->getLensFlare().runDrawFlares(ctx, cmdb);
	}
}

void ForwardShading::setDependencies(const RenderingContext& ctx, GraphicsRenderPassDescription& pass)
{
	pass.newTextureDependency(m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::kSampledFragment, kHiZHalfSurface);
	pass.newTextureDependency(m_r->getVolumetricLightingAccumulation().getRt(), TextureUsageBit::kSampledFragment);

	if(ctx.m_renderQueue->m_lensFlares.getSize())
	{
		pass.newBufferDependency(m_r->getLensFlare().getIndirectDrawBuffer(), BufferUsageBit::kIndirectDraw);
	}
}

} // end namespace anki
