// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ShadowmapsResolve.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/RtShadows.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

static BoolCVar g_smResolveQuarterRezCVar(CVarSubsystem::kRenderer, "SmResolveQuarterRez", ANKI_PLATFORM_MOBILE, "Shadowmapping resolve quality");

Error ShadowmapsResolve::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize shadow resolve pass");
	}

	return Error::kNone;
}

Error ShadowmapsResolve::initInternal()
{
	m_quarterRez = g_smResolveQuarterRezCVar.get();
	const U32 width = getRenderer().getInternalResolution().x() / (m_quarterRez + 1);
	const U32 height = getRenderer().getInternalResolution().y() / (m_quarterRez + 1);

	ANKI_R_LOGV("Initializing shadowmaps resolve. Resolution %ux%u", width, height);

	m_rtDescr = getRenderer().create2DRenderTargetDescription(width, height, Format::kR8G8B8A8_Unorm, "SM resolve");
	m_rtDescr.bake();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	// Prog
	ANKI_CHECK(ResourceManager::getSingleton().loadResource((g_preferComputeCVar.get()) ? "ShaderBinaries/ShadowmapsResolveCompute.ankiprogbin"
																						: "ShaderBinaries/ShadowmapsResolveRaster.ankiprogbin",
															m_prog));
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("kFramebufferSize", UVec2(width, height));
	variantInitInfo.addConstant("kTileCount", getRenderer().getTileCounts());
	variantInitInfo.addConstant("kZSplitCount", getRenderer().getZSplitCount());
	variantInitInfo.addMutation("PCF", g_shadowMappingPcfCVar.get() != 0);
	variantInitInfo.addMutation("DIRECTIONAL_LIGHT_SHADOW_RESOLVED", getRenderer().getRtShadowsEnabled());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg.reset(&variant->getProgram());

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	return Error::kNone;
}

void ShadowmapsResolve::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(ShadowmapsResolve);

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	if(g_preferComputeCVar.get())
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("ResolveShadows");

		rpass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			run(rgraphCtx);
		});

		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kUavComputeWrite);
		rpass.newTextureDependency((m_quarterRez) ? getRenderer().getDepthDownscale().getRt() : getRenderer().getGBuffer().getDepthRt(),
								   TextureUsageBit::kSampledCompute, TextureSurfaceInfo(0, 0, 0, 0));
		rpass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSampledCompute);

		rpass.newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), BufferUsageBit::kUavComputeRead);
		rpass.newBufferDependency(getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kLight),
								  BufferUsageBit::kUavComputeRead);

		if(getRenderer().getRtShadowsEnabled())
		{
			rpass.newTextureDependency(getRenderer().getRtShadows().getRt(), TextureUsageBit::kSampledCompute);
		}
	}
	else
	{
		GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("ResolveShadows");
		rpass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_rt});

		rpass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			run(rgraphCtx);
		});

		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite);
		rpass.newTextureDependency((m_quarterRez) ? getRenderer().getDepthDownscale().getRt() : getRenderer().getGBuffer().getDepthRt(),
								   TextureUsageBit::kSampledFragment, TextureSurfaceInfo(0, 0, 0, 0));
		rpass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSampledFragment);

		rpass.newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), BufferUsageBit::kUavFragmentRead);
		rpass.newBufferDependency(getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kLight),
								  BufferUsageBit::kUavFragmentRead);

		if(getRenderer().getRtShadowsEnabled())
		{
			rpass.newTextureDependency(getRenderer().getRtShadows().getRt(), TextureUsageBit::kSampledFragment);
		}
	}
}

void ShadowmapsResolve::run(RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(ShadowmapsResolve);
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	cmdb.bindShaderProgram(m_grProg.get());

	cmdb.bindConstantBuffer(0, 0, getRenderer().getClusterBinning().getClusteredShadingConstants());
	cmdb.bindUavBuffer(0, 1, getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
	rgraphCtx.bindColorTexture(0, 2, getRenderer().getShadowMapping().getShadowmapRt());
	cmdb.bindUavBuffer(0, 3, getRenderer().getClusterBinning().getClustersBuffer());

	cmdb.bindSampler(0, 4, getRenderer().getSamplers().m_trilinearClamp.get());
	cmdb.bindSampler(0, 5, getRenderer().getSamplers().m_trilinearClampShadow.get());
	cmdb.bindSampler(0, 6, getRenderer().getSamplers().m_trilinearRepeat.get());

	if(m_quarterRez)
	{
		rgraphCtx.bindTexture(0, 7, getRenderer().getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
	}
	else
	{
		rgraphCtx.bindTexture(0, 7, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
	}
	cmdb.bindTexture(0, 8, &m_noiseImage->getTextureView());

	if(getRenderer().getRtShadowsEnabled())
	{
		rgraphCtx.bindColorTexture(0, 9, getRenderer().getRtShadows().getRt());
	}

	if(g_preferComputeCVar.get())
	{
		rgraphCtx.bindUavTexture(0, 10, m_runCtx.m_rt, TextureSubresourceInfo());
		dispatchPPCompute(cmdb, 8, 8, m_rtDescr.m_width, m_rtDescr.m_height);
	}
	else
	{
		cmdb.setViewport(0, 0, m_rtDescr.m_width, m_rtDescr.m_height);
		cmdb.draw(PrimitiveTopology::kTriangles, 3);
	}
}

} // end namespace anki
