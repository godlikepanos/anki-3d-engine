// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Reflections.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Indirect.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/LightShading.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

Reflections::~Reflections()
{
}

Error Reflections::init(const ConfigSet& cfg)
{
	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize reflection pass");
	}
	return err;
}

Error Reflections::initInternal(const ConfigSet& cfg)
{
	U32 width = m_r->getWidth();
	U32 height = m_r->getHeight();
	ANKI_R_LOGI("Initializing reflection pass (%ux%u)", width, height);

	// Create RTs
	m_rtDescr = m_r->create2DRenderTargetDescription(width,
		height,
		LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::IMAGE_COMPUTE_WRITE,
		"IndirectRes");
	m_rtDescr.bake();

	// Create shader
	ANKI_CHECK(getResourceManager().loadResource("programs/Reflections.ankiprog", m_prog));

	ShaderProgramResourceConstantValueInitList<9> consts(m_prog);
	consts.add("FB_SIZE", UVec2(width, height));
	consts.add("WORKGROUP_SIZE", UVec2(m_workgroupSize[0], m_workgroupSize[1]));
	consts.add("MAX_STEPS", U32(64));
	consts.add("LIGHT_BUFFER_MIP_COUNT", U32(m_r->getDownscaleBlur().getMipmapCount()));
	consts.add("HIZ_MIP_COUNT", U32(HIERARCHICAL_Z_MIPMAP_COUNT));
	consts.add("CLUSTER_COUNT_X", U32(cfg.getNumber("r.clusterSizeX")));
	consts.add("CLUSTER_COUNT_Y", U32(cfg.getNumber("r.clusterSizeY")));
	consts.add("CLUSTER_COUNT_Z", U32(cfg.getNumber("r.clusterSizeZ")));
	consts.add("IR_MIPMAP_COUNT", U32(m_r->getIndirect().getReflectionTextureMipmapCount()));

	ShaderProgramResourceMutationInitList<1> mutations(m_prog);
	mutations.add("VARIANT", 0);

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutations.get(), consts.get(), variant);
	m_grProg[0] = variant->getProgram();

	mutations[0].m_value = 1;
	m_prog->getOrCreateVariant(mutations.get(), consts.get(), variant);
	m_grProg[1] = variant->getProgram();

	return Error::NONE;
}

void Reflections::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;

	// Create RTs
	m_runCtx.m_indirectRt = rgraph.newRenderTarget(m_rtDescr);

	// Create pass
	ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("IndirectRes");
	rpass.setWork(runCallback, this, 0);

	rpass.newConsumer({m_runCtx.m_indirectRt, TextureUsageBit::IMAGE_COMPUTE_WRITE});
	rpass.newConsumer({m_r->getGBuffer().getColorRt(0), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newConsumer({m_r->getGBuffer().getColorRt(1), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newConsumer({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newConsumer({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newConsumer({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newConsumer({m_r->getDownscaleBlur().getRt(), TextureUsageBit::SAMPLED_COMPUTE});

	rpass.newConsumer({m_r->getIndirect().getReflectionRt(), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newConsumer({m_r->getIndirect().getIrradianceRt(), TextureUsageBit::SAMPLED_COMPUTE});

	rpass.newProducer({m_runCtx.m_indirectRt, TextureUsageBit::IMAGE_COMPUTE_WRITE});
}

void Reflections::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_grProg[m_r->getFrameCount() & 1]);

	// Bind textures
	rgraphCtx.bindColorTextureAndSampler(0, 0, m_r->getGBuffer().getColorRt(0), m_r->getNearestSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getGBuffer().getColorRt(1), m_r->getNearestSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 2, m_r->getGBuffer().getColorRt(2), m_r->getNearestSampler());
	rgraphCtx.bindTextureAndSampler(0,
		3,
		m_r->getGBuffer().getDepthRt(),
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
		m_r->getNearestSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 4, m_r->getDepthDownscale().getHiZRt(), m_r->getNearestNearestSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 5, m_r->getDownscaleBlur().getRt(), m_r->getTrilinearRepeatSampler());

	rgraphCtx.bindColorTextureAndSampler(0, 6, m_r->getIndirect().getReflectionRt(), m_r->getTrilinearRepeatSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 7, m_r->getIndirect().getIrradianceRt(), m_r->getTrilinearRepeatSampler());
	cmdb->bindTextureAndSampler(0,
		8,
		m_r->getIndirect().getIntegrationLut(),
		m_r->getIndirect().getIntegrationLutSampler(),
		TextureUsageBit::SAMPLED_COMPUTE);

	// Bind image
	rgraphCtx.bindImage(0, 0, m_runCtx.m_indirectRt, TextureSubresourceInfo());

	// Bind uniforms
	const LightShadingResources& rsrc = m_r->getLightShading().getResources();
	bindUniforms(cmdb, 0, 0, rsrc.m_commonUniformsToken);
	bindUniforms(cmdb, 0, 1, rsrc.m_probesToken);
	bindStorage(cmdb, 0, 0, rsrc.m_clustersToken);
	bindStorage(cmdb, 0, 1, rsrc.m_lightIndicesToken);

	// Dispatch
	const U sizeX = (m_r->getWidth() + m_workgroupSize[0] - 1) / m_workgroupSize[0];
	const U sizeY = (m_r->getHeight() + m_workgroupSize[1] - 1) / m_workgroupSize[1];
	cmdb->dispatchCompute(sizeX / 2, sizeY, 1);
}

} // end namespace anki
