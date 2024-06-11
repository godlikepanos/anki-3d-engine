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

static NumericCVar<U32> g_ssaoSampleCountCVar(CVarSubsystem::kRenderer, "SsaoSampleCount", 4, 1, 1024, "SSAO sample count");
static NumericCVar<F32> g_ssaoRadiusCVar(CVarSubsystem::kRenderer, "SsaoRadius", 2.0f, 0.1f, 100.0f, "SSAO radius in meters");
static BoolCVar g_ssaoQuarterRez(CVarSubsystem::kRenderer, "SsaoQuarterResolution", ANKI_PLATFORM_MOBILE, "Render SSAO in quarter rez");
static NumericCVar<F32> g_ssaoPower(CVarSubsystem::kRenderer, "SsaoPower", 1.5f, 0.1f, 100.0f, "SSAO power");
static NumericCVar<U8> g_ssaoSpatialQuality(CVarSubsystem::kRenderer, "SsaoSpatialQuality", (ANKI_PLATFORM_MOBILE) ? 0 : 1, 0, 1,
											"SSAO spatial denoise quality");

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
		usage |= (preferCompute) ? TextureUsageBit::kStorageComputeWrite : TextureUsageBit::kFramebufferWrite;
		TextureInitInfo texInit =
			getRenderer().create2DRenderTargetInitInfo(rez.x(), rez.y(), Format::kR8G8B8A8_Snorm, usage, "Bent normals + SSAO #1");
		m_tex[0] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kAllSampled);

		texInit.setName("Bent normals + SSAO #2");
		m_tex[1] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kAllSampled);
	}

	m_bentNormalsAndSsaoRtDescr =
		getRenderer().create2DRenderTargetDescription(rez.x(), rez.y(), Format::kR8G8B8A8_Snorm, "Bent normals + SSAO temp");
	m_bentNormalsAndSsaoRtDescr.bake();

	ANKI_CHECK(
		loadShaderProgram("ShaderBinaries/Ssao.ankiprogbin", {{"SPATIAL_DENOISE_QUALITY", g_ssaoSpatialQuality.get()}}, m_prog, m_grProg, "Ssao"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Ssao.ankiprogbin", {{"SPATIAL_DENOISE_QUALITY", g_ssaoSpatialQuality.get()}}, m_prog,
								 m_spatialDenoiseGrProg, "SsaoSpatialDenoise"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Ssao.ankiprogbin", {{"SPATIAL_DENOISE_QUALITY", g_ssaoSpatialQuality.get()}}, m_prog,
								 m_tempralDenoiseGrProg, "SsaoTemporalDenoise"));

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	return Error::kNone;
}

void Ssao::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Ssao);
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_preferComputeCVar.get();

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
		finalRt = rgraph.importRenderTarget(m_tex[writeRtIdx].get(), TextureUsageBit::kAllSampled);
		historyRt = rgraph.importRenderTarget(m_tex[readRtIdx].get(), TextureUsageBit::kAllSampled);
		m_texImportedOnce = true;
	}

	m_runCtx.m_finalRt = finalRt;

	const RenderTargetHandle bentNormalsAndSsaoTempRt = rgraph.newRenderTarget(m_bentNormalsAndSsaoRtDescr);

	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	if(preferCompute)
	{
		readUsage = TextureUsageBit::kSampledCompute;
		writeUsage = TextureUsageBit::kStorageComputeWrite;
	}
	else
	{
		readUsage = TextureUsageBit::kSampledFragment;
		writeUsage = TextureUsageBit::kFramebufferWrite;
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

		ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(2), readUsage);
		ppass->newTextureDependency((g_ssaoQuarterRez.get()) ? getRenderer().getDepthDownscale().getRt() : getRenderer().getGBuffer().getDepthRt(),
									readUsage);
		ppass->newTextureDependency(finalRt, writeUsage);

		ppass->setWork([this, &ctx, finalRt](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_grProg.get());

			rgraphCtx.bindTexture(ANKI_REG(t0), getRenderer().getGBuffer().getColorRt(2));
			rgraphCtx.bindTexture(ANKI_REG(t1),
								  (g_ssaoQuarterRez.get()) ? getRenderer().getDepthDownscale().getRt() : getRenderer().getGBuffer().getDepthRt());

			cmdb.bindTexture(ANKI_REG(t2), TextureView(&m_noiseImage->getTexture(), TextureSubresourceDesc::all()));
			cmdb.bindSampler(ANKI_REG(s0), getRenderer().getSamplers().m_trilinearRepeat.get());
			cmdb.bindSampler(ANKI_REG(s1), getRenderer().getSamplers().m_trilinearClamp.get());

			const UVec2 rez = (g_ssaoQuarterRez.get()) ? getRenderer().getInternalResolution() / 2u : getRenderer().getInternalResolution();

			SsaoUniforms consts;
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
			cmdb.setPushConstants(&consts, sizeof(consts));

			if(g_preferComputeCVar.get())
			{
				rgraphCtx.bindTexture(ANKI_REG(u0), finalRt);

				dispatchPPCompute(cmdb, 8, 8, rez.x(), rez.y());
			}
			else
			{
				cmdb.setViewport(0, 0, rez.x(), rez.y());

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
			pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(bentNormalsAndSsaoTempRt)});
			ppass = &pass;
		}

		ppass->newTextureDependency(finalRt, readUsage);
		ppass->newTextureDependency(getRenderer().getGBuffer().getDepthRt(), readUsage);
		ppass->newTextureDependency(bentNormalsAndSsaoTempRt, writeUsage);

		ppass->setWork([this, finalRt, bentNormalsAndSsaoTempRt, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_spatialDenoiseGrProg.get());

			cmdb.bindSampler(ANKI_REG(s0), getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindTexture(ANKI_REG(t0), finalRt);
			rgraphCtx.bindTexture(ANKI_REG(t1), getRenderer().getGBuffer().getDepthRt());

			const UVec2 rez = (g_ssaoQuarterRez.get()) ? getRenderer().getInternalResolution() / 2u : getRenderer().getInternalResolution();

			SsaoSpatialDenoiseUniforms consts;
			computeLinearizeDepthOptimal(ctx.m_cameraNear, ctx.m_cameraFar, consts.m_linearizeDepthParams.x(), consts.m_linearizeDepthParams.y());
			consts.m_viewToWorldMat = ctx.m_matrices.m_cameraTransform;
			cmdb.setPushConstants(&consts, sizeof(consts));

			if(g_preferComputeCVar.get())
			{
				rgraphCtx.bindTexture(ANKI_REG(u0), bentNormalsAndSsaoTempRt);
				dispatchPPCompute(cmdb, 8, 8, rez.x(), rez.y());
			}
			else
			{
				cmdb.setViewport(0, 0, rez.x(), rez.y());
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
			pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(finalRt)});
			ppass = &pass;
		}

		ppass->newTextureDependency(bentNormalsAndSsaoTempRt, readUsage);
		ppass->newTextureDependency(historyRt, readUsage);
		ppass->newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), readUsage);
		ppass->newTextureDependency(finalRt, writeUsage);

		ppass->setWork([this, bentNormalsAndSsaoTempRt, finalRt, historyRt](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_tempralDenoiseGrProg.get());

			cmdb.bindSampler(ANKI_REG(s0), getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindTexture(ANKI_REG(t0), bentNormalsAndSsaoTempRt);
			rgraphCtx.bindTexture(ANKI_REG(t1), historyRt);
			rgraphCtx.bindTexture(ANKI_REG(t2), getRenderer().getMotionVectors().getMotionVectorsRt());

			const UVec2 rez = (g_ssaoQuarterRez.get()) ? getRenderer().getInternalResolution() / 2u : getRenderer().getInternalResolution();

			if(g_preferComputeCVar.get())
			{
				rgraphCtx.bindTexture(ANKI_REG(u0), finalRt);
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
