// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ssao.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/util/Functions.h>
#include <anki/core/ConfigSet.h>

namespace anki
{

Ssao::~Ssao()
{
}

Error Ssao::initMain(const ConfigSet& config)
{
	// Noise
	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_main.m_noiseTex));

	// Shader
	if(m_useCompute)
	{
		ANKI_CHECK(getResourceManager().loadResource("shaders/SsaoCompute.ankiprog", m_main.m_prog));
	}
	else
	{
		ANKI_CHECK(getResourceManager().loadResource("shaders/Ssao.ankiprog", m_main.m_prog));
	}

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_main.m_prog);
	variantInitInfo.addMutation("USE_NORMAL", (m_useNormal) ? 1u : 0u);
	variantInitInfo.addMutation("SOFT_BLUR", (m_useSoftBlur) ? 1u : 0u);
	variantInitInfo.addConstant("NOISE_MAP_SIZE", U32(m_main.m_noiseTex->getWidth()));
	variantInitInfo.addConstant("FB_SIZE", UVec2(m_width, m_height));
	variantInitInfo.addConstant("RADIUS", 2.5f);
	variantInitInfo.addConstant("BIAS", 0.0f);
	variantInitInfo.addConstant("STRENGTH", 2.5f);
	variantInitInfo.addConstant("SAMPLE_COUNT", 8u);
	const ShaderProgramResourceVariant* variant;
	m_main.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_main.m_workgroupSize[0] = variant->getWorkgroupSizes()[0];
	m_main.m_workgroupSize[1] = variant->getWorkgroupSizes()[1];
	m_main.m_grProg = variant->getProgram();

	return Error::NONE;
}

Error Ssao::initBlur(const ConfigSet& config)
{
	// shader
	if(m_blurUseCompute)
	{
		ANKI_CHECK(m_r->getResourceManager().loadResource("shaders/GaussianBlurCompute.ankiprog", m_blur.m_prog));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_blur.m_prog);
		variantInitInfo.addMutation("ORIENTATION", 2);
		variantInitInfo.addMutation("KERNEL_SIZE", 3);
		variantInitInfo.addMutation("COLOR_COMPONENTS", 1);
		variantInitInfo.addConstant("TEXTURE_SIZE", UVec2(m_width, m_height));

		const ShaderProgramResourceVariant* variant;
		m_blur.m_prog->getOrCreateVariant(variantInitInfo, variant);

		m_blur.m_workgroupSize[0] = variant->getWorkgroupSizes()[0];
		m_blur.m_workgroupSize[1] = variant->getWorkgroupSizes()[1];

		m_blur.m_grProg = variant->getProgram();
	}
	else
	{
		ANKI_CHECK(m_r->getResourceManager().loadResource("shaders/GaussianBlur.ankiprog", m_blur.m_prog));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_blur.m_prog);
		variantInitInfo.addMutation("ORIENTATION", 2);
		variantInitInfo.addMutation("KERNEL_SIZE", 3);
		variantInitInfo.addMutation("COLOR_COMPONENTS", 1);
		variantInitInfo.addConstant("TEXTURE_SIZE", UVec2(m_width, m_height));

		const ShaderProgramResourceVariant* variant;
		m_blur.m_prog->getOrCreateVariant(variantInitInfo, variant);

		m_blur.m_grProg = variant->getProgram();
	}

	return Error::NONE;
}

Error Ssao::init(const ConfigSet& config)
{
	m_width = m_r->getWidth() / SSAO_FRACTION;
	m_height = m_r->getHeight() / SSAO_FRACTION;

	ANKI_R_LOGI("Initializing SSAO. Size %ux%u", m_width, m_height);

	// RT
	m_rtDescrs[0] = m_r->create2DRenderTargetDescription(m_width, m_height, RT_PIXEL_FORMAT, "SSAOMain");
	m_rtDescrs[0].bake();

	m_rtDescrs[1] = m_r->create2DRenderTargetDescription(m_width, m_height, RT_PIXEL_FORMAT, "SSAOBlur");
	m_rtDescrs[1].bake();

	// FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fbDescr.bake();

	Error err = initMain(config);

	if(!err)
	{
		err = initBlur(config);
	}

	if(err)
	{
		ANKI_R_LOGE("Failed to init PPS SSAO");
	}

	return err;
}

void Ssao::runMain(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_main.m_grProg);

	// Bind resources
	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearRepeat);

	rgraphCtx.bindTexture(0, 2, m_r->getDepthDownscale().getHiZRt(), HIZ_HALF_DEPTH);
	cmdb->bindTexture(0, 3, m_main.m_noiseTex->getGrTextureView(), TextureUsageBit::SAMPLED_FRAGMENT);

	if(m_useNormal)
	{
		rgraphCtx.bindColorTexture(0, 4, m_r->getGBuffer().getColorRt(2));
	}

	struct Unis
	{
		Vec4 m_unprojectionParams;
		Vec4 m_projectionMat;
		Mat3x4 m_viewRotMat;
	} unis;

	const Mat4& pmat = ctx.m_renderQueue->m_projectionMatrix;
	unis.m_unprojectionParams = ctx.m_unprojParams;
	unis.m_projectionMat = Vec4(pmat(0, 0), pmat(1, 1), pmat(2, 2), pmat(2, 3));
	unis.m_viewRotMat = Mat3x4(Vec3(0.0f), ctx.m_renderQueue->m_viewMatrix.getRotationPart());
	cmdb->setPushConstants(&unis, sizeof(unis));

	if(m_useCompute)
	{
		rgraphCtx.bindImage(0, 5, m_runCtx.m_rts[0], TextureSubresourceInfo());
		dispatchPPCompute(cmdb, m_main.m_workgroupSize[0], m_main.m_workgroupSize[1], m_width, m_height);
	}
	else
	{
		cmdb->setViewport(0, 0, m_width, m_height);
		drawQuad(cmdb);
	}
}

void Ssao::runBlur(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_blur.m_grProg);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_rts[0]);

	if(m_blurUseCompute)
	{
		rgraphCtx.bindImage(0, 2, m_runCtx.m_rts[1], TextureSubresourceInfo());
		dispatchPPCompute(cmdb, m_blur.m_workgroupSize[0], m_blur.m_workgroupSize[1], m_width, m_height);
	}
	else
	{
		cmdb->setViewport(0, 0, m_width, m_height);
		drawQuad(cmdb);
	}
}

void Ssao::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create RTs
	m_runCtx.m_rts[0] = rgraph.newRenderTarget(m_rtDescrs[0]);
	m_runCtx.m_rts[1] = rgraph.newRenderTarget(m_rtDescrs[1]);

	// Create main render pass
	{
		if(m_useCompute)
		{
			ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("SSAO main");

			if(m_useNormal)
			{
				pass.newDependency({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE});
			}

			pass.newDependency({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_COMPUTE, HIZ_HALF_DEPTH});
			pass.newDependency({m_runCtx.m_rts[0], TextureUsageBit::IMAGE_COMPUTE_WRITE});

			pass.setWork(
				[](RenderPassWorkContext& rgraphCtx) {
					Ssao* const self = static_cast<Ssao*>(rgraphCtx.m_userData);
					self->runMain(*self->m_runCtx.m_ctx, rgraphCtx);
				},
				this, 0);
		}
		else
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("SSAO main");

			pass.setFramebufferInfo(m_fbDescr, {{m_runCtx.m_rts[0]}}, {});

			if(m_useNormal)
			{
				pass.newDependency({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_FRAGMENT});
			}

			pass.newDependency(
				{m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_FRAGMENT, HIZ_HALF_DEPTH});
			pass.newDependency({m_runCtx.m_rts[0], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

			pass.setWork(
				[](RenderPassWorkContext& rgraphCtx) {
					Ssao* const self = static_cast<Ssao*>(rgraphCtx.m_userData);
					self->runMain(*self->m_runCtx.m_ctx, rgraphCtx);
				},
				this, 0);
		}
	}

	// Create Blur pass
	{
		if(m_blurUseCompute)
		{
			ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("SSAO blur");

			pass.setWork(
				[](RenderPassWorkContext& rgraphCtx) {
					Ssao* const self = static_cast<Ssao*>(rgraphCtx.m_userData);
					self->runBlur(rgraphCtx);
				},
				this, 0);

			pass.newDependency({m_runCtx.m_rts[1], TextureUsageBit::IMAGE_COMPUTE_WRITE});
			pass.newDependency({m_runCtx.m_rts[0], TextureUsageBit::SAMPLED_COMPUTE});
		}
		else
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("SSAO blur");

			pass.setWork(
				[](RenderPassWorkContext& rgraphCtx) {
					Ssao* const self = static_cast<Ssao*>(rgraphCtx.m_userData);
					self->runBlur(rgraphCtx);
				},
				this, 0);
			pass.setFramebufferInfo(m_fbDescr, {{m_runCtx.m_rts[1]}}, {});

			pass.newDependency({m_runCtx.m_rts[1], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
			pass.newDependency({m_runCtx.m_rts[0], TextureUsageBit::SAMPLED_FRAGMENT});
		}
	}
}

} // end namespace anki
