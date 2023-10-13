// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ForwardShading.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/LensFlare.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/LensFlare.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Core/App.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

void ForwardShading::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	const Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};

	FrustumGpuVisibilityInput visIn;
	visIn.m_passesName = "FW shading visibility";
	visIn.m_technique = RenderingTechnique::kForward;
	visIn.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjection;
	visIn.m_lodReferencePoint = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
	visIn.m_lodDistances = lodDistances;
	visIn.m_rgraph = &rgraph;
	visIn.m_gatherAabbIndices = g_dbgCVar.get();
	RenderTargetHandle hzb = getRenderer().getGBuffer().getHzbRt();
	visIn.m_hzbRt = &hzb;

	getRenderer().getGpuVisibility().populateRenderGraph(visIn, m_runCtx.m_visOut);
}

void ForwardShading::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(ForwardShading);
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	// Set state
	cmdb.setDepthWrite(false);
	cmdb.setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);

	// Bind stuff
	const U32 set = U32(MaterialSet::kGlobal);
	cmdb.bindSampler(set, U32(MaterialBinding::kLinearClampSampler), getRenderer().getSamplers().m_trilinearClamp.get());
	cmdb.bindSampler(set, U32(MaterialBinding::kShadowSampler), getRenderer().getSamplers().m_trilinearClampShadow.get());

	rgraphCtx.bindTexture(set, U32(MaterialBinding::kDepthRt), getRenderer().getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
	rgraphCtx.bindColorTexture(set, U32(MaterialBinding::kLightVolume), getRenderer().getVolumetricLightingAccumulation().getRt());

	cmdb.bindConstantBuffer(set, U32(MaterialBinding::kClusterShadingConstants), getRenderer().getClusterBinning().getClusteredShadingConstants());

	cmdb.bindUavBuffer(set, U32(MaterialBinding::kClusterShadingLights),
					   getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));

	rgraphCtx.bindColorTexture(set, U32(MaterialBinding::kClusterShadingLights) + 1, getRenderer().getShadowMapping().getShadowmapRt());

	cmdb.bindUavBuffer(set, U32(MaterialBinding::kClusters), getRenderer().getClusterBinning().getClustersBuffer());

	// Draw
	RenderableDrawerArguments args;
	args.m_viewMatrix = ctx.m_matrices.m_view;
	args.m_cameraTransform = ctx.m_matrices.m_cameraTransform;
	args.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjectionJitter;
	args.m_previousViewProjectionMatrix = ctx.m_matrices.m_jitter * ctx.m_prevMatrices.m_viewProjection;
	args.m_sampler = getRenderer().getSamplers().m_trilinearRepeatAnisoResolutionScalingBias.get();
	args.m_renderingTechinuqe = RenderingTechnique::kForward;
	args.m_viewport = UVec4(0, 0, getRenderer().getInternalResolution());
	args.fillMdi(m_runCtx.m_visOut);
	getRenderer().getSceneDrawer().drawMdi(args, cmdb);

	// Restore state
	cmdb.setDepthWrite(true);
	cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);

	// Do lens flares
	getRenderer().getLensFlare().runDrawFlares(ctx, cmdb);
}

void ForwardShading::setDependencies(GraphicsRenderPassDescription& pass)
{
	pass.newTextureDependency(getRenderer().getDepthDownscale().getRt(), TextureUsageBit::kSampledFragment,
							  DepthDownscale::kQuarterInternalResolution);
	pass.newTextureDependency(getRenderer().getVolumetricLightingAccumulation().getRt(), TextureUsageBit::kSampledFragment);

	if(getRenderer().getLensFlare().getIndirectDrawBuffer().isValid())
	{
		pass.newBufferDependency(getRenderer().getLensFlare().getIndirectDrawBuffer(), BufferUsageBit::kIndirectDraw);
	}

	pass.newBufferDependency(m_runCtx.m_visOut.m_someBufferHandle, BufferUsageBit::kIndirectDraw);
}

} // end namespace anki
