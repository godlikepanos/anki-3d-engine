// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/GBufferPost.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

GBufferPost::~GBufferPost()
{
}

Error GBufferPost::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize GBufferPost pass");
	}
	return err;
}

Error GBufferPost::initInternal()
{
	ANKI_R_LOGV("Initializing GBufferPost pass");

	// Load shaders
	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/GBufferPost.ankiprogbin", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("kTileCount", m_r->getTileCounts());
	variantInitInfo.addConstant("kZSplitCount", m_r->getZSplitCount());
	variantInitInfo.addConstant("kTileSize", m_r->getTileSize());

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 2;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::kLoad;
	m_fbDescr.m_colorAttachments[1].m_loadOperation = AttachmentLoadOperation::kLoad;
	m_fbDescr.bake();

	return Error::kNone;
}

void GBufferPost::populateRenderGraph(RenderingContext& ctx)
{
	if(ctx.m_renderQueue->m_decals.getSize() == 0)
	{
		// If there are no decals don't bother
		return;
	}

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create pass
	GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("GBuffPost");

	rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		run(ctx, rgraphCtx);
	});

	rpass.setFramebufferInfo(m_fbDescr, {m_r->getGBuffer().getColorRt(0), m_r->getGBuffer().getColorRt(1)});

	rpass.newTextureDependency(m_r->getGBuffer().getColorRt(0), TextureUsageBit::kAllFramebuffer);
	rpass.newTextureDependency(m_r->getGBuffer().getColorRt(1), TextureUsageBit::kAllFramebuffer);
	rpass.newTextureDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::kSampledFragment,
							   TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
	rpass.newBufferDependency(ctx.m_clusteredShading.m_clustersBufferHandle, BufferUsageBit::kStorageFragmentRead);
}

void GBufferPost::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	const ClusteredShadingContext& rsrc = ctx.m_clusteredShading;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());
	cmdb->bindShaderProgram(m_grProg);

	cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kSrcAlpha, BlendFactor::kZero, BlendFactor::kOne);
	cmdb->setBlendFactors(1, BlendFactor::kOne, BlendFactor::kSrcAlpha, BlendFactor::kZero, BlendFactor::kOne);

	// Bind all
	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);

	rgraphCtx.bindTexture(0, 1, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

	cmdb->bindSampler(0, 2, m_r->getSamplers().m_trilinearRepeat);

	bindUniforms(cmdb, 0, 3, rsrc.m_clusteredShadingUniformsToken);
	bindUniforms(cmdb, 0, 4, rsrc.m_decalsToken);

	cmdb->bindTexture(0, 5,
					  (rsrc.m_diffuseDecalTextureView) ? rsrc.m_diffuseDecalTextureView : m_r->getDummyTextureView2d());
	cmdb->bindTexture(0, 6,
					  (rsrc.m_specularRoughnessDecalTextureView) ? rsrc.m_specularRoughnessDecalTextureView
																 : m_r->getDummyTextureView2d());

	bindStorage(cmdb, 0, 7, rsrc.m_clustersToken);

	// Draw
	cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	cmdb->setBlendFactors(1, BlendFactor::kOne, BlendFactor::kZero);
}

} // end namespace anki
