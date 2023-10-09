// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

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
	CString progFname =
		(g_preferComputeCVar.get()) ? "ShaderBinaries/MotionVectorsCompute.ankiprogbin" : "ShaderBinaries/MotionVectorsRaster.ankiprogbin";
	ANKI_CHECK(ResourceManager::getSingleton().loadResource(progFname, m_prog));
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("kFramebufferSize", UVec2(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y()));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg.reset(&variant->getProgram());

	// RTs
	m_motionVectorsRtDescr = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), Format::kR16G16_Sfloat, "MotionVectors");
	m_motionVectorsRtDescr.bake();

	TextureUsageBit historyLengthUsage = TextureUsageBit::kAllSampled;
	if(g_preferComputeCVar.get())
	{
		historyLengthUsage |= TextureUsageBit::kUavComputeWrite;
	}
	else
	{
		historyLengthUsage |= TextureUsageBit::kFramebufferWrite;
	}

	TextureInitInfo historyLengthTexInit =
		getRenderer().create2DRenderTargetInitInfo(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
												   Format::kR8_Unorm, historyLengthUsage, "MotionVectorsHistoryLen#1");
	m_historyLengthTextures[0] = getRenderer().createAndClearRenderTarget(historyLengthTexInit, TextureUsageBit::kAllSampled);
	historyLengthTexInit.setName("MotionVectorsHistoryLen#2");
	m_historyLengthTextures[1] = getRenderer().createAndClearRenderTarget(historyLengthTexInit, TextureUsageBit::kAllSampled);

	m_fbDescr.m_colorAttachmentCount = 2;
	m_fbDescr.bake();

	return Error::kNone;
}

void MotionVectors::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(MotionVectors);
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_motionVectorsRtHandle = rgraph.newRenderTarget(m_motionVectorsRtDescr);

	const U32 writeHistoryLenTexIdx = getRenderer().getFrameCount() & 1;
	const U32 readHistoryLenTexIdx = !writeHistoryLenTexIdx;

	if(m_historyLengthTexturesImportedOnce) [[likely]]
	{
		m_runCtx.m_historyLengthWriteRtHandle = rgraph.importRenderTarget(m_historyLengthTextures[writeHistoryLenTexIdx].get());
		m_runCtx.m_historyLengthReadRtHandle = rgraph.importRenderTarget(m_historyLengthTextures[readHistoryLenTexIdx].get());
	}
	else
	{
		m_runCtx.m_historyLengthWriteRtHandle =
			rgraph.importRenderTarget(m_historyLengthTextures[writeHistoryLenTexIdx].get(), TextureUsageBit::kAllSampled);
		m_runCtx.m_historyLengthReadRtHandle =
			rgraph.importRenderTarget(m_historyLengthTextures[readHistoryLenTexIdx].get(), TextureUsageBit::kAllSampled);

		m_historyLengthTexturesImportedOnce = true;
	}

	RenderPassDescriptionBase* ppass;
	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	if(g_preferComputeCVar.get())
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("MotionVectors");

		readUsage = TextureUsageBit::kSampledCompute;
		writeUsage = TextureUsageBit::kUavComputeWrite;
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
		ANKI_TRACE_SCOPED_EVENT(MotionVectors);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_grProg.get());

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindTexture(0, 1, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		rgraphCtx.bindTexture(0, 2, getRenderer().getGBuffer().getPreviousFrameDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		rgraphCtx.bindColorTexture(0, 3, getRenderer().getGBuffer().getColorRt(3));
		rgraphCtx.bindColorTexture(0, 4, m_runCtx.m_historyLengthReadRtHandle);

		class Constants
		{
		public:
			Mat4 m_reprojectionMat;
			Mat4 m_viewProjectionInvMat;
			Mat4 m_prevViewProjectionInvMat;
		} * pc;
		pc = allocateAndBindConstants<Constants>(cmdb, 0, 5);

		pc->m_reprojectionMat = ctx.m_matrices.m_reprojection;
		pc->m_viewProjectionInvMat = ctx.m_matrices.m_invertedViewProjectionJitter;
		pc->m_prevViewProjectionInvMat = ctx.m_prevMatrices.m_invertedViewProjectionJitter;

		if(g_preferComputeCVar.get())
		{
			rgraphCtx.bindUavTexture(0, 6, m_runCtx.m_motionVectorsRtHandle, TextureSubresourceInfo());
			rgraphCtx.bindUavTexture(0, 7, m_runCtx.m_historyLengthWriteRtHandle, TextureSubresourceInfo());
		}

		if(g_preferComputeCVar.get())
		{
			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		}
		else
		{
			cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());

			cmdb.draw(PrimitiveTopology::kTriangles, 3);
		}
	});

	ppass->newTextureDependency(m_runCtx.m_motionVectorsRtHandle, writeUsage);
	ppass->newTextureDependency(m_runCtx.m_historyLengthWriteRtHandle, writeUsage);
	ppass->newTextureDependency(m_runCtx.m_historyLengthReadRtHandle, readUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(3), readUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getDepthRt(), readUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getPreviousFrameDepthRt(), readUsage);
}

} // end namespace anki
