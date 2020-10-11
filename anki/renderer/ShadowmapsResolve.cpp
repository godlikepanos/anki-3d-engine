// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/ShadowmapsResolve.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/core/ConfigSet.h>

namespace anki
{

ShadowmapsResolve::~ShadowmapsResolve()
{
}

Error ShadowmapsResolve::init(const ConfigSet& cfg)
{
	const Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize shadow resolve pass");
	}

	return Error::NONE;
}

Error ShadowmapsResolve::initInternal(const ConfigSet& cfg)
{
	U32 width = U32(cfg.getNumberF32("r_smResolveFactor") * F32(m_r->getWidth()));
	width = min(m_r->getWidth(), getAlignedRoundUp(4, width));
	U32 height = U32(cfg.getNumberF32("r_smResolveFactor") * F32(m_r->getHeight()));
	height = min(m_r->getHeight(), getAlignedRoundUp(4, height));
	ANKI_R_LOGI("Initializing shadow resolve pass. Size %ux%u", width, height);

	m_rtDescr = m_r->create2DRenderTargetDescription(width, height, Format::R8G8B8A8_UNORM, "SM_resolve");
	m_rtDescr.bake();

	ANKI_CHECK(getResourceManager().loadResource("shaders/ShadowmapsResolve.ankiprog", m_prog));
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("CLUSTER_COUNT_X", U32(m_r->getClusterCount()[0]));
	variantInitInfo.addConstant("CLUSTER_COUNT_Y", U32(m_r->getClusterCount()[1]));
	variantInitInfo.addConstant("FB_SIZE", UVec2(width, height));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	return Error::NONE;
}

void ShadowmapsResolve::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SM_resolve");
	rpass.setWork(
		[](RenderPassWorkContext& rgraphCtx) { static_cast<ShadowmapsResolve*>(rgraphCtx.m_userData)->run(rgraphCtx); },
		this, 0);

	rpass.newDependency({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_WRITE});
	rpass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newDependency({m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::SAMPLED_COMPUTE});
}

void ShadowmapsResolve::run(RenderPassWorkContext& rgraphCtx)
{
	const RenderingContext& ctx = *m_runCtx.m_ctx;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const ClusterBinOut& rsrc = ctx.m_clusterBinOut;

	cmdb->bindShaderProgram(m_grProg);

	rgraphCtx.bindImage(0, 0, m_runCtx.m_rt, TextureSubresourceInfo());
	cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);

	rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

	bindUniforms(cmdb, 0, 3, ctx.m_lightShadingUniformsToken);

	bindUniforms(cmdb, 0, 4, rsrc.m_pointLightsToken);
	bindUniforms(cmdb, 0, 5, rsrc.m_spotLightsToken);
	rgraphCtx.bindColorTexture(0, 6, m_r->getShadowMapping().getShadowmapRt());

	bindStorage(cmdb, 0, 7, rsrc.m_clustersToken);
	bindStorage(cmdb, 0, 8, rsrc.m_indicesToken);

	dispatchPPCompute(cmdb, 8, 8, m_rtDescr.m_width, m_rtDescr.m_height);
}

} // end namespace anki
