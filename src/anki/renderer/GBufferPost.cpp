// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/GBufferPost.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Ssao.h>
#include <anki/renderer/LightShading.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

GBufferPost::~GBufferPost()
{
}

Error GBufferPost::init(const ConfigSet& cfg)
{
	Error err = initInternal(cfg);
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
	ANKI_CHECK(getResourceManager().loadResource("programs/GBufferPost.ankiprog", m_prog));

	ShaderProgramResourceConstantValueInitList<3> consts(m_prog);
	consts.add("CLUSTER_COUNT_X", U32(cfg.getNumber("r.clusterSizeX")));
	consts.add("CLUSTER_COUNT_Y", U32(cfg.getNumber("r.clusterSizeY")));
	consts.add("CLUSTER_COUNT_Z", U32(cfg.getNumber("r.clusterSizeZ")));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(consts.get(), variant);
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

	rpass.setWork(runCallback, this, 0);
	rpass.setFramebufferInfo(m_fbDescr, {{m_r->getGBuffer().getColorRt(0), m_r->getGBuffer().getColorRt(1)}}, {});

	rpass.newConsumer({m_r->getGBuffer().getColorRt(0), TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE});
	rpass.newConsumer({m_r->getGBuffer().getColorRt(1), TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE});
	rpass.newConsumer({m_r->getGBuffer().getDepthRt(),
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});

	rpass.newConsumer({m_r->getSsao().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});

	rpass.newProducer({m_r->getGBuffer().getColorRt(0), TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE});
	rpass.newProducer({m_r->getGBuffer().getColorRt(1), TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE});
}

void GBufferPost::run(RenderPassWorkContext& rgraphCtx)
{
	const LightShadingResources& rsrc = m_r->getLightShading().getResources();
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindShaderProgram(m_grProg);

	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::SRC_ALPHA, BlendFactor::ZERO, BlendFactor::ONE);
	cmdb->setBlendFactors(1, BlendFactor::ONE, BlendFactor::SRC_ALPHA, BlendFactor::ZERO, BlendFactor::ONE);

	// Bind textures
	rgraphCtx.bindTextureAndSampler(0,
		0,
		m_r->getGBuffer().getDepthRt(),
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
		m_r->getNearestSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getSsao().getRt(), m_r->getLinearSampler());
	cmdb->bindTextureAndSampler(0,
		2,
		(rsrc.m_diffDecalTexView) ? rsrc.m_diffDecalTexView : m_r->getDummyTextureView(),
		m_r->getTrilinearRepeatSampler(),
		TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->bindTextureAndSampler(0,
		3,
		(rsrc.m_specularRoughnessDecalTexView) ? rsrc.m_specularRoughnessDecalTexView : m_r->getDummyTextureView(),
		m_r->getTrilinearRepeatSampler(),
		TextureUsageBit::SAMPLED_FRAGMENT);

	// Uniforms
	bindUniforms(cmdb, 0, 0, rsrc.m_commonUniformsToken);
	bindUniforms(cmdb, 0, 1, rsrc.m_decalsToken);

	// Storage
	bindStorage(cmdb, 0, 0, rsrc.m_clustersToken);
	bindStorage(cmdb, 0, 1, rsrc.m_lightIndicesToken);

	cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setBlendFactors(1, BlendFactor::ONE, BlendFactor::ZERO);
}

} // end namespace anki
