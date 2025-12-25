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
#include <AnKi/Core/App.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

void ForwardShading::populateRenderGraph()
{
	m_runCtx = {};
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	const Array<F32, kMaxLodCount - 1> lodDistances = {g_cvarRenderLod0MaxDistance, g_cvarRenderLod1MaxDistance};

	FrustumGpuVisibilityInput visIn;
	visIn.m_passesName = "FW shading";
	visIn.m_technique = RenderingTechnique::kForward;
	visIn.m_viewProjectionMatrix = getRenderingContext().m_matrices.m_viewProjection;
	visIn.m_lodReferencePoint = getRenderingContext().m_matrices.m_cameraTransform.getTranslationPart().xyz;
	visIn.m_lodDistances = lodDistances;
	visIn.m_rgraph = &rgraph;
	visIn.m_gatherAabbIndices = !!(getDbg().getOptions() & DbgOption::kGatherAabbs);
	RenderTargetHandle hzb = getGBuffer().getHzbRt();
	visIn.m_hzbRt = &hzb;
	visIn.m_viewportSize = getRenderer().getInternalResolution();

	getRenderer().getGpuVisibility().populateRenderGraph(visIn, m_runCtx.m_visOut);
}

void ForwardShading::run(RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(ForwardShading);

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
	RenderingContext& ctx = getRenderingContext();

	if(m_runCtx.m_visOut.containsDrawcalls())
	{
		// Set state
		cmdb.setDepthWrite(false);
		cmdb.setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);

		// Draw
		RenderableDrawerArguments args;
		args.m_viewMatrix = ctx.m_matrices.m_view;
		args.m_cameraTransform = ctx.m_matrices.m_cameraTransform;
		args.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjectionJitter;
		args.m_previousViewProjectionMatrix = ctx.m_matrices.m_jitter * ctx.m_prevMatrices.m_viewProjection;
		args.m_renderingTechinuqe = RenderingTechnique::kForward;
		args.m_viewport = UVec4(0, 0, getRenderer().getInternalResolution());
		args.fill(m_runCtx.m_visOut);

		getRenderer().getRenderableDrawer().drawMdi(args, rgraphCtx);

		// Restore state
		cmdb.setDepthWrite(true);
		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	}

	// Do lens flares
	getLensFlare().runDrawFlares(cmdb);
}

void ForwardShading::setDependencies(GraphicsRenderPass& pass)
{
	const Bool bForwardShading = true;
#define ANKI_DEPENDENCIES 1
#define ANKI_RASTER_PATH 1
#include <AnKi/Shaders/Include/MaterialBindings.def.h>

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
