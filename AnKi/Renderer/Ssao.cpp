// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Ssao.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

static NumericCVar<U32> g_ssaoSampleCountCVar(CVarSubsystem::kRenderer, "SsaoSampleCount", 4, 1, 1024, "SSAO sample count");
static NumericCVar<F32> g_ssaoRadiusCVar(CVarSubsystem::kRenderer, "SsaoRadius", 2.0f, 0.1f, 100.0f, "SSAO radius in meters");
static BoolCVar g_ssaoQuarterRez(CVarSubsystem::kRenderer, "SsaoQuarterResolution", ANKI_PLATFORM_MOBILE, "Render SSAO in quarter rez");
static NumericCVar<F32> g_ssaoPower(CVarSubsystem::kRenderer, "SsaoPower", 3.0f, 0.1f, 100.0f, "SSAO power");

Error Ssao::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize SSAO");
	}
	return err;
}

Error Ssao::initInternal()
{
	const UVec2 rez = (g_ssaoQuarterRez.get()) ? getRenderer().getInternalResolution() / 2 : getRenderer().getInternalResolution();

	ANKI_R_LOGV("Initializing SSAO. Resolution %ux%u", rez.x(), rez.y());

	const Bool preferCompute = g_preferComputeCVar.get();

	{
		TextureUsageBit usage = TextureUsageBit::kAllSampled;
		usage |= (preferCompute) ? TextureUsageBit::kUavComputeWrite : TextureUsageBit::kFramebufferWrite;
		TextureInitInfo texInit = getRenderer().create2DRenderTargetInitInfo(rez.x(), rez.y(), Format::kR8_Unorm, usage, "SSAO #1");
		m_rts[0] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kAllSampled);
		texInit.setName("SSAO #2");
		m_rts[1] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kAllSampled);
	}

	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Ssao.ankiprogbin", Array<SubMutation, 1>{{{"SAMPLE_COUNT", 3}}}, m_prog, m_grProg, "Ssao"));

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Ssao.ankiprogbin", Array<SubMutation, 1>{{{"SAMPLE_COUNT", 5}}}, m_prog, m_denoiseGrProgs[0],
								 "SsaoDenoiseHorizontal"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Ssao.ankiprogbin", Array<SubMutation, 1>{{{"SAMPLE_COUNT", 5}}}, m_prog, m_denoiseGrProgs[1],
								 "SsaoDenoiseVertical"));

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	return Error::kNone;
}

void Ssao::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Ssao);
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_preferComputeCVar.get();

	const U32 readRtIdx = getRenderer().getFrameCount() & 1;
	const U32 writeRtIdx = !readRtIdx;
	if(m_rtsImportedOnce) [[likely]]
	{
		m_runCtx.m_ssaoRts[0] = rgraph.importRenderTarget(m_rts[readRtIdx].get());
		m_runCtx.m_ssaoRts[1] = rgraph.importRenderTarget(m_rts[writeRtIdx].get());
	}
	else
	{
		m_runCtx.m_ssaoRts[0] = rgraph.importRenderTarget(m_rts[readRtIdx].get(), TextureUsageBit::kAllSampled);
		m_runCtx.m_ssaoRts[1] = rgraph.importRenderTarget(m_rts[writeRtIdx].get(), TextureUsageBit::kAllSampled);
		m_rtsImportedOnce = true;
	}

	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	if(preferCompute)
	{
		readUsage = TextureUsageBit::kSampledCompute;
		writeUsage = TextureUsageBit::kUavComputeWrite;
	}
	else
	{
		readUsage = TextureUsageBit::kSampledFragment;
		writeUsage = TextureUsageBit::kFramebufferWrite;
	}

	// Main pass
	{
		RenderPassDescriptionBase* ppass;
		if(preferCompute)
		{
			ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("SSAO");
			ppass = &pass;
		}
		else
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("SSAO");
			pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_ssaoRts[1]}, {});
			ppass = &pass;
		}

		ppass->newTextureDependency(m_runCtx.m_ssaoRts[1], writeUsage);
		ppass->newTextureDependency(m_runCtx.m_ssaoRts[0], readUsage);
		ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(2), readUsage);
		ppass->newTextureDependency(getRenderer().getGBuffer().getDepthRt(), readUsage);
		ppass->newTextureDependency(getRenderer().getMotionVectors().getHistoryLengthRt(), readUsage);
		ppass->newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), readUsage);

		ppass->setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_grProg.get());

			rgraphCtx.bindColorTexture(0, 0, getRenderer().getGBuffer().getColorRt(2));
			rgraphCtx.bindTexture(0, 1, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

			cmdb.bindTexture(0, 2, &m_noiseImage->getTextureView());
			cmdb.bindSampler(0, 3, getRenderer().getSamplers().m_trilinearRepeat.get());
			cmdb.bindSampler(0, 4, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindColorTexture(0, 5, m_runCtx.m_ssaoRts[0]);
			rgraphCtx.bindColorTexture(0, 6, getRenderer().getMotionVectors().getMotionVectorsRt());
			rgraphCtx.bindColorTexture(0, 7, getRenderer().getMotionVectors().getHistoryLengthRt());

			const UVec2 rez = (g_ssaoQuarterRez.get()) ? getRenderer().getInternalResolution() / 2 : getRenderer().getInternalResolution();

			SsaoConstants consts;
			consts.m_radius = g_ssaoRadiusCVar.get();
			consts.m_sampleCount = g_ssaoSampleCountCVar.get();
			consts.m_viewportSizef = Vec2(rez);
			consts.m_unprojectionParameters = ctx.m_matrices.m_unprojectionParameters;
			consts.m_projectionMat00 = ctx.m_matrices.m_projection(0, 0);
			consts.m_projectionMat11 = ctx.m_matrices.m_projection(1, 1);
			consts.m_projectionMat22 = ctx.m_matrices.m_projection(2, 2);
			consts.m_projectionMat23 = ctx.m_matrices.m_projection(2, 3);
			consts.m_frameCount = getRenderer().getFrameCount() % kMaxU32;
			consts.m_ssaoPower = g_ssaoPower.get();
			consts.m_viewMat = ctx.m_matrices.m_view;
			consts.m_prevJitterUv = ctx.m_matrices.m_jitterOffsetNdc / 2.0f;
			cmdb.setPushConstants(&consts, sizeof(consts));

			if(g_preferComputeCVar.get())
			{
				rgraphCtx.bindUavTexture(0, 8, m_runCtx.m_ssaoRts[1]);

				dispatchPPCompute(cmdb, 8, 8, rez.x(), rez.y());
			}
			else
			{
				cmdb.setViewport(0, 0, rez.x(), rez.y());

				drawQuad(cmdb);
			}
		});
	}

	// Vertical and horizontal blur
	for(U32 dir = 0; dir < 2; ++dir)
	{
		RenderPassDescriptionBase* ppass;

		const U32 readRt = (dir == 0) ? 1 : 0;
		const U32 writeRt = !readRt;

		CString passName = (dir == 0) ? "SSAO vert blur" : "SSAO horiz blur";
		if(preferCompute)
		{
			ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(passName);
			ppass = &pass;
		}
		else
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passName);
			pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_ssaoRts[writeRt]}, {});
			ppass = &pass;
		}

		ppass->newTextureDependency(m_runCtx.m_ssaoRts[readRt], readUsage);
		ppass->newTextureDependency(m_runCtx.m_ssaoRts[writeRt], writeUsage);
		if(g_ssaoQuarterRez.get())
		{
			ppass->newTextureDependency(getRenderer().getDepthDownscale().getRt(), readUsage);
		}
		else
		{
			ppass->newTextureDependency(getRenderer().getGBuffer().getDepthRt(), readUsage);
		}

		ppass->setWork([this, dir, readRt](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_denoiseGrProgs[dir].get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_ssaoRts[readRt]);
			if(g_ssaoQuarterRez.get())
			{
				rgraphCtx.bindColorTexture(0, 2, getRenderer().getDepthDownscale().getRt());
			}
			else
			{
				rgraphCtx.bindTexture(0, 2, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
			}

			const UVec2 rez = (g_ssaoQuarterRez.get()) ? getRenderer().getInternalResolution() / 2 : getRenderer().getInternalResolution();

			if(g_preferComputeCVar.get())
			{
				rgraphCtx.bindUavTexture(0, 3, m_runCtx.m_ssaoRts[!readRt]);

				dispatchPPCompute(cmdb, 8, 8, rez.x(), rez.y());
			}
			else
			{
				cmdb.setViewport(0, 0, rez.x(), rez.y());

				drawQuad(cmdb);
			}
		});
	}
}

} // end namespace anki
