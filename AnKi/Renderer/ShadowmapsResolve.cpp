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

	// Prog
	ANKI_CHECK(loadShaderProgram(
		"ShaderBinaries/ShadowmapsResolve.ankiprogbin",
		{{"PCF", g_shadowMappingPcfCVar.get() != 0}, {"DIRECTIONAL_LIGHT_SHADOW_RESOLVED", getRenderer().getRtShadowsEnabled()}}, m_prog, m_grProg));

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

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			run(rgraphCtx, ctx);
		});

		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kStorageComputeWrite);
		rpass.newTextureDependency((m_quarterRez) ? getRenderer().getDepthDownscale().getRt() : getRenderer().getGBuffer().getDepthRt(),
								   TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSampledCompute);

		rpass.newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), BufferUsageBit::kStorageComputeRead);
		rpass.newBufferDependency(getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kLight),
								  BufferUsageBit::kStorageComputeRead);

		if(getRenderer().getRtShadowsEnabled())
		{
			rpass.newTextureDependency(getRenderer().getRtShadows().getRt(), TextureUsageBit::kSampledCompute);
		}
	}
	else
	{
		GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("ResolveShadows");

		rpass.setRenderpassInfo({RenderTargetInfo(m_runCtx.m_rt)});

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			run(rgraphCtx, ctx);
		});

		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite);
		rpass.newTextureDependency((m_quarterRez) ? getRenderer().getDepthDownscale().getRt() : getRenderer().getGBuffer().getDepthRt(),
								   TextureUsageBit::kSampledFragment);
		rpass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSampledFragment);

		rpass.newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), BufferUsageBit::kStorageFragmentRead);
		rpass.newBufferDependency(getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kLight),
								  BufferUsageBit::kStorageFragmentRead);

		if(getRenderer().getRtShadowsEnabled())
		{
			rpass.newTextureDependency(getRenderer().getRtShadows().getRt(), TextureUsageBit::kSampledFragment);
		}
	}
}

void ShadowmapsResolve::run(RenderPassWorkContext& rgraphCtx, RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(ShadowmapsResolve);
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	cmdb.bindShaderProgram(m_grProg.get());

	cmdb.bindUniformBuffer(ANKI_REG(b0), ctx.m_globalRenderingUniformsBuffer);
	cmdb.bindStorageBuffer(ANKI_REG(t0), getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
	cmdb.bindStorageBuffer(ANKI_REG(t1), getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
	rgraphCtx.bindTexture(ANKI_REG(t2), getRenderer().getShadowMapping().getShadowmapRt());
	cmdb.bindStorageBuffer(ANKI_REG(t3), getRenderer().getClusterBinning().getClustersBuffer());

	cmdb.bindSampler(ANKI_REG(s0), getRenderer().getSamplers().m_trilinearClamp.get());
	cmdb.bindSampler(ANKI_REG(s1), getRenderer().getSamplers().m_trilinearClampShadow.get());
	cmdb.bindSampler(ANKI_REG(s2), getRenderer().getSamplers().m_trilinearRepeat.get());

	if(m_quarterRez)
	{
		rgraphCtx.bindTexture(ANKI_REG(t4), getRenderer().getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
	}
	else
	{
		rgraphCtx.bindTexture(ANKI_REG(t4), getRenderer().getGBuffer().getDepthRt());
	}
	cmdb.bindTexture(ANKI_REG(t5), TextureView(&m_noiseImage->getTexture(), TextureSubresourceDescriptor::all()));

	if(getRenderer().getRtShadowsEnabled())
	{
		rgraphCtx.bindTexture(ANKI_REG(t6), getRenderer().getRtShadows().getRt());
	}

	if(g_preferComputeCVar.get() || g_shadowMappingPcfCVar.get())
	{
		const Vec4 consts(F32(m_rtDescr.m_width), F32(m_rtDescr.m_height), 0.0f, 0.0f);
		cmdb.setPushConstants(&consts, sizeof(consts));
	}

	if(g_preferComputeCVar.get())
	{
		rgraphCtx.bindTexture(ANKI_REG(u0), m_runCtx.m_rt);
		dispatchPPCompute(cmdb, 8, 8, m_rtDescr.m_width, m_rtDescr.m_height);
	}
	else
	{
		cmdb.setViewport(0, 0, m_rtDescr.m_width, m_rtDescr.m_height);
		cmdb.draw(PrimitiveTopology::kTriangles, 3);
	}
}

} // end namespace anki
