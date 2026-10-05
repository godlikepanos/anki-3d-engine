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

	m_tmpRtDesc1 =
		getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR16G16B16A16_Sfloat, "IndirectDiffuse: Irradiance #1");
	m_tmpRtDesc1.bake();

	m_tmpRtDesc2 =
		getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR16G16B16A16_Sfloat, "IndirectDiffuse: Irradiance #2");
	m_tmpRtDesc2.bake();

	const TextureInitInfo init = getRenderer().create2DRenderTargetInitInfo(
		getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, Format::kR16G16B16A16_Sfloat,
		TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuse: Final"));
	m_finalTex = getRenderer().createAndClearRenderTarget(init, TextureUsageBit::kSrvCompute);

	const Array<SubMutation, 1> mutation = {{{"QUARTER_REZ", g_cvarRenderIndirectDiffuseQuarterRez}}};

	constexpr CString kProgFname = "ShaderBinaries/IndirectDiffuse.ankiprogbin";
	ShaderProgramResourcePtr mainProgRsrc;
	ANKI_CHECK(ResourceManager::getSingleton().loadResource(kProgFname, mainProgRsrc)); // Keep it alive to avoid reloading

	ANKI_CHECK(m_applyProbeIrradianceProg.load(kProgFname, mutation, "ApplyProbeIrradiance"));
	ANKI_CHECK(m_applyGiUsingInlineRtProg.load(kProgFname, mutation, "ApplyInlineRt"));
	ANKI_CHECK(m_temporalDenoiseProg.load(kProgFname, mutation, "TemporalDenoise"));
	ANKI_CHECK(m_antiFireflyProg.load(kProgFname, mutation, "AntiFirefly"));
	ANKI_CHECK(m_bilateralDenoiseProg.load(kProgFname, mutation, "BilateralDenoise"));
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

	const RenderTargetHandle tmpRt1 = rgraph.newRenderTarget(m_tmpRtDesc1);
	RenderTargetHandle tmpRt2;
	if(firstBounceUsesRt)
	{
		tmpRt2 = rgraph.newRenderTarget(m_tmpRtDesc2);
	}

	const RenderTargetHandle finalRt = rgraph.importRenderTarget(m_finalTex.get(), !m_texturesImportedOnce, TextureUsageBit::kSrvCompute);

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

		pass.newTextureDependency(tmpRt1, writeUsage);
		setRgenSpace2Dependencies(pass, g_cvarRenderIndirectDiffuseInlineRt);

		pass.setWork([this, sbtBuffer, tmpRt1, bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseApplyRtIrradiance);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			// Space 0 globals
#include <AnKi/Shaders/MaterialBindings.def.h>

			bindRgenSpace2Resources(rgraphCtx);
			rgraphCtx.bindSrv(6, 2, getSsao().getRt());
			rgraphCtx.bindUav(0, 2, tmpRt1);
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
		pass.newTextureDependency((bQuarterRez) ? tmpRt1 : finalRt, TextureUsageBit::kUavCompute);
		getIndirectDiffuseClipmaps().setDependencies(pass, TextureUsageBit::kSrvCompute);

		pass.setWork([this, tmpRt1, bQuarterRez, finalRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseApplyProbeIrradiance);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_applyProbeIrradianceProg.get());

			rgraphCtx.bindSrv(0, 0, (bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(1, 0, getSsao().getRt());
			cmdb.bindSrv(2, 0, TextureView(&m_blueNoiseImg->getTexture(), TextureSubresourceDesc::firstSurface()));

			rgraphCtx.bindUav(0, 0, (bQuarterRez) ? tmpRt1 : finalRt);

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

		pass.newTextureDependency(tmpRt1, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(finalRt, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getAdjustedMotionVectorsRt() : getMotionVectors().getAdjustedMotionVectorsRt(),
								  TextureUsageBit::kSrvCompute);
		pass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(tmpRt2, TextureUsageBit::kUavCompute);

		pass.setWork([this, tmpRt1, tmpRt2, finalRt, bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseTemporalDenoise);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_temporalDenoiseProg.get());

			rgraphCtx.bindSrv(0, 0, tmpRt1);
			rgraphCtx.bindSrv(1, 0, finalRt);
			rgraphCtx.bindSrv(2, 0,
							  (bQuarterRez) ? getDepthDownscale().getAdjustedMotionVectorsRt() : getMotionVectors().getAdjustedMotionVectorsRt());
			rgraphCtx.bindSrv(3, 0, (bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt());

			rgraphCtx.bindUav(0, 0, tmpRt2);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	// Anti-firefly
	if(firstBounceUsesRt)
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuse: Anti-firefly");

		pass.newTextureDependency(tmpRt2, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(tmpRt1, TextureUsageBit::kUavCompute);

		pass.setWork([this, tmpRt1, tmpRt2, bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseAntiFirefly);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_antiFireflyProg.get());

			rgraphCtx.bindSrv(0, 0, tmpRt2);
			rgraphCtx.bindUav(0, 0, tmpRt1);

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	// Bilateral denoise
	if(firstBounceUsesRt)
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuse: Bilateral denoise");

		pass.newTextureDependency(tmpRt1, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getHistoryLength().getRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency((bQuarterRez) ? tmpRt2 : finalRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, tmpRt1, tmpRt2, bQuarterRez, finalRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseBilateralDenoise);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_bilateralDenoiseProg.get());

			rgraphCtx.bindSrv(0, 0, tmpRt1);
			rgraphCtx.bindSrv(1, 0, getHistoryLength().getRt());
			rgraphCtx.bindUav(0, 0, (bQuarterRez) ? tmpRt2 : finalRt);

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	// Upscale
	if(bQuarterRez)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("IndirectDiffuse: Upscale");

		const RenderTargetHandle srcRt = (firstBounceUsesRt) ? tmpRt2 : tmpRt1;

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
