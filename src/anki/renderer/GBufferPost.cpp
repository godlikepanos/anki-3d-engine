// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/GBufferPost.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/LightShading.h>
#include <anki/core/ConfigSet.h>

namespace anki
{

GBufferPost::~GBufferPost()
{
}

Error GBufferPost::init(const ConfigSet& cfg)
{
	const Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize GBufferPost pass");
	}
	return err;
}

Error GBufferPost::initInternal(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing GBufferPost pass");

	// Load shaders
	ANKI_CHECK(getResourceManager().loadResource("shaders/GBufferPost.ankiprog", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("CLUSTER_COUNT_X", cfg.getNumberU32("r_clusterSizeX"));
	variantInitInfo.addConstant("CLUSTER_COUNT_Y", cfg.getNumberU32("r_clusterSizeY"));

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
	m_runCtx.m_ctx = &ctx;

	// Create pass
	GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("GBuffPost");

	rpass.setWork(
		[](RenderPassWorkContext& rgraphCtx) { static_cast<GBufferPost*>(rgraphCtx.m_userData)->run(rgraphCtx); }, this,
		0);

	rpass.setFramebufferInfo(m_fbDescr, {m_r->getGBuffer().getColorRt(0), m_r->getGBuffer().getColorRt(1)}, {});

	rpass.newDependency({m_r->getGBuffer().getColorRt(0), TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT});
	rpass.newDependency({m_r->getGBuffer().getColorRt(1), TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT});
	rpass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_FRAGMENT,
						 TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});
}

void GBufferPost::run(RenderPassWorkContext& rgraphCtx)
{
	const RenderingContext& ctx = *m_runCtx.m_ctx;
	const ClusterBinOut& rsrc = ctx.m_clusterBinOut;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindShaderProgram(m_grProg);

	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::SRC_ALPHA, BlendFactor::ZERO, BlendFactor::ONE);
	cmdb->setBlendFactors(1, BlendFactor::ONE, BlendFactor::SRC_ALPHA, BlendFactor::ZERO, BlendFactor::ONE);

	// Bind all
	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);

	rgraphCtx.bindTexture(0, 1, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

	cmdb->bindSampler(0, 2, m_r->getSamplers().m_trilinearRepeat);

	bindUniforms(cmdb, 0, 3, ctx.m_lightShadingUniformsToken);
	bindUniforms(cmdb, 0, 4, rsrc.m_decalsToken);

	cmdb->bindTexture(0, 5, (rsrc.m_diffDecalTexView) ? rsrc.m_diffDecalTexView : m_r->getDummyTextureView2d(),
					  TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->bindTexture(0, 6,
					  (rsrc.m_specularRoughnessDecalTexView) ? rsrc.m_specularRoughnessDecalTexView
															 : m_r->getDummyTextureView2d(),
					  TextureUsageBit::SAMPLED_FRAGMENT);

	bindStorage(cmdb, 0, 7, rsrc.m_clustersToken);
	bindStorage(cmdb, 0, 8, rsrc.m_indicesToken);

	// Draw
	cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setBlendFactors(1, BlendFactor::ONE, BlendFactor::ZERO);
}

} // end namespace anki
