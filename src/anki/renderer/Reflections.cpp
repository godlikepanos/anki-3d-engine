// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Reflections.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/RenderQueue.h>

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

	// Create RT descr
	m_rtDescr = m_r->create2DRenderTargetDescription(width,
		height,
		LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::IMAGE_COMPUTE_WRITE
			| TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		"Refl");
	m_rtDescr.bake();

	// Create shader
	ANKI_CHECK(getResourceManager().loadResource("programs/Reflections.ankiprog", m_prog));

	ShaderProgramResourceConstantValueInitList<4> consts(m_prog);
	consts.add("FB_SIZE", UVec2(width, height));
	consts.add("WORKGROUP_SIZE", UVec2(m_workgroupSize[0], m_workgroupSize[1]));
	consts.add("MAX_STEPS", U32(256));
	consts.add("LIGHT_BUFFER_MIP_COUNT", U32(m_r->getDownscaleBlur().getMipmapCount()));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(consts.get(), variant);
	m_grProg = variant->getProgram();

	return Error::NONE;
}

void Reflections::populateRenderGraph(RenderingContext& ctx)
{
	return; // TODO

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	// Create pass
	ComputeRenderPassDescription& rpass = ctx.m_renderGraphDescr.newComputeRenderPass("Refl");
	rpass.setWork(runCallback, this, 0);

	rpass.newConsumer({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_WRITE});
	rpass.newConsumer({m_r->getGBuffer().getColorRt(1), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newConsumer({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newConsumer({m_r->getDepthDownscale().getHalfDepthColorRt(), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newConsumer({m_r->getDownscaleBlur().getRt(), TextureUsageBit::SAMPLED_COMPUTE});

	rpass.newProducer({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_WRITE});
}

void Reflections::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	struct Unis
	{
		Mat4 m_projMat;
		Mat4 m_invViewProjMat;
		Mat4 m_invProjMat;
		Mat4 m_viewMat;
		Vec4 m_camPosNear;
	};

	Unis* unis = allocateAndBindUniforms<Unis*>(sizeof(Unis), cmdb, 0, 0);
	unis->m_projMat = m_runCtx.m_ctx->m_projMatJitter;
	unis->m_invViewProjMat = m_runCtx.m_ctx->m_viewProjMatJitter.getInverse();
	unis->m_invProjMat = m_runCtx.m_ctx->m_projMatJitter.getInverse();
	unis->m_viewMat = m_runCtx.m_ctx->m_renderQueue->m_viewMatrix;
	unis->m_camPosNear = Vec4(m_runCtx.m_ctx->m_renderQueue->m_cameraTransform.getTranslationPart().xyz(),
		m_runCtx.m_ctx->m_renderQueue->m_cameraNear + 0.1f);

	rgraphCtx.bindColorTextureAndSampler(0, 0, m_r->getGBuffer().getColorRt(1), m_r->getLinearSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getGBuffer().getColorRt(2), m_r->getLinearSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 2, m_r->getDepthDownscale().getHalfDepthColorRt(), m_r->getLinearSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 3, m_r->getDownscaleBlur().getRt(), m_r->getTrilinearRepeatSampler());

	TextureSubresourceInfo subresource;
	rgraphCtx.bindImage(0, 0, m_runCtx.m_rt, subresource);

	cmdb->bindShaderProgram(m_grProg);

	cmdb->dispatchCompute((m_r->getWidth() + m_workgroupSize[0] - 1) / m_workgroupSize[0],
		(m_r->getHeight() + m_workgroupSize[1] - 1) / m_workgroupSize[1],
		1);
}

} // end namespace anki
