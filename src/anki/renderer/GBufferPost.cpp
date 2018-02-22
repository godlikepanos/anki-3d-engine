// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/GBufferPost.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Ssao.h>
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
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	// TODO
}

} // end namespace anki