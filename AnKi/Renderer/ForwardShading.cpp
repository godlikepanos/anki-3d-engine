// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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
		cmdb->setBlendFactors(0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA);

		const ClusteredShadingContext& rsrc = ctx.m_clusteredShading;
		cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);

		rgraphCtx.bindTexture(0, 1, m_r->getDepthDownscale().getHiZRt(), HIZ_HALF_DEPTH);
		rgraphCtx.bindColorTexture(0, 2, m_r->getVolumetricLightingAccumulation().getRt());

		bindUniforms(cmdb, 0, 3, rsrc.m_clusteredShadingUniformsToken);
		bindUniforms(cmdb, 0, 4, rsrc.m_pointLightsToken);
		bindUniforms(cmdb, 0, 5, rsrc.m_spotLightsToken);
		rgraphCtx.bindColorTexture(0, 6, m_r->getShadowMapping().getShadowmapRt());
		bindStorage(cmdb, 0, 7, rsrc.m_clustersToken);

		// Start drawing
		m_r->getSceneDrawer().drawRange(Pass::FS, ctx.m_matrices.m_view, ctx.m_matrices.m_viewProjectionJitter,
										ctx.m_prevMatrices.m_viewProjectionJitter, cmdb,
										m_r->getSamplers().m_trilinearRepeatAnisoResolutionScalingBias,
										ctx.m_renderQueue->m_forwardShadingRenderables.getBegin() + start,
										ctx.m_renderQueue->m_forwardShadingRenderables.getBegin() + end);

		// Restore state
		cmdb->setDepthWrite(true);
		cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	}

	if(threadId == threadCount - 1 && ctx.m_renderQueue->m_lensFlares.getSize())
	{
		m_r->getLensFlare().runDrawFlares(ctx, cmdb);
	}
}

void ForwardShading::setDependencies(const RenderingContext& ctx, GraphicsRenderPassDescription& pass)
{
	pass.newDependency({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_FRAGMENT, HIZ_HALF_DEPTH});
	pass.newDependency({m_r->getVolumetricLightingAccumulation().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});

	if(ctx.m_renderQueue->m_lensFlares.getSize())
	{
		pass.newDependency({m_r->getLensFlare().getIndirectDrawBuffer(), BufferUsageBit::INDIRECT_DRAW});
	}
}

} // end namespace anki
