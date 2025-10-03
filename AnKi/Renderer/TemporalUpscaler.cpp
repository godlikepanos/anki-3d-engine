// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/TemporalUpscaler.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/TemporalAA.h>
#include <AnKi/Renderer/Tonemapping.h>

#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error TemporalUpscaler::init()
{
	const Bool needsScaling = getRenderer().getPostProcessResolution() != getRenderer().getInternalResolution();

	if(!needsScaling)
	{
		return Error::kNone;
	}

	if(GrManager::getSingleton().getDeviceCapabilities().m_dlss && g_cvarRenderDlss)
	{

		GrUpscalerInitInfo inf;
		inf.m_sourceTextureResolution = getRenderer().getInternalResolution();
		inf.m_targetTextureResolution = getRenderer().getPostProcessResolution();
		inf.m_upscalerType = GrUpscalerType::kDlss2;
		inf.m_qualityMode = GrUpscalerQualityMode(U32(g_cvarRenderDlssQuality));

		m_grUpscaler = GrManager::getSingleton().newGrUpscaler(inf);
	}
	else
	{
		ANKI_R_LOGW("No support of temporal upscaler. Will just blit instead");

		ANKI_CHECK(loadShaderProgram("ShaderBinaries/Blit.ankiprogbin", m_blitProg, m_blitGrProg));
	}

	m_rtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getPostProcessResolution().x(),
															 getRenderer().getPostProcessResolution().y(), getRenderer().getHdrFormat(), "Upscaled");
	m_rtDesc.bake();

	return Error::kNone;
}

void TemporalUpscaler::populateRenderGraph(RenderingContext& ctx)
{

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDesc);

	if(m_grUpscaler.isCreated())
	{
		NonGraphicsRenderPass& pass = ctx.m_renderGraphDescr.newNonGraphicsRenderPass("DLSS");

		// DLSS says input textures in sampled state and out as storage image
		const TextureUsageBit readUsage = TextureUsageBit::kAllSrv & TextureUsageBit::kAllCompute;
		const TextureUsageBit writeUsage = TextureUsageBit::kAllUav & TextureUsageBit::kAllCompute;

		pass.newTextureDependency(getRenderer().getLightShading().getRt(), readUsage);
		pass.newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), readUsage);
		pass.newTextureDependency(getGBuffer().getDepthRt(), readUsage, TextureSubresourceDesc::firstSurface(DepthStencilAspectBit::kDepth));
		pass.newTextureDependency(m_runCtx.m_rt, writeUsage);

		pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(TemporalUpscaler);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			const Vec2 srcRes(getRenderer().getInternalResolution());
			const Bool reset = getRenderer().getFrameCount() == 0;
			const Vec2 mvScale = srcRes; // UV space to Pixel space factor
			// In [-texSize / 2, texSize / 2] -> sub-pixel space {-0.5, 0.5}
			const Vec2 jitterOffset = ctx.m_matrices.m_jitter.getTranslationPart().xy() * srcRes * 0.5f;

			const TextureView srcView = rgraphCtx.createTextureView(getRenderer().getLightShading().getRt(), TextureSubresourceDesc::firstSurface());
			const TextureView motionVectorsView =
				rgraphCtx.createTextureView(getRenderer().getMotionVectors().getMotionVectorsRt(), TextureSubresourceDesc::firstSurface());
			const TextureView depthView =
				rgraphCtx.createTextureView(getGBuffer().getDepthRt(), TextureSubresourceDesc::firstSurface(DepthStencilAspectBit::kDepth));
			const TextureView exposureView =
				rgraphCtx.createTextureView(getRenderer().getTonemapping().getExposureAndAvgLuminanceRt(), TextureSubresourceDesc::firstSurface());
			const TextureView dstView = rgraphCtx.createTextureView(m_runCtx.m_rt, TextureSubresourceDesc::firstSurface());

			cmdb.upscale(m_grUpscaler.get(), srcView, dstView, motionVectorsView, depthView, exposureView, reset, jitterOffset, mvScale);
		});
	}
	else
	{
		const Bool preferCompute = g_cvarRenderPreferCompute;

		TextureUsageBit readUsage;
		TextureUsageBit writeUsage;
		if(preferCompute)
		{
			readUsage = TextureUsageBit::kSrvCompute;
			writeUsage = TextureUsageBit::kUavCompute;
		}
		else
		{
			readUsage = TextureUsageBit::kSrvPixel;
			writeUsage = TextureUsageBit::kRtvDsvWrite;
		}

		RenderPassBase* ppass;
		if(preferCompute)
		{
			NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Upscaling");
			ppass = &pass;
		}
		else
		{
			GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("Upscaling");
			pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(m_runCtx.m_rt)});
			ppass = &pass;
		}

		ppass->newTextureDependency(getRenderer().getLightShading().getRt(), readUsage);
		ppass->newTextureDependency(m_runCtx.m_rt, writeUsage);

		ppass->setWork([this](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(TemporalUpscaler);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			rgraphCtx.bindSrv(0, 0, getRenderer().getLightShading().getRt());
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			const Bool preferCompute = g_cvarRenderPreferCompute;

			if(preferCompute)
			{
				rgraphCtx.bindUav(0, 0, m_runCtx.m_rt);

				dispatchPPCompute(cmdb, 8, 8, getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y());
			}
			else
			{
				drawQuad(cmdb);
			}
		});
	}
}

} // end namespace anki
