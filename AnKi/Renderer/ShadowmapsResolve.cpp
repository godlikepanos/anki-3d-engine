// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ShadowmapsResolve.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

ShadowmapsResolve::~ShadowmapsResolve()
{
}

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
	m_quarterRez = getConfig().getRSmResolveQuarterRez();
	const U32 width = m_r->getInternalResolution().x() / (m_quarterRez + 1);
	const U32 height = m_r->getInternalResolution().y() / (m_quarterRez + 1);

	ANKI_R_LOGV("Initializing shadowmaps resolve. Resolution %ux%u", width, height);

	m_rtDescr = m_r->create2DRenderTargetDescription(width, height, Format::kR8G8B8A8_Unorm, "SM resolve");
	m_rtDescr.bake();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	// Prog
	ANKI_CHECK(getResourceManager().loadResource((getConfig().getRPreferCompute())
													 ? "ShaderBinaries/ShadowmapsResolveCompute.ankiprogbin"
													 : "ShaderBinaries/ShadowmapsResolveRaster.ankiprogbin",
												 m_prog));
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	if(getConfig().getRPreferCompute())
	{
		variantInitInfo.addConstant("kFramebufferSize", UVec2(width, height));
	}
	variantInitInfo.addConstant("kTileCount", m_r->getTileCounts());
	variantInitInfo.addConstant("kZSplitCount", m_r->getZSplitCount());
	variantInitInfo.addConstant("kTileSize", m_r->getTileSize());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	return Error::kNone;
}

void ShadowmapsResolve::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	if(getConfig().getRPreferCompute())
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SM resolve");

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			run(ctx, rgraphCtx);
		});

		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kImageComputeWrite);
		rpass.newTextureDependency((m_quarterRez) ? m_r->getDepthDownscale().getHiZRt()
												  : m_r->getGBuffer().getDepthRt(),
								   TextureUsageBit::kSampledCompute, TextureSurfaceInfo(0, 0, 0, 0));
		rpass.newTextureDependency(m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::kSampledCompute);

		rpass.newBufferDependency(ctx.m_clusteredShading.m_clustersBufferHandle, BufferUsageBit::kStorageComputeRead);
	}
	else
	{
		GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("SM resolve");
		rpass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_rt});

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			run(ctx, rgraphCtx);
		});

		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite);
		rpass.newTextureDependency((m_quarterRez) ? m_r->getDepthDownscale().getHiZRt()
												  : m_r->getGBuffer().getDepthRt(),
								   TextureUsageBit::kSampledFragment, TextureSurfaceInfo(0, 0, 0, 0));
		rpass.newTextureDependency(m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::kSampledFragment);

		rpass.newBufferDependency(ctx.m_clusteredShading.m_clustersBufferHandle, BufferUsageBit::kStorageFragmentRead);
	}
}

void ShadowmapsResolve::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const ClusteredShadingContext& rsrc = ctx.m_clusteredShading;

	cmdb->bindShaderProgram(m_grProg);

	bindUniforms(cmdb, 0, 0, rsrc.m_clusteredShadingUniformsToken);
	bindUniforms(cmdb, 0, 1, rsrc.m_pointLightsToken);
	bindUniforms(cmdb, 0, 2, rsrc.m_spotLightsToken);
	rgraphCtx.bindColorTexture(0, 3, m_r->getShadowMapping().getShadowmapRt());
	bindStorage(cmdb, 0, 4, rsrc.m_clustersToken);

	cmdb->bindSampler(0, 5, m_r->getSamplers().m_trilinearClamp);
	if(m_quarterRez)
	{
		rgraphCtx.bindTexture(0, 6, m_r->getDepthDownscale().getHiZRt(),
							  TextureSubresourceInfo(TextureSurfaceInfo(0, 0, 0, 0)));
	}
	else
	{
		rgraphCtx.bindTexture(0, 6, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
	}

	if(getConfig().getRPreferCompute())
	{
		rgraphCtx.bindImage(0, 7, m_runCtx.m_rt, TextureSubresourceInfo());
		dispatchPPCompute(cmdb, 8, 8, m_rtDescr.m_width, m_rtDescr.m_height);
	}
	else
	{
		cmdb->setViewport(0, 0, m_rtDescr.m_width, m_rtDescr.m_height);
		cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
	}
}

} // end namespace anki
