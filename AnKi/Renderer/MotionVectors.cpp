// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

MotionVectors::~MotionVectors()
{
}

Error MotionVectors::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize motion vectors");
	}
	return err;
}

Error MotionVectors::initInternal()
{
	ANKI_R_LOGV("Initializing motion vectors");

	// Prog
	ANKI_CHECK(getResourceManager().loadResource((getConfig().getRPreferCompute())
													 ? "ShaderBinaries/MotionVectorsCompute.ankiprogbin"
													 : "ShaderBinaries/MotionVectorsRaster.ankiprogbin",
												 m_prog));
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("kFramebufferSize",
								UVec2(m_r->getInternalResolution().x(), m_r->getInternalResolution().y()));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	// RTs
	m_motionVectorsRtDescr = m_r->create2DRenderTargetDescription(
		m_r->getInternalResolution().x(), m_r->getInternalResolution().y(), Format::kR16G16_Sfloat, "MotionVectors");
	m_motionVectorsRtDescr.bake();

	TextureUsageBit historyLengthUsage = TextureUsageBit::kAllSampled;
	if(getConfig().getRPreferCompute())
	{
		historyLengthUsage |= TextureUsageBit::kImageComputeWrite;
	}
	else
	{
		historyLengthUsage |= TextureUsageBit::kFramebufferWrite;
	}

	TextureInitInfo historyLengthTexInit =
		m_r->create2DRenderTargetInitInfo(m_r->getInternalResolution().x(), m_r->getInternalResolution().y(),
										  Format::kR8_Unorm, historyLengthUsage, "MotionVectorsHistoryLen#1");
	m_historyLengthTextures[0] = m_r->createAndClearRenderTarget(historyLengthTexInit, TextureUsageBit::kAllSampled);
	historyLengthTexInit.setName("MotionVectorsHistoryLen#2");
	m_historyLengthTextures[1] = m_r->createAndClearRenderTarget(historyLengthTexInit, TextureUsageBit::kAllSampled);

	m_fbDescr.m_colorAttachmentCount = 2;
	m_fbDescr.bake();

	return Error::kNone;
}

void MotionVectors::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_motionVectorsRtHandle = rgraph.newRenderTarget(m_motionVectorsRtDescr);

	const U32 writeHistoryLenTexIdx = m_r->getFrameCount() & 1;
	const U32 readHistoryLenTexIdx = !writeHistoryLenTexIdx;

	if(ANKI_LIKELY(m_historyLengthTexturesImportedOnce))
	{
		m_runCtx.m_historyLengthWriteRtHandle =
			rgraph.importRenderTarget(m_historyLengthTextures[writeHistoryLenTexIdx]);
		m_runCtx.m_historyLengthReadRtHandle = rgraph.importRenderTarget(m_historyLengthTextures[readHistoryLenTexIdx]);
	}
	else
	{
		m_runCtx.m_historyLengthWriteRtHandle =
			rgraph.importRenderTarget(m_historyLengthTextures[writeHistoryLenTexIdx], TextureUsageBit::kAllSampled);
		m_runCtx.m_historyLengthReadRtHandle =
			rgraph.importRenderTarget(m_historyLengthTextures[readHistoryLenTexIdx], TextureUsageBit::kAllSampled);

		m_historyLengthTexturesImportedOnce = true;
	}

	RenderPassDescriptionBase* ppass;
	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	if(getConfig().getRPreferCompute())
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("MotionVectors");

		readUsage = TextureUsageBit::kSampledCompute;
		writeUsage = TextureUsageBit::kImageComputeWrite;
		ppass = &pass;
	}
	else
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("MotionVectors");
		pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_motionVectorsRtHandle, m_runCtx.m_historyLengthWriteRtHandle});

		readUsage = TextureUsageBit::kSampledFragment;
		writeUsage = TextureUsageBit::kFramebufferWrite;
		ppass = &pass;
	}

	ppass->setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) -> void {
		run(ctx, rgraphCtx);
	});

	ppass->newTextureDependency(m_runCtx.m_motionVectorsRtHandle, writeUsage);
	ppass->newTextureDependency(m_runCtx.m_historyLengthWriteRtHandle, writeUsage);
	ppass->newTextureDependency(m_runCtx.m_historyLengthReadRtHandle, readUsage);
	ppass->newTextureDependency(m_r->getGBuffer().getColorRt(3), readUsage);
	ppass->newTextureDependency(m_r->getGBuffer().getDepthRt(), readUsage);
	ppass->newTextureDependency(m_r->getGBuffer().getPreviousFrameDepthRt(), readUsage);
}

void MotionVectors::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grProg);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindTexture(0, 1, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
	rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getPreviousFrameDepthRt(),
						  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
	rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(3));
	rgraphCtx.bindColorTexture(0, 4, m_runCtx.m_historyLengthReadRtHandle);

	class Uniforms
	{
	public:
		Mat4 m_reprojectionMat;
		Mat4 m_viewProjectionInvMat;
		Mat4 m_prevViewProjectionInvMat;
	} * pc;
	pc = allocateAndBindUniforms<Uniforms*>(sizeof(*pc), cmdb, 0, 5);

	pc->m_reprojectionMat = ctx.m_matrices.m_reprojection;
	pc->m_viewProjectionInvMat = ctx.m_matrices.m_invertedViewProjectionJitter;
	pc->m_prevViewProjectionInvMat = ctx.m_prevMatrices.m_invertedViewProjectionJitter;

	if(getConfig().getRPreferCompute())
	{
		rgraphCtx.bindImage(0, 6, m_runCtx.m_motionVectorsRtHandle, TextureSubresourceInfo());
		rgraphCtx.bindImage(0, 7, m_runCtx.m_historyLengthWriteRtHandle, TextureSubresourceInfo());
	}

	if(getConfig().getRPreferCompute())
	{
		dispatchPPCompute(cmdb, 8, 8, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());
	}
	else
	{
		cmdb->setViewport(0, 0, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());

		cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
	}
}

} // end namespace anki
