// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Ssao.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/HistoryLength.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error Ssao::init()
{
	const UVec2 rez = getRenderer().getInternalResolution();
	const Bool preferCompute = g_cvarRenderPreferCompute;

	{
		TextureUsageBit usage = TextureUsageBit::kAllSrv;
		usage |= (preferCompute) ? TextureUsageBit::kUavCompute : TextureUsageBit::kRtvDsvWrite;
		TextureInitInfo texInit = getRenderer().create2DRenderTargetInitInfo(rez.x, rez.y, Format::kR8G8B8A8_Snorm, usage, "Bent normals + SSAO #1");
		m_tex[0] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kAllSrv);

		texInit.setName("Bent normals + SSAO #2");
		m_tex[1] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kAllSrv);
	}

	m_bentNormalsAndSsaoRtDescr = getRenderer().create2DRenderTargetDescription(rez.x, rez.y, Format::kR8G8B8A8_Snorm, "Bent normals + SSAO temp");
	m_bentNormalsAndSsaoRtDescr.bake();

	// Use 16bit only on Arm because that path increases the instruction count on AMD
	Array<SubMutation, 2> mutation = {{{"SSAO_QUARTER_REZ", MutatorValue(g_cvarRenderSsaoQuarterRez)},
									   {"SSAO_16BIT", GrManager::getSingleton().getDeviceCapabilities().m_gpuVendor == GpuVendor::kArm}}};

	ANKI_CHECK(m_ssaoGrProg.load("ShaderBinaries/Ssao.ankiprogbin", mutation, "Ssao"));
	ANKI_CHECK(m_spatialDenoiseGrProg.load("ShaderBinaries/Ssao.ankiprogbin", mutation, "SsaoSpatialDenoise"));

	for(MutatorValue quarterRez = 0; quarterRez < 2; ++quarterRez)
	{
		mutation[0].m_value = quarterRez;
		ANKI_CHECK(m_tempralDenoiseGrProgs[quarterRez].load("ShaderBinaries/Ssao.ankiprogbin", mutation, "SsaoTemporalDenoise"));
	}

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	return Error::kNone;
}

void Ssao::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(Ssao);
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;
	const Bool preferCompute = g_cvarRenderPreferCompute;

	const U32 readRtIdx = getRenderer().getFrameCount() & 1;
	const U32 writeRtIdx = !readRtIdx;

	RenderTargetHandle historyRt;
	RenderTargetHandle finalRt;

	if(m_texImportedOnce) [[likely]]
	{
		finalRt = rgraph.importRenderTarget(m_tex[writeRtIdx].get());
		historyRt = rgraph.importRenderTarget(m_tex[readRtIdx].get());
	}
	else
	{
		finalRt = rgraph.importRenderTarget(m_tex[writeRtIdx].get(), TextureUsageBit::kAllSrv);
		historyRt = rgraph.importRenderTarget(m_tex[readRtIdx].get(), TextureUsageBit::kAllSrv);
		m_texImportedOnce = true;
	}

	m_runCtx.m_finalRt = finalRt;

	const RenderTargetHandle bentNormalsAndSsaoTempRt = rgraph.newRenderTarget(m_bentNormalsAndSsaoRtDescr);

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

	// Main pass
	{
		RenderPassBase* ppass;
		if(preferCompute)
		{
			NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("SSAO");
			ppass = &pass;
		}
		else
		{
			GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("SSAO");
			pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(finalRt)});
			ppass = &pass;
		}

		ppass->newTextureDependency(getGBuffer().getColorRt(2), readUsage);
		ppass->newTextureDependency(getGBuffer().getDepthRt(), readUsage);
		ppass->newTextureDependency(finalRt, writeUsage);

		ppass->setWork([this, finalRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SsaoMain);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			RenderingContext& ctx = getRenderingContext();

			cmdb.bindShaderProgram(m_ssaoGrProg.get());

			rgraphCtx.bindSrv(0, 0, getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(1, 0, getGBuffer().getDepthRt());

			cmdb.bindSrv(2, 0, TextureView(&m_noiseImage->getTexture(), TextureSubresourceDesc::all()));
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
			cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			const UVec2 rez = (g_cvarRenderSsaoQuarterRez) ? getRenderer().getInternalResolution() / 2 : getRenderer().getInternalResolution();

			SsaoConstants& consts = *allocateAndBindConstants<SsaoConstants>(cmdb, 0, 0);
			consts.m_radius = g_cvarRenderSsaoRadius;
			consts.m_stepCountf = F32(g_cvarRenderSsaoStepCount);
			consts.m_sliceCountf = F32(g_cvarRenderSsaoSliceCount);
			consts.m_viewportSizef = Vec2(rez);
			consts.m_unprojectionParameters = ctx.m_matrices.m_unprojectionParameters;
			consts.m_projectionMat00 = ctx.m_matrices.m_projection(0, 0);
			consts.m_projectionMat11 = ctx.m_matrices.m_projection(1, 1);
			consts.m_projectionMat22 = ctx.m_matrices.m_projection(2, 2);
			consts.m_projectionMat23 = ctx.m_matrices.m_projection(2, 3);
			consts.m_frameCount = getRenderer().getFrameCount() % kMaxU32;
			consts.m_ssaoPower = g_cvarRenderSsaoPower;
			consts.m_viewMat = ctx.m_matrices.m_view;
			consts.m_viewToWorldMat = ctx.m_matrices.m_cameraTransform;

			if(g_cvarRenderPreferCompute)
			{
				rgraphCtx.bindUav(0, 0, finalRt);

				dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
			}
			else
			{
				cmdb.setViewport(0, 0, rez.x, rez.y);

				drawQuad(cmdb);
			}
		});
	}

	// Temporal denoise
	{
		RenderPassBase* ppass;

		if(preferCompute)
		{
			NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("SSAO temporal denoise");
			ppass = &pass;
		}
		else
		{
			GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("SSAO temporal denoise");
			pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(bentNormalsAndSsaoTempRt)});
			ppass = &pass;
		}

		ppass->newTextureDependency(finalRt, readUsage);
		ppass->newTextureDependency(historyRt, readUsage);
		ppass->newTextureDependency(getMotionVectors().getMotionVectorsRt(), readUsage);
		ppass->newTextureDependency(bentNormalsAndSsaoTempRt, writeUsage);
		ppass->newTextureDependency(getHistoryLength().getRt(), readUsage);

		ppass->setWork([this, bentNormalsAndSsaoTempRt, finalRt, historyRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SsaoTemporalDenoise);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_tempralDenoiseGrProgs[U32(g_cvarRenderSsaoQuarterRez)].get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, finalRt);
			rgraphCtx.bindSrv(1, 0, historyRt);
			rgraphCtx.bindSrv(2, 0, getMotionVectors().getMotionVectorsRt());
			rgraphCtx.bindSrv(3, 0, getHistoryLength().getRt());

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			const UVec2 rez = getRenderer().getInternalResolution();

			if(g_cvarRenderPreferCompute)
			{
				rgraphCtx.bindUav(0, 0, bentNormalsAndSsaoTempRt);
				dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
			}
			else
			{
				cmdb.setViewport(0, 0, rez.x, rez.y);
				drawQuad(cmdb);
			}
		});
	}

	// Spatial denoise
	{
		RenderPassBase* ppass;

		if(preferCompute)
		{
			NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("SSAO spatial denoise");
			ppass = &pass;
		}
		else
		{
			GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("SSAO spatial denoise");
			pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(finalRt)});
			ppass = &pass;
		}

		ppass->newTextureDependency(bentNormalsAndSsaoTempRt, readUsage);
		ppass->newTextureDependency(getGBuffer().getDepthRt(), readUsage);
		ppass->newTextureDependency(finalRt, writeUsage);

		ppass->setWork([this, finalRt, bentNormalsAndSsaoTempRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SsaoSpatialDenoise);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_spatialDenoiseGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, bentNormalsAndSsaoTempRt);
			rgraphCtx.bindSrv(1, 0, getGBuffer().getDepthRt());

			const UVec2 rez = getRenderer().getInternalResolution();

			const IVec4 consts(max(1, g_cvarRenderSsaoSpatialDenoiseSampleCount / 2));
			cmdb.setFastConstants(&consts, sizeof(consts));

			if(g_cvarRenderPreferCompute)
			{
				rgraphCtx.bindUav(0, 0, finalRt);
				dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
			}
			else
			{
				cmdb.setViewport(0, 0, rez.x, rez.y);
				drawQuad(cmdb);
			}
		});
	}
}

} // end namespace anki
