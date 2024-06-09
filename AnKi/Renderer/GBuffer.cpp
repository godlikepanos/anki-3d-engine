// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/VrsSriGeneration.h>
#include <AnKi/Renderer/Scale.h>
#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Core/App.h>

namespace anki {

static NumericCVar<U32> g_hzbWidthCVar(CVarSubsystem::kRenderer, "HzbWidth", 512, 16, 4 * 1024, "HZB map width");
static NumericCVar<U32> g_hzbHeightCVar(CVarSubsystem::kRenderer, "HzbHeight", 256, 16, 4 * 1024, "HZB map height");
static BoolCVar g_gbufferVrsCVar(CVarSubsystem::kRenderer, "GBufferVrs", false, "Enable VRS in GBuffer");

GBuffer::~GBuffer()
{
}

Error GBuffer::init()
{
	Error err = initInternal();

	if(err)
	{
		ANKI_R_LOGE("Failed to initialize g-buffer pass");
	}

	return err;
}

Error GBuffer::initInternal()
{
	ANKI_R_LOGV("Initializing GBuffer. Resolution %ux%u", getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());

	// RTs
	static constexpr Array<const char*, 2> depthRtNames = {{"GBuffer depth #0", "GBuffer depth #1"}};
	for(U32 i = 0; i < 2; ++i)
	{
		const TextureUsageBit usage = TextureUsageBit::kAllSampled | TextureUsageBit::kAllFramebuffer;
		TextureInitInfo texinit =
			getRenderer().create2DRenderTargetInitInfo(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
													   getRenderer().getDepthNoStencilFormat(), usage, depthRtNames[i]);

		m_depthRts[i] = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment);
	}

	static constexpr Array<const char*, kGBufferColorRenderTargetCount> rtNames = {{"GBuffer rt0", "GBuffer rt1", "GBuffer rt2", "GBuffer rt3"}};
	for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
	{
		m_colorRtDescrs[i] = getRenderer().create2DRenderTargetDescription(
			getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), kGBufferColorRenderTargetFormats[i], rtNames[i]);
		m_colorRtDescrs[i].bake();
	}

	{
		const TextureUsageBit usage = TextureUsageBit::kSampledCompute | TextureUsageBit::kStorageComputeWrite | TextureUsageBit::kSampledGeometry;

		TextureInitInfo texinit =
			getRenderer().create2DRenderTargetInitInfo(g_hzbWidthCVar.get(), g_hzbHeightCVar.get(), Format::kR32_Sfloat, usage, "GBuffer HZB");
		texinit.m_mipmapCount = U8(computeMaxMipmapCount2d(texinit.m_width, texinit.m_height));
		ClearValue clear;
		clear.m_colorf = {1.0f, 1.0f, 1.0f, 1.0f};
		m_hzbRt = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSampledCompute, clear);
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/VisualizeGBufferNormal.ankiprogbin", m_visNormalProg, m_visNormalGrProg));

	return Error::kNone;
}

void GBuffer::importRenderTargets(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	if(m_runCtx.m_crntFrameDepthRt.isValid()) [[likely]]
	{
		// Already imported once
		m_runCtx.m_crntFrameDepthRt = rgraph.importRenderTarget(m_depthRts[getRenderer().getFrameCount() & 1].get(), TextureUsageBit::kNone);
		m_runCtx.m_prevFrameDepthRt = rgraph.importRenderTarget(m_depthRts[(getRenderer().getFrameCount() + 1) & 1].get());

		m_runCtx.m_hzbRt = rgraph.importRenderTarget(m_hzbRt.get());
	}
	else
	{
		m_runCtx.m_crntFrameDepthRt = rgraph.importRenderTarget(m_depthRts[getRenderer().getFrameCount() & 1].get(), TextureUsageBit::kNone);
		m_runCtx.m_prevFrameDepthRt =
			rgraph.importRenderTarget(m_depthRts[(getRenderer().getFrameCount() + 1) & 1].get(), TextureUsageBit::kSampledFragment);

		m_runCtx.m_hzbRt = rgraph.importRenderTarget(m_hzbRt.get(), TextureUsageBit::kSampledCompute);
	}
}

void GBuffer::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(GBuffer);

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Visibility
	GpuVisibilityOutput visOut;
	GpuMeshletVisibilityOutput meshletVisOut;
	{
		const CommonMatrices& matrices = (getRenderer().getFrameCount() <= 1) ? ctx.m_matrices : ctx.m_prevMatrices;
		const Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};

		FrustumGpuVisibilityInput visIn;
		visIn.m_passesName = "GBuffer";
		visIn.m_technique = RenderingTechnique::kGBuffer;
		visIn.m_viewProjectionMatrix = matrices.m_viewProjection;
		visIn.m_lodReferencePoint = matrices.m_cameraTransform.getTranslationPart().xyz();
		visIn.m_lodDistances = lodDistances;
		visIn.m_rgraph = &rgraph;
		visIn.m_hzbRt = &m_runCtx.m_hzbRt;
		visIn.m_gatherAabbIndices = g_dbgCVar.get();
		visIn.m_viewportSize = getRenderer().getInternalResolution();

		getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOut);

		m_runCtx.m_visibleAaabbIndicesBuffer = visOut.m_visibleAaabbIndicesBuffer;
		m_runCtx.m_visibleAaabbIndicesBufferDepedency = visOut.m_dependency;

		if(getRenderer().runSoftwareMeshletRendering())
		{
			GpuMeshletVisibilityInput meshIn;
			meshIn.m_passesName = "GBuffer";
			meshIn.m_technique = RenderingTechnique::kGBuffer;
			meshIn.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjection;
			meshIn.m_cameraTransform = ctx.m_matrices.m_cameraTransform;
			meshIn.m_viewportSize = getRenderer().getInternalResolution();
			meshIn.m_rgraph = &rgraph;
			meshIn.m_hzbRt = getRenderer().getGBuffer().getHzbRt();
			meshIn.fillBuffers(visOut);

			getRenderer().getGpuVisibility().populateRenderGraph(meshIn, meshletVisOut);
		}
	}

	const Bool enableVrs = GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_vrsCVar.get() && g_gbufferVrsCVar.get();

	// Create RTs
	Array<RenderTargetHandle, kMaxColorRenderTargets> rts;
	for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
	{
		m_runCtx.m_colorRts[i] = rgraph.newRenderTarget(m_colorRtDescrs[i]);
		rts[i] = m_runCtx.m_colorRts[i];
	}

	RenderTargetHandle sriRt;
	if(enableVrs)
	{
		sriRt = getRenderer().getVrsSriGeneration().getSriRt();
	}

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GBuffer");

	Array<RenderTargetInfo, kGBufferColorRenderTargetCount> colorRti;
	colorRti[0].m_handle = rts[0];
	colorRti[0].m_loadOperation = RenderTargetLoadOperation::kClear;
	colorRti[1].m_handle = rts[1];
	colorRti[1].m_loadOperation = RenderTargetLoadOperation::kClear;
	colorRti[2].m_handle = rts[2];
	colorRti[2].m_loadOperation = RenderTargetLoadOperation::kClear;
	colorRti[3].m_handle = rts[3];
	colorRti[3].m_loadOperation = RenderTargetLoadOperation::kClear;
	colorRti[3].m_clearValue.m_colorf = {1.0f, 1.0f, 1.0f, 1.0f};
	RenderTargetInfo depthRti(m_runCtx.m_crntFrameDepthRt);
	depthRti.m_loadOperation = RenderTargetLoadOperation::kClear;
	depthRti.m_clearValue.m_depthStencil.m_depth = 1.0f;
	depthRti.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;

	pass.setRenderpassInfo(WeakArray{colorRti}, &depthRti, 0, 0, kMaxU32, kMaxU32, (enableVrs) ? &sriRt : nullptr,
						   (enableVrs) ? getRenderer().getVrsSriGeneration().getSriTexelDimension() : 0,
						   (enableVrs) ? getRenderer().getVrsSriGeneration().getSriTexelDimension() : 0);
	pass.setWork([this, &ctx, visOut, meshletVisOut](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(GBuffer);

		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		// Set some state, leave the rest to default
		cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		cmdb.setRasterizationOrder(RasterizationOrder::kRelaxed);

		const Bool enableVrs = GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_vrsCVar.get() && g_gbufferVrsCVar.get();
		if(enableVrs)
		{
			// Just set some low value, the attachment will take over
			cmdb.setVrsRate(VrsRate::k1x1);
		}

		RenderableDrawerArguments args;
		args.m_viewMatrix = ctx.m_matrices.m_view;
		args.m_cameraTransform = ctx.m_matrices.m_cameraTransform;
		args.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjectionJitter;
		args.m_previousViewProjectionMatrix = ctx.m_matrices.m_jitter * ctx.m_prevMatrices.m_viewProjection;
		args.m_sampler = getRenderer().getSamplers().m_trilinearRepeatAnisoResolutionScalingBias.get();
		args.m_renderingTechinuqe = RenderingTechnique::kGBuffer;
		args.m_viewport = UVec4(0, 0, getRenderer().getInternalResolution());

		if(GrManager::getSingleton().getDeviceCapabilities().m_meshShaders)
		{
			const TextureSubresourceDescriptor subresource = TextureSubresourceDescriptor::all();
			Texture* tex;
			rgraphCtx.getRenderTargetState(m_runCtx.m_hzbRt, subresource, tex);
			args.m_hzbTexture = TextureView(tex, subresource);
		}

		args.fill(visOut);
		if(meshletVisOut.isFilled())
		{
			args.fill(meshletVisOut);
		}

		cmdb.setDepthCompareOperation(CompareOperation::kLessEqual);
		getRenderer().getSceneDrawer().drawMdi(args, cmdb);
	});

	for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
	{
		pass.newTextureDependency(m_runCtx.m_colorRts[i], TextureUsageBit::kFramebufferWrite);
	}

	pass.newTextureDependency(m_runCtx.m_crntFrameDepthRt, TextureUsageBit::kAllFramebuffer);

	if(enableVrs)
	{
		pass.newTextureDependency(sriRt, TextureUsageBit::kFramebufferShadingRate);
	}

	if(GrManager::getSingleton().getDeviceCapabilities().m_meshShaders)
	{
		pass.newTextureDependency(m_runCtx.m_hzbRt, TextureUsageBit::kSampledGeometry);
	}

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kStorageGeometryRead | BufferUsageBit::kStorageFragmentRead);

	// Only add one depedency to the GPU visibility. No need to track all buffers
	if(meshletVisOut.isFilled())
	{
		pass.newBufferDependency(meshletVisOut.m_dependency, BufferUsageBit::kIndirectDraw);
	}
	else if(visOut.containsDrawcalls())
	{
		pass.newBufferDependency(visOut.m_dependency, BufferUsageBit::kIndirectDraw);
	}
	else
	{
		// Weird, make a check
		ANKI_ASSERT(GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount() == 0);
	}

	// HZB generation for the next frame
	getRenderer().getHzbGenerator().populateRenderGraph(m_runCtx.m_crntFrameDepthRt, getRenderer().getInternalResolution(), m_runCtx.m_hzbRt,
														UVec2(m_hzbRt->getWidth(), m_hzbRt->getHeight()), rgraph);
}

void GBuffer::getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
								   ShaderProgramPtr& optionalShaderProgram) const
{
	if(rtName == "GBufferAlbedo")
	{
		handles[0] = m_runCtx.m_colorRts[0];
	}
	else if(rtName == "GBufferNormals")
	{
		handles[0] = m_runCtx.m_colorRts[2];
		optionalShaderProgram = m_visNormalGrProg;
	}
	else if(rtName == "GBufferVelocity")
	{
		handles[0] = m_runCtx.m_colorRts[3];
	}
	else
	{
		ANKI_ASSERT(!"See file");
	}
}

} // end namespace anki
