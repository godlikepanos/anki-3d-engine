// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuse.h>
#include <AnKi/Renderer/IndirectDiffuseClipmaps.h>
#include <AnKi/Renderer/Ssao.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/ImageStreaming.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/HistoryLength.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Resource/StreamingImageResourceManager.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error IndirectDiffuse::init()
{
	ANKI_ASSERT(isIndirectDiffuseClipmapsEnabled() && "IndirectDiffuse samples the clipmaps");
	ANKI_CHECK(RtMaterialFetchRendererObject::init());

	UVec2 operatingRez = getRenderer().getInternalResolution();
	if(g_cvarRenderIndirectDiffuseQuarterRez)
	{
		operatingRez /= 2;
	}

	m_colorAndDepthRtDesc1 = getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR16G16B16A16_Sfloat,
																		   "IndirectDiffuse: Color+depth #1");
	m_colorAndDepthRtDesc1.bake();

	m_colorAndDepthRtDesc2 = getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR16G16B16A16_Sfloat,
																		   "IndirectDiffuse: Color+depth #2");
	m_colorAndDepthRtDesc2.bake();

	m_varianceRtDesc1 =
		getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR32_Sfloat, "IndirectDiffuse: Variance #1");
	m_varianceRtDesc1.bake();

	m_varianceRtDesc2 =
		getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR32_Sfloat, "IndirectDiffuse: Variance #2");
	m_varianceRtDesc2.bake();

	m_depthGradientRtDesc =
		getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR16G16_Sfloat, "IndirectDiffuse: Depth gradient");
	m_depthGradientRtDesc.bake();

	m_finalRtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y,
																  getRenderer().getHdrFormat(), "IndirectDiffuse: Final");
	m_finalRtDesc.bake();

	TextureInitInfo texInit = getRenderer().create2DRenderTargetInitInfo(operatingRez.x, operatingRez.y, Format::kR32G32_Sfloat,
																		 TextureUsageBit::kAllShaderResource, "IndirectDiffuse: Moments #1");
	m_momentsTex[0] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);
	texInit.setName("IndirectDiffuse: Moments #2");
	m_momentsTex[1] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);

	texInit = getRenderer().create2DRenderTargetInitInfo(operatingRez.x, operatingRez.y, Format::kR16G16B16A16_Sfloat,
														 TextureUsageBit::kAllShaderResource, "IndirectDiffuse: History");
	m_historyTex = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);

	const Array<SubMutation, 1> mutation = {{{"QUARTER_REZ", g_cvarRenderIndirectDiffuseQuarterRez}}};

	constexpr CString kProgFname = "ShaderBinaries/IndirectDiffuse.ankiprogbin";
	ShaderProgramResourcePtr mainProgRsrc;
	ANKI_CHECK(ResourceManager::getSingleton().loadResource(kProgFname, mainProgRsrc)); // Keep it alive to avoid reloading

	ANKI_CHECK(m_applyProbeIrradianceProg.load(kProgFname, mutation, "ApplyProbeIrradiance"));
	ANKI_CHECK(m_applyGiUsingInlineRtProg.load(kProgFname, mutation, "ApplyInlineRt"));
	ANKI_CHECK(m_temporalFilterProg.load(kProgFname, mutation, "TemporalDenoise"));
	ANKI_CHECK(m_aTrousFilterProg.load(kProgFname, mutation, "ATrous"));
	ANKI_CHECK(m_upscaleProg.load(kProgFname, mutation, "Upscale"));
	ANKI_CHECK(m_rtMaterialFetchProg.load(kProgFname, mutation, "RtMaterialFetch", ShaderTypeBit::kRayGen));

	ANKI_CHECK(m_missProg.load("ShaderBinaries/RtMaterialFetchMiss.ankiprogbin", {}, "RtMaterialFetch", ShaderTypeBit::kMiss));

	m_sbtRecordSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment,
										GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize + U32(sizeof(UVec4)));

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_blueNoiseImg));

	return Error::kNone;
}

void IndirectDiffuse::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuse);

	const Bool firstBounceUsesRt = g_cvarRenderIndirectDiffuseFirstBounceRayDistance > 0.0f;
	const Bool bQuarterRez = g_cvarRenderIndirectDiffuseQuarterRez;

	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	const RenderTargetHandle colorAndDepthRt1 = rgraph.newRenderTarget(m_colorAndDepthRtDesc1);
	RenderTargetHandle colorAndDepthRt2, momentsReadRt, momentsWriteRt, historyColorDepthRt, depthGradientRt;
	if(firstBounceUsesRt)
	{
		colorAndDepthRt2 = rgraph.newRenderTarget(m_colorAndDepthRtDesc2);
		depthGradientRt = rgraph.newRenderTarget(m_depthGradientRtDesc);
		const U64 frame = getRenderer().getFrameCount();
		momentsReadRt = rgraph.importRenderTarget(m_momentsTex[frame & 1].get(), !m_texturesImportedOnce, TextureUsageBit::kSrvCompute);
		momentsWriteRt = rgraph.importRenderTarget(m_momentsTex[(frame + 1) & 1].get(), !m_texturesImportedOnce, TextureUsageBit::kSrvCompute);
		historyColorDepthRt = rgraph.importRenderTarget(m_historyTex.get(), !m_texturesImportedOnce, TextureUsageBit::kSrvCompute);
	}

	const RenderTargetHandle finalRt = rgraph.newRenderTarget(m_finalRtDesc);

	m_texturesImportedOnce = true;

	// SBT build
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	if(firstBounceUsesRt && !g_cvarRenderIndirectDiffuseInlineRt)
	{
		buildShaderBindingTablePass("IndirectDiffuse: Build SBT", m_rtMaterialFetchProg.getShaderGroupHandlesBuffer(),
									m_rtMaterialFetchProg.getShaderGroupHandleIndex(), m_missProg.getShaderGroupHandleIndex(), m_sbtRecordSize,
									rgraph, sbtHandle, sbtBuffer);
	}

	// Apply GI
	if(firstBounceUsesRt)
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuse: Apply RT irradiance");

		if(!g_cvarRenderIndirectDiffuseInlineRt)
		{
			pass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		}

		const TextureUsageBit readUsage = (g_cvarRenderIndirectDiffuseInlineRt) ? TextureUsageBit::kSrvCompute : TextureUsageBit::kSrvDispatchRays;
		const TextureUsageBit writeUsage = (g_cvarRenderIndirectDiffuseInlineRt) ? TextureUsageBit::kUavCompute : TextureUsageBit::kUavDispatchRays;

		getIndirectDiffuseClipmaps().setDependencies(pass, readUsage);
		pass.newTextureDependency(getSsao().getRt(), readUsage);
		if(bQuarterRez)
		{
			pass.newTextureDependency(getDepthDownscale().getDepthRt(), readUsage);
		}

		pass.newTextureDependency(colorAndDepthRt1, writeUsage);
		setRgenSpace2Dependencies(pass, g_cvarRenderIndirectDiffuseInlineRt);

		pass.setWork([this, sbtBuffer, colorAndDepthRt1, bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseApplyRtIrradiance);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			// Space 0 globals
#include <AnKi/Shaders/MaterialBindings.def.h>

			bindRgenSpace2Resources(rgraphCtx);
			rgraphCtx.bindSrv(6, 2, getSsao().getRt());
			rgraphCtx.bindUav(0, 2, colorAndDepthRt1);
			if(bQuarterRez)
			{
				rgraphCtx.bindSrv(4, 2, getDepthDownscale().getDepthRt());
			}

			const Array<Vec4, 3> consts = {Vec4(g_cvarRenderIndirectDiffuseFirstBounceRayDistance), {}, {}};
			cmdb.setFastConstants(&consts, sizeof(consts));

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			if(g_cvarRenderIndirectDiffuseInlineRt)
			{
				cmdb.bindShaderProgram(m_applyGiUsingInlineRtProg.get());

				dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
			}
			else
			{
				cmdb.bindShaderProgram(m_rtMaterialFetchProg.get());

				cmdb.dispatchRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1, rez.x,
								  rez.y, 1);
			}
		});
	}
	else
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuse: Apply probe irradiance");

		pass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getSsao().getRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency((bQuarterRez) ? colorAndDepthRt1 : finalRt, TextureUsageBit::kUavCompute);
		getIndirectDiffuseClipmaps().setDependencies(pass, TextureUsageBit::kSrvCompute);

		pass.setWork([this, colorAndDepthRt1, bQuarterRez, finalRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseApplyProbeIrradiance);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_applyProbeIrradianceProg.get());

			rgraphCtx.bindSrv(0, 0, (bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(1, 0, getSsao().getRt());
			cmdb.bindSrv(2, 0, TextureView(&m_blueNoiseImg->getTexture(), TextureSubresourceDesc::firstSurface()));

			rgraphCtx.bindUav(0, 0, (bQuarterRez) ? colorAndDepthRt1 : finalRt);

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	// Temporal denoise
	if(firstBounceUsesRt)
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuse: Temporal denoise");

		pass.newTextureDependency(colorAndDepthRt1, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getHistoryLength().getRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(historyColorDepthRt, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getAdjustedMotionVectorsRt() : getMotionVectors().getAdjustedMotionVectorsRt(),
								  TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(momentsReadRt, TextureUsageBit::kSrvCompute);

		pass.newTextureDependency(colorAndDepthRt2, TextureUsageBit::kUavCompute);
		pass.newTextureDependency(momentsWriteRt, TextureUsageBit::kUavCompute);
		pass.newTextureDependency(depthGradientRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, colorAndDepthRt1, colorAndDepthRt2, momentsReadRt, momentsWriteRt, historyColorDepthRt, depthGradientRt,
					  bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseTemporalDenoise);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_temporalFilterProg.get());

			rgraphCtx.bindSrv(0, 0, colorAndDepthRt1);
			rgraphCtx.bindSrv(1, 0, historyColorDepthRt);
			rgraphCtx.bindSrv(2, 0,
							  (bQuarterRez) ? getDepthDownscale().getAdjustedMotionVectorsRt() : getMotionVectors().getAdjustedMotionVectorsRt());
			rgraphCtx.bindSrv(3, 0, momentsReadRt);
			rgraphCtx.bindSrv(4, 0, getHistoryLength().getRt());

			rgraphCtx.bindUav(0, 0, colorAndDepthRt2);
			rgraphCtx.bindUav(1, 0, momentsWriteRt);
			rgraphCtx.bindUav(2, 0, depthGradientRt);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	// A-Trous
	RenderTargetHandle aTrousOutRt;
	if(firstBounceUsesRt)
	{
		// Ping-pong the color and the variance. The 1st pass reads the moments and the temporal output
		const Array<RenderTargetHandle, 2> colorAndDepthRts = {colorAndDepthRt2, colorAndDepthRt1};
		const Array<RenderTargetHandle, 2> varianceRts = {rgraph.newRenderTarget(m_varianceRtDesc1), rgraph.newRenderTarget(m_varianceRtDesc2)};
		const U32 passCount = g_cvarRenderIndirectDiffuseATrousPassCount;
		ANKI_ASSERT(passCount >= 2 && "The 1st pass writes the history, a later pass needs to write the output");

		for(U32 passIdx = 0; passIdx < passCount; ++passIdx)
		{
			const Bool firstPass = passIdx == 0;
			const Bool lastPass = passIdx == passCount - 1;

			RenderTargetHandle colorAndDepthInRt;
			if(passIdx == 1)
			{
				colorAndDepthInRt = historyColorDepthRt;
			}
			else
			{
				colorAndDepthInRt = colorAndDepthRts[passIdx & 1];
			}

			RenderTargetHandle colorAndDepthOutRt;
			if(firstPass)
			{
				colorAndDepthOutRt = historyColorDepthRt;
			}
			else if(lastPass && !bQuarterRez)
			{
				colorAndDepthOutRt = finalRt;
			}
			else
			{
				colorAndDepthOutRt = colorAndDepthRts[(passIdx + 1) & 1];
			}
			const RenderTargetHandle momentsOrVarianceInRt = (firstPass) ? momentsWriteRt : varianceRts[(passIdx + 1) & 1];
			const RenderTargetHandle varianceOutRt = varianceRts[passIdx & 1];

			NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("IndirectDiffuse: A-Trous #%u", passIdx));

			pass.newTextureDependency(colorAndDepthInRt, TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(momentsOrVarianceInRt, TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(getHistoryLength().getRt(), TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(depthGradientRt, TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(colorAndDepthOutRt, TextureUsageBit::kUavCompute);
			pass.newTextureDependency(varianceOutRt, TextureUsageBit::kUavCompute);

			pass.setWork([this, colorAndDepthInRt, colorAndDepthOutRt, momentsOrVarianceInRt, varianceOutRt, depthGradientRt, passIdx,
						  bQuarterRez](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseATrous);
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram(m_aTrousFilterProg.get());

				rgraphCtx.bindSrv(0, 0, colorAndDepthInRt);
				rgraphCtx.bindSrv(1, 0, momentsOrVarianceInRt);
				rgraphCtx.bindSrv(2, 0, getHistoryLength().getRt());
				rgraphCtx.bindSrv(3, 0, depthGradientRt);

				rgraphCtx.bindUav(0, 0, colorAndDepthOutRt);
				rgraphCtx.bindUav(1, 0, varianceOutRt);

				const UVec4 consts(passIdx, 0, 0, 0);
				cmdb.setFastConstants(&consts, sizeof(consts));

				UVec2 rez = getRenderer().getInternalResolution();
				if(bQuarterRez)
				{
					rez /= 2;
				}

				dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
			});

			aTrousOutRt = colorAndDepthOutRt;
		}
	}

	// Upscale
	if(bQuarterRez)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("IndirectDiffuse: Upscale");

		const RenderTargetHandle srcRt = (firstBounceUsesRt) ? aTrousOutRt : colorAndDepthRt1;

		rpass.newTextureDependency(srcRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(finalRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, srcRt, finalRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseUpscale);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_upscaleProg.get());

			rgraphCtx.bindSrv(0, 0, srcRt);
			rgraphCtx.bindSrv(1, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(2, 0, getGBuffer().getColorRt(2));

			rgraphCtx.bindUav(0, 0, finalRt);

			const UVec2 rez = getRenderer().getInternalResolution() / 2;

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	m_runCtx.m_rt = finalRt;
}

} // end namespace anki
