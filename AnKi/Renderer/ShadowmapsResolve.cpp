// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ShadowmapsResolve.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/ShadowMapping.h>
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

	return Error::NONE;
}

Error ShadowmapsResolve::initInternal()
{
	U32 width = U32(getConfig().getRSmResolveFactor() * F32(m_r->getInternalResolution().x()));
	width = min(m_r->getInternalResolution().x(), getAlignedRoundUp(4, width));
	U32 height = U32(getConfig().getRSmResolveFactor() * F32(m_r->getInternalResolution().y()));
	height = min(m_r->getInternalResolution().y(), getAlignedRoundUp(4, height));

	m_rtDescr = m_r->create2DRenderTargetDescription(width, height, Format::R8G8B8A8_UNORM, "SM resolve");
	m_rtDescr.bake();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	m_fbDescr.bake();

	// Prog
	ANKI_CHECK(getResourceManager().loadResource((getConfig().getRPreferCompute())
													 ? "Shaders/ShadowmapsResolveCompute.ankiprog"
													 : "Shaders/ShadowmapsResolveRaster.ankiprog",
												 m_prog));
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	if(getConfig().getRPreferCompute())
	{
		variantInitInfo.addConstant("FB_SIZE", UVec2(width, height));
	}
	variantInitInfo.addConstant("TILE_COUNTS", m_r->getTileCounts());
	variantInitInfo.addConstant("Z_SPLIT_COUNT", m_r->getZSplitCount());
	variantInitInfo.addConstant("TILE_SIZE", m_r->getTileSize());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	return Error::NONE;
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

		rpass.newDependency(RenderPassDependency(m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_WRITE));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(
			RenderPassDependency(m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::SAMPLED_COMPUTE));

		rpass.newDependency(
			RenderPassDependency(ctx.m_clusteredShading.m_clustersBufferHandle, BufferUsageBit::STORAGE_COMPUTE_READ));
	}
	else
	{
		GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("SM resolve");
		rpass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_rt}, {});

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			run(ctx, rgraphCtx);
		});

		rpass.newDependency(RenderPassDependency(m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_FRAGMENT));
		rpass.newDependency(
			RenderPassDependency(m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::SAMPLED_FRAGMENT));

		rpass.newDependency(
			RenderPassDependency(ctx.m_clusteredShading.m_clustersBufferHandle, BufferUsageBit::STORAGE_FRAGMENT_READ));
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
	rgraphCtx.bindTexture(0, 6, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

	if(getConfig().getRPreferCompute())
	{
		rgraphCtx.bindImage(0, 7, m_runCtx.m_rt, TextureSubresourceInfo());
		dispatchPPCompute(cmdb, 8, 8, m_rtDescr.m_width, m_rtDescr.m_height);
	}
	else
	{
		cmdb->setViewport(0, 0, m_rtDescr.m_width, m_rtDescr.m_height);
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
	}
}

} // end namespace anki
