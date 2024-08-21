// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Renderer/Utils/Drawer.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Core/App.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

void ForwardShading::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx = {};
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	const Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};

	FrustumGpuVisibilityInput visIn;
	visIn.m_passesName = "FW shading";
	visIn.m_technique = RenderingTechnique::kForward;
	visIn.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjection;
	visIn.m_lodReferencePoint = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
	visIn.m_lodDistances = lodDistances;
	visIn.m_rgraph = &rgraph;
	visIn.m_gatherAabbIndices = g_dbgCVar.get();
	RenderTargetHandle hzb = getRenderer().getGBuffer().getHzbRt();
	visIn.m_hzbRt = &hzb;
	visIn.m_viewportSize = getRenderer().getInternalResolution();

	getRenderer().getGpuVisibility().populateRenderGraph(visIn, m_runCtx.m_visOut);
}

void ForwardShading::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(ForwardShading);

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	if(m_runCtx.m_visOut.containsDrawcalls())
	{
		// Set state
		cmdb.setDepthWrite(false);
		cmdb.setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);

		// Bind stuff
		cmdb.bindSampler(ANKI_MATERIAL_REGISTER_LINEAR_CLAMP_SAMPLER, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		cmdb.bindSampler(ANKI_MATERIAL_REGISTER_SHADOW_SAMPLER, 0, getRenderer().getSamplers().m_trilinearClampShadow.get());

		rgraphCtx.bindSrv(ANKI_MATERIAL_REGISTER_SCENE_DEPTH, 0, getRenderer().getDepthDownscale().getRt(),
						  DepthDownscale::kQuarterInternalResolution);
		rgraphCtx.bindSrv(ANKI_MATERIAL_REGISTER_LIGHT_VOLUME, 0, getRenderer().getVolumetricLightingAccumulation().getRt());

		cmdb.bindConstantBuffer(ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_CONSTANTS, 0, ctx.m_globalRenderingConstantsBuffer);

		cmdb.bindSrv(ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_POINT_LIGHTS, 0,
					 getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
		cmdb.bindSrv(ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_SPOT_LIGHTS, 0,
					 getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));

		rgraphCtx.bindSrv(ANKI_MATERIAL_REGISTER_SHADOW_ATLAS, 0, getRenderer().getShadowMapping().getShadowmapRt());

		cmdb.bindSrv(ANKI_MATERIAL_REGISTER_CLUSTERS, 0, getRenderer().getClusterBinning().getClustersBuffer());

		// Draw
		RenderableDrawerArguments args;
		args.m_viewMatrix = ctx.m_matrices.m_view;
		args.m_cameraTransform = ctx.m_matrices.m_cameraTransform;
		args.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjectionJitter;
		args.m_previousViewProjectionMatrix = ctx.m_matrices.m_jitter * ctx.m_prevMatrices.m_viewProjection;
		args.m_sampler = getRenderer().getSamplers().m_trilinearRepeatAnisoResolutionScalingBias.get();
		args.m_renderingTechinuqe = RenderingTechnique::kForward;
		args.m_viewport = UVec4(0, 0, getRenderer().getInternalResolution());
		args.fill(m_runCtx.m_visOut);

		getRenderer().getRenderableDrawer().drawMdi(args, cmdb);

		// Restore state
		cmdb.setDepthWrite(true);
		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	}

	// Do lens flares
	getRenderer().getLensFlare().runDrawFlares(ctx, cmdb);
}

void ForwardShading::setDependencies(GraphicsRenderPass& pass)
{
	pass.newTextureDependency(getRenderer().getDepthDownscale().getRt(), TextureUsageBit::kSrvPixel, DepthDownscale::kQuarterInternalResolution);
	pass.newTextureDependency(getRenderer().getVolumetricLightingAccumulation().getRt(), TextureUsageBit::kSrvPixel);

	if(getRenderer().getLensFlare().getIndirectDrawBuffer().isValid())
	{
		pass.newBufferDependency(getRenderer().getLensFlare().getIndirectDrawBuffer(), BufferUsageBit::kIndirectDraw);
	}

	if(m_runCtx.m_visOut.containsDrawcalls())
	{
		pass.newBufferDependency(m_runCtx.m_visOut.m_dependency, BufferUsageBit::kIndirectDraw);
	}
}

} // end namespace anki
