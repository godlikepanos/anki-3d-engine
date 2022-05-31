// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectSpecular.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

IndirectSpecular::~IndirectSpecular()
{
}

Error IndirectSpecular::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize indirect specular pass");
	}
	return err;
}

Error IndirectSpecular::initInternal()
{
	const U32 width = m_r->getInternalResolution().x() / 2;
	const U32 height = m_r->getInternalResolution().y() / 2;
	const Bool preferCompute = getConfig().getRPreferCompute();

	ANKI_R_LOGV("Initializing indirect specular. Resolution %ux%u", width, height);

	ANKI_CHECK(getResourceManager().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	// Create RT
	TextureUsageBit usage = TextureUsageBit::ALL_SAMPLED;

	usage |= (preferCompute) ? TextureUsageBit::IMAGE_COMPUTE_WRITE : TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;

	TextureInitInfo texInit = m_r->create2DRenderTargetInitInfo(width, height, m_r->getHdrFormat(), usage, "SSR #1");
	m_rts[0] = m_r->createAndClearRenderTarget(texInit, TextureUsageBit::ALL_SAMPLED);
	texInit.setName("SSR #2");
	m_rts[1] = m_r->createAndClearRenderTarget(texInit, TextureUsageBit::ALL_SAMPLED);

	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	// Create shader
	ANKI_CHECK(getResourceManager().loadResource((getConfig().getRPreferCompute())
													 ? "ShaderBinaries/IndirectSpecularCompute.ankiprogbin"
													 : "ShaderBinaries/IndirectSpecularRaster.ankiprogbin",
												 m_prog));

	ShaderProgramResourceVariantInitInfo variantInit(m_prog);
	variantInit.addMutation("EXTRA_REJECTION", false);
	variantInit.addMutation("STOCHASTIC", getConfig().getRSsrStochastic());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInit, variant);
	m_grProg = variant->getProgram();

	return Error::NONE;
}

void IndirectSpecular::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = getConfig().getRPreferCompute();

	// Create/import RTs
	const U32 readRtIdx = m_r->getFrameCount() & 1;
	const U32 writeRtIdx = !readRtIdx;
	if(ANKI_LIKELY(m_rtsImportedOnce))
	{
		m_runCtx.m_rts[0] = rgraph.importRenderTarget(m_rts[readRtIdx]);
		m_runCtx.m_rts[1] = rgraph.importRenderTarget(m_rts[writeRtIdx]);
	}
	else
	{
		m_runCtx.m_rts[0] = rgraph.importRenderTarget(m_rts[readRtIdx], TextureUsageBit::ALL_SAMPLED);
		m_runCtx.m_rts[1] = rgraph.importRenderTarget(m_rts[writeRtIdx], TextureUsageBit::ALL_SAMPLED);
		m_rtsImportedOnce = true;
	}

	// Create pass
	RenderPassDescriptionBase* ppass;
	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	if(preferCompute)
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("SSR");

		ppass = &pass;
		readUsage = TextureUsageBit::SAMPLED_COMPUTE;
		writeUsage = TextureUsageBit::IMAGE_COMPUTE_WRITE;
	}
	else
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("SSR");
		pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_rts[WRITE]});

		ppass = &pass;
		readUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		writeUsage = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
	}

	ppass->newDependency(RenderPassDependency(m_runCtx.m_rts[WRITE], writeUsage));
	ppass->newDependency(RenderPassDependency(m_runCtx.m_rts[READ], readUsage));
	ppass->newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(1), readUsage));
	ppass->newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(2), readUsage));

	TextureSubresourceInfo hizSubresource;
	hizSubresource.m_mipmapCount = min(getConfig().getRSsrDepthLod() + 1, m_r->getDepthDownscale().getMipmapCount());
	ppass->newDependency(RenderPassDependency(m_r->getDepthDownscale().getHiZRt(), readUsage, hizSubresource));

	ppass->newDependency(RenderPassDependency(m_r->getProbeReflections().getReflectionRt(), readUsage));

	ppass->newDependency(RenderPassDependency(m_r->getMotionVectors().getMotionVectorsRt(), readUsage));
	ppass->newDependency(RenderPassDependency(m_r->getMotionVectors().getHistoryLengthRt(), readUsage));

	ppass->setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		run(ctx, rgraphCtx);
	});
}

void IndirectSpecular::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_grProg);

	const U32 depthLod = min(getConfig().getRSsrDepthLod(), m_r->getDepthDownscale().getMipmapCount() - 1);

	// Bind uniforms
	SsrUniforms* unis = allocateAndBindUniforms<SsrUniforms*>(sizeof(SsrUniforms), cmdb, 0, 0);
	unis->m_depthBufferSize = m_r->getInternalResolution() >> (depthLod + 1);
	unis->m_framebufferSize = UVec2(m_r->getInternalResolution().x(), m_r->getInternalResolution().y()) / 2;
	unis->m_frameCount = m_r->getFrameCount() & MAX_U32;
	unis->m_depthMipCount = m_r->getDepthDownscale().getMipmapCount();
	unis->m_maxSteps = getConfig().getRSsrMaxSteps();
	unis->m_lightBufferMipCount = m_r->getDownscaleBlur().getMipmapCount();
	unis->m_firstStepPixels = getConfig().getRSsrFirstStepPixels();
	unis->m_prevViewProjMatMulInvViewProjMat =
		ctx.m_prevMatrices.m_viewProjection * ctx.m_matrices.m_viewProjectionJitter.getInverse();
	unis->m_projMat = ctx.m_matrices.m_projectionJitter;
	unis->m_invProjMat = ctx.m_matrices.m_projectionJitter.getInverse();
	unis->m_normalMat = Mat3x4(Vec3(0.0f), ctx.m_matrices.m_view.getRotationPart());
	unis->m_roughnessCutoff = getConfig().getRSsrRoughnessCutoff();

	// Bind all
	cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);

	rgraphCtx.bindColorTexture(0, 2, m_r->getGBuffer().getColorRt(1));
	rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(2));

	TextureSubresourceInfo hizSubresource;
	hizSubresource.m_mipmapCount = depthLod + 1;
	rgraphCtx.bindTexture(0, 4, m_r->getDepthDownscale().getHiZRt(), hizSubresource);

	rgraphCtx.bindColorTexture(0, 5, m_r->getDownscaleBlur().getRt());

	rgraphCtx.bindColorTexture(0, 6, m_runCtx.m_rts[READ]);
	rgraphCtx.bindColorTexture(0, 7, m_r->getMotionVectors().getMotionVectorsRt());
	rgraphCtx.bindColorTexture(0, 8, m_r->getMotionVectors().getHistoryLengthRt());

	cmdb->bindSampler(0, 9, m_r->getSamplers().m_trilinearRepeat);
	cmdb->bindTexture(0, 10, m_noiseImage->getTextureView());

	const ClusteredShadingContext& binning = ctx.m_clusteredShading;
	bindUniforms(cmdb, 0, 11, binning.m_clusteredShadingUniformsToken);
	bindUniforms(cmdb, 0, 12, binning.m_reflectionProbesToken);
	rgraphCtx.bindColorTexture(0, 13, m_r->getProbeReflections().getReflectionRt());
	bindStorage(cmdb, 0, 14, binning.m_clustersToken);

	if(getConfig().getRPreferCompute())
	{
		rgraphCtx.bindImage(0, 15, m_runCtx.m_rts[WRITE], TextureSubresourceInfo());

		dispatchPPCompute(cmdb, 8, 8, m_r->getInternalResolution().x() / 2, m_r->getInternalResolution().y() / 2);
	}
	else
	{
		cmdb->setViewport(0, 0, m_r->getInternalResolution().x() / 2, m_r->getInternalResolution().y() / 2);

		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
	}
}

} // end namespace anki
