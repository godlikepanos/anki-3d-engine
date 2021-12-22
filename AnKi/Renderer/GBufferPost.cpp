// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/GBufferPost.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/LightShading.h>

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
	ANKI_R_LOGI("Initializing GBufferPost pass");

	// Load shaders
	ANKI_CHECK(getResourceManager().loadResource("Shaders/GBufferPost.ankiprog", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("TILE_COUNTS", m_r->getTileCounts());
	variantInitInfo.addConstant("Z_SPLIT_COUNT", m_r->getZSplitCount());
	variantInitInfo.addConstant("TILE_SIZE", m_r->getTileSize());

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 2;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::LOAD;
	m_fbDescr.m_colorAttachments[1].m_loadOperation = AttachmentLoadOperation::LOAD;
	m_fbDescr.bake();

	return Error::NONE;
}

void GBufferPost::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create pass
	GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("GBuffPost");

	rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		run(ctx, rgraphCtx);
	});

	rpass.setFramebufferInfo(m_fbDescr, {m_r->getGBuffer().getColorRt(0), m_r->getGBuffer().getColorRt(1)});

	rpass.newDependency(
		RenderPassDependency(m_r->getGBuffer().getColorRt(0), TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT));
	rpass.newDependency(
		RenderPassDependency(m_r->getGBuffer().getColorRt(1), TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT));
	rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_FRAGMENT,
											 TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)));
	rpass.newDependency(
		RenderPassDependency(ctx.m_clusteredShading.m_clustersBufferHandle, BufferUsageBit::STORAGE_FRAGMENT_READ));
}

void GBufferPost::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	const ClusteredShadingContext& rsrc = ctx.m_clusteredShading;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());
	cmdb->bindShaderProgram(m_grProg);

	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::SRC_ALPHA, BlendFactor::ZERO, BlendFactor::ONE);
	cmdb->setBlendFactors(1, BlendFactor::ONE, BlendFactor::SRC_ALPHA, BlendFactor::ZERO, BlendFactor::ONE);

	// Bind all
	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);

	rgraphCtx.bindTexture(0, 1, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

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
	cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setBlendFactors(1, BlendFactor::ONE, BlendFactor::ZERO);
}

} // end namespace anki
