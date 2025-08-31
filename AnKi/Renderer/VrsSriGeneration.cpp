// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/VrsSriGeneration.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error VrsSriGeneration::init()
{
	if(!GrManager::getSingleton().getDeviceCapabilities().m_vrs)
	{
		return Error::kNone;
	}

	m_sriTexelDimension = U8(GrManager::getSingleton().getDeviceCapabilities().m_minShadingRateImageTexelSize);
	ANKI_ASSERT(m_sriTexelDimension == 8 || m_sriTexelDimension == 16);
	const UVec2 rez = (getRenderer().getInternalResolution() + m_sriTexelDimension - 1) / m_sriTexelDimension;

	// Create textures
	const TextureUsageBit texUsage = TextureUsageBit::kShadingRate | TextureUsageBit::kUavCompute | TextureUsageBit::kAllSrv;
	TextureInitInfo sriInitInfo = getRenderer().create2DRenderTargetInitInfo(rez.x(), rez.y(), Format::kR8_Uint, texUsage, "VrsSri");
	m_sriTex = getRenderer().createAndClearRenderTarget(sriInitInfo, TextureUsageBit::kShadingRate);

	const UVec2 rezDownscaled = (getRenderer().getInternalResolution() / 2 + m_sriTexelDimension - 1) / m_sriTexelDimension;
	sriInitInfo = getRenderer().create2DRenderTargetInitInfo(rezDownscaled.x(), rezDownscaled.y(), Format::kR8_Uint, texUsage, "VrsSriDownscaled");
	m_downscaledSriTex = getRenderer().createAndClearRenderTarget(sriInitInfo, TextureUsageBit::kShadingRate);

	// Load programs
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/VrsSriGenerationCompute.ankiprogbin", m_prog));
	ShaderProgramResourceVariantInitInfo variantInit(m_prog);
	variantInit.addMutation("SRI_TEXEL_DIMENSION", m_sriTexelDimension);

	if(m_sriTexelDimension == 16 && GrManager::getSingleton().getDeviceCapabilities().m_minWaveSize >= 32)
	{
		// Algorithm's workgroup size is 32, GPU's subgroup size is min 32 -> each workgroup has 1 subgroup -> No need for shared mem
		variantInit.addMutation("SHARED_MEMORY", 0);
	}
	else if(m_sriTexelDimension == 8 && GrManager::getSingleton().getDeviceCapabilities().m_minWaveSize >= 16)
	{
		// Algorithm's workgroup size is 16, GPU's subgroup size is min 16 -> each workgroup has 1 subgroup -> No need for shared mem
		variantInit.addMutation("SHARED_MEMORY", 0);
	}
	else
	{
		variantInit.addMutation("SHARED_MEMORY", 1);
	}

	variantInit.addMutation("LIMIT_RATE_TO_2X2", g_cvarRenderVrsLimitTo2x2);

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInit, variant);
	m_grProg.reset(&variant->getProgram());

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/VrsSriVisualizeRenderTarget.ankiprogbin", m_visualizeProg, m_visualizeGrProg));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/VrsSriDownscale.ankiprogbin", m_downscaleProg, m_downscaleGrProg));

	return Error::kNone;
}

void VrsSriGeneration::getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
											ShaderProgramPtr& optionalShaderProgram) const
{
	if(rtName == "VrsSri")
	{
		handles[0] = m_runCtx.m_rt;
	}
	else
	{
		ANKI_ASSERT(rtName == "VrsSriDownscaled");
		handles[0] = m_runCtx.m_downscaledRt;
	}

	optionalShaderProgram = m_visualizeGrProg;
}

void VrsSriGeneration::importRenderTargets(RenderingContext& ctx)
{
	const Bool enableVrs = GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_cvarGrVrs;
	if(!enableVrs)
	{
		return;
	}

	if(m_sriTexImportedOnce)
	{
		m_runCtx.m_rt = ctx.m_renderGraphDescr.importRenderTarget(m_sriTex.get());
		m_runCtx.m_downscaledRt = ctx.m_renderGraphDescr.importRenderTarget(m_downscaledSriTex.get());
	}
	else
	{
		m_runCtx.m_rt = ctx.m_renderGraphDescr.importRenderTarget(m_sriTex.get(), TextureUsageBit::kShadingRate);
		m_runCtx.m_downscaledRt = ctx.m_renderGraphDescr.importRenderTarget(m_downscaledSriTex.get(), TextureUsageBit::kShadingRate);
		m_sriTexImportedOnce = true;
	}
}

void VrsSriGeneration::populateRenderGraph(RenderingContext& ctx)
{
	const Bool enableVrs = GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_cvarGrVrs;
	if(!enableVrs)
	{
		return;
	}

	ANKI_TRACE_SCOPED_EVENT(VrsSriGeneration);

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	// SRI generation
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("VRS SRI generation");

		pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kUavCompute);
		pass.newTextureDependency(getRenderer().getLightShading().getRt(), TextureUsageBit::kSrvCompute);

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(VrsSriGeneration);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_grProg.get());

			rgraphCtx.bindSrv(0, 0, getRenderer().getLightShading().getRt());
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
			rgraphCtx.bindUav(0, 0, m_runCtx.m_rt);
			const Vec4 pc(1.0f / Vec2(getRenderer().getInternalResolution()), g_cvarRenderVrsThreshold, 0.0f);
			cmdb.setFastConstants(&pc, sizeof(pc));

			const U32 fakeWorkgroupSizeXorY = m_sriTexelDimension;
			dispatchPPCompute(cmdb, fakeWorkgroupSizeXorY, fakeWorkgroupSizeXorY, getRenderer().getInternalResolution().x(),
							  getRenderer().getInternalResolution().y());
		});
	}

	// Downscale
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("VRS SRI downscale");

		pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(m_runCtx.m_downscaledRt, TextureUsageBit::kUavCompute);

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(VrsSriGeneration);
			const UVec2 rezDownscaled = (getRenderer().getInternalResolution() / 2 + m_sriTexelDimension - 1) / m_sriTexelDimension;

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_downscaleGrProg.get());

			rgraphCtx.bindSrv(0, 0, m_runCtx.m_rt);
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
			rgraphCtx.bindUav(0, 0, m_runCtx.m_downscaledRt);
			const Vec4 pc(1.0f / Vec2(rezDownscaled), 0.0f, 0.0f);
			cmdb.setFastConstants(&pc, sizeof(pc));

			dispatchPPCompute(cmdb, 8, 8, rezDownscaled.x(), rezDownscaled.y());
		});
	}
}

} // end namespace anki
