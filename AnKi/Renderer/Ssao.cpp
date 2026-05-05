// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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

Error Ssao::init()
{
	UVec2 mainRez = getRenderer().getInternalResolution();
	if(g_cvarRenderSsaoQuarterRez)
	{
		mainRez /= 2;
	}

	{
		TextureUsageBit usage = TextureUsageBit::kAllSrv;
		usage |= TextureUsageBit::kUavCompute;
		TextureInitInfo texInit = getRenderer().create2DRenderTargetInitInfo(
			getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, Format::kR8G8B8A8_Snorm, usage, "Bent normals + SSAO");
		m_tex = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kAllSrv);
	}

	m_bentNormalsAndSsaoRtDescr[0] =
		getRenderer().create2DRenderTargetDescription(mainRez.x, mainRez.y, Format::kR8G8B8A8_Snorm, "Bent normals + SSAO tmp #1");
	m_bentNormalsAndSsaoRtDescr[0].bake();

	m_bentNormalsAndSsaoRtDescr[1] =
		getRenderer().create2DRenderTargetDescription(mainRez.x, mainRez.y, Format::kR8G8B8A8_Snorm, "Bent normals + SSAO tmp #2");
	m_bentNormalsAndSsaoRtDescr[1].bake();

	// Use 16bit only on Arm because that path increases the instruction count on AMD
	const Array<SubMutation, 3> mutation = {{{"PACKED_NORMALS", MutatorValue(!g_cvarRenderSsaoQuarterRez)},
											 {"SSAO_16BIT", GrManager::getSingleton().getDeviceCapabilities().m_gpuVendor == GpuVendor::kArm},
											 {"QUARTER_REZ", MutatorValue(g_cvarRenderSsaoQuarterRez)}}};

	ANKI_CHECK(m_ssaoGrProg.load("ShaderBinaries/Ssao.ankiprogbin", mutation, "Ssao"));
	ANKI_CHECK(m_spatialDenoiseGrProg.load("ShaderBinaries/Ssao.ankiprogbin", mutation, "SpatialDenoise"));
	ANKI_CHECK(m_temporalDenoiseGrProg.load("ShaderBinaries/Ssao.ankiprogbin", mutation, "TemporalDenoise"));
	if(g_cvarRenderSsaoQuarterRez)
	{
		ANKI_CHECK(m_upscaleGrProg.load("ShaderBinaries/Ssao.ankiprogbin", mutation, "Upscale"));
	}

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	return Error::kNone;
}

void Ssao::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(Ssao);
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;
	const Bool preferCompute = g_cvarRenderPreferCompute;
	const Bool quarterRez = g_cvarRenderSsaoQuarterRez;

	const RenderTargetHandle historyRt = rgraph.importRenderTarget(m_tex.get(), !m_texImportedOnce, TextureUsageBit::kAllSrv);
	m_texImportedOnce = true;

	m_runCtx.m_finalRt = historyRt;

	const RenderTargetHandle tmpRt1 = rgraph.newRenderTarget(m_bentNormalsAndSsaoRtDescr[0]);
	const RenderTargetHandle tmpRt2 = rgraph.newRenderTarget(m_bentNormalsAndSsaoRtDescr[1]);

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
			pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(tmpRt1)});
			ppass = &pass;
		}

		ppass->newTextureDependency((quarterRez) ? getDepthDownscale().getNormalsRt() : getGBuffer().getColorRt(2), readUsage);
		ppass->newTextureDependency((quarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt(), readUsage);
		ppass->newTextureDependency(tmpRt1, writeUsage);

		ppass->setWork([this, tmpRt1](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SsaoMain);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			RenderingContext& ctx = getRenderingContext();
			const Bool quarterRez = g_cvarRenderSsaoQuarterRez;

			cmdb.bindShaderProgram(m_ssaoGrProg.get());

			rgraphCtx.bindSrv(0, 0, (quarterRez) ? getDepthDownscale().getNormalsRt() : getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(1, 0, (quarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt());
			cmdb.bindSrv(2, 0, TextureView(&m_noiseImage->getTexture(), TextureSubresourceDesc::all()));
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
			cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			const UVec2 rez = (quarterRez) ? getRenderer().getInternalResolution() / 2 : getRenderer().getInternalResolution();

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
				rgraphCtx.bindUav(0, 0, tmpRt1);

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
			pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(tmpRt2)});
			ppass = &pass;
		}

		ppass->newTextureDependency(tmpRt1, readUsage);
		ppass->newTextureDependency(historyRt, readUsage);
		ppass->newTextureDependency((quarterRez) ? getDepthDownscale().getAdjustedMotionVectorsRt() : getMotionVectors().getAdjustedMotionVectorsRt(),
									readUsage);
		ppass->newTextureDependency(tmpRt2, writeUsage);

		ppass->setWork([this, tmpRt1, tmpRt2, historyRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SsaoTemporalDenoise);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			const Bool quarterRez = g_cvarRenderSsaoQuarterRez;

			cmdb.bindShaderProgram(m_temporalDenoiseGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindSrv(0, 0, tmpRt1);
			rgraphCtx.bindSrv(1, 0, historyRt);
			rgraphCtx.bindSrv(2, 0,
							  (quarterRez) ? getDepthDownscale().getAdjustedMotionVectorsRt() : getMotionVectors().getAdjustedMotionVectorsRt());

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			UVec2 rez = getRenderer().getInternalResolution();
			if(quarterRez)
			{
				rez /= 2;
			}

			if(g_cvarRenderPreferCompute)
			{
				rgraphCtx.bindUav(0, 0, tmpRt2);
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
			pass.setRenderpassInfo({GraphicsRenderPassTargetDesc((quarterRez) ? tmpRt1 : historyRt)});
			ppass = &pass;
		}

		ppass->newTextureDependency(tmpRt2, readUsage);
		ppass->newTextureDependency((quarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt(), readUsage);
		ppass->newTextureDependency((quarterRez) ? tmpRt1 : historyRt, writeUsage);

		ppass->setWork([this, tmpRt2, tmpRt1, historyRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SsaoSpatialDenoise);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			const Bool quarterRez = g_cvarRenderSsaoQuarterRez;

			cmdb.bindShaderProgram(m_spatialDenoiseGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindSrv(0, 0, tmpRt2);
			rgraphCtx.bindSrv(1, 0, (quarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt());

			UVec2 rez = getRenderer().getInternalResolution();
			if(quarterRez)
			{
				rez /= 2;
			}

			const IVec4 consts(max(1, g_cvarRenderSsaoSpatialDenoiseSampleCount / 2));
			cmdb.setFastConstants(&consts, sizeof(consts));

			if(g_cvarRenderPreferCompute)
			{
				rgraphCtx.bindUav(0, 0, (quarterRez) ? tmpRt1 : historyRt);
				dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
			}
			else
			{
				cmdb.setViewport(0, 0, rez.x, rez.y);
				drawQuad(cmdb);
			}
		});
	}

	// Upscale
	if(quarterRez)
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("SSAO upscale");

		const TextureUsageBit readUsage = TextureUsageBit::kSrvCompute;
		const TextureUsageBit writeUsage = TextureUsageBit::kUavCompute;

		pass.newTextureDependency(tmpRt1, readUsage);
		pass.newTextureDependency(getGBuffer().getDepthRt(), readUsage);
		pass.newTextureDependency(getGBuffer().getColorRt(2), readUsage);
		pass.newTextureDependency(historyRt, writeUsage);

		pass.setWork([this, tmpRt1, historyRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SsaoUpscale);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_upscaleGrProg.get());

			rgraphCtx.bindSrv(0, 0, tmpRt1);
			rgraphCtx.bindSrv(1, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(2, 0, getGBuffer().getColorRt(2));

			const UVec2 rez = getRenderer().getInternalResolution() / 2;

			rgraphCtx.bindUav(0, 0, historyRt);
			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}
}

} // end namespace anki
