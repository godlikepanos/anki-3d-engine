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
		cmdb->setBlendFactors(0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA);

		const ClusteredShadingContext& rsrc = ctx.m_clusteredShading;
		const U32 set = MATERIAL_SET_GLOBAL;
		cmdb->bindSampler(set, MATERIAL_BINDING_LINEAR_CLAMP_SAMPLER, m_r->getSamplers().m_trilinearClamp);

		rgraphCtx.bindTexture(set, MATERIAL_BINDING_DEPTH_RT, m_r->getDepthDownscale().getHiZRt(), HIZ_HALF_DEPTH);
		rgraphCtx.bindColorTexture(set, MATERIAL_BINDING_LIGHT_VOLUME,
								   m_r->getVolumetricLightingAccumulation().getRt());

		bindUniforms(cmdb, set, MATERIAL_BINDING_CLUSTER_SHADING_UNIFORMS, rsrc.m_clusteredShadingUniformsToken);
		bindUniforms(cmdb, set, MATERIAL_BINDING_CLUSTER_SHADING_LIGHTS, rsrc.m_pointLightsToken);
		bindUniforms(cmdb, set, MATERIAL_BINDING_CLUSTER_SHADING_LIGHTS + 1, rsrc.m_spotLightsToken);
		rgraphCtx.bindColorTexture(set, MATERIAL_BINDING_CLUSTER_SHADING_LIGHTS + 2,
								   m_r->getShadowMapping().getShadowmapRt());
		bindStorage(cmdb, set, MATERIAL_BINDING_CLUSTERS, rsrc.m_clustersToken);

		RenderableDrawerArguments args;
		args.m_viewMatrix = ctx.m_matrices.m_view;
		args.m_cameraTransform = ctx.m_matrices.m_cameraTransform;
		args.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjectionJitter;
		args.m_previousViewProjectionMatrix = ctx.m_prevMatrices.m_viewProjectionJitter; // Not sure about that
		args.m_sampler = m_r->getSamplers().m_trilinearRepeatAnisoResolutionScalingBias;

		// Start drawing
		m_r->getSceneDrawer().drawRange(RenderingTechnique::FORWARD, args,
										ctx.m_renderQueue->m_forwardShadingRenderables.getBegin() + start,
										ctx.m_renderQueue->m_forwardShadingRenderables.getBegin() + end, cmdb);

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
