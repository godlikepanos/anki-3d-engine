// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/VrsSriGeneration.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

VrsSriGeneration::VrsSriGeneration(Renderer* r)
	: RendererObject(r)
{
	registerDebugRenderTarget("VrsSri");
	registerDebugRenderTarget("VrsSriDownscaled");
}

VrsSriGeneration::~VrsSriGeneration()
{
}

Error VrsSriGeneration::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize VRS SRI generation");
	}
	return err;
}

Error VrsSriGeneration::initInternal()
{
	if(!getGrManager().getDeviceCapabilities().m_vrs)
	{
		return Error::kNone;
	}

	m_sriTexelDimension = getGrManager().getDeviceCapabilities().m_minShadingRateImageTexelSize;
	ANKI_ASSERT(m_sriTexelDimension == 8 || m_sriTexelDimension == 16);
	const UVec2 rez = (m_r->getInternalResolution() + m_sriTexelDimension - 1) / m_sriTexelDimension;

	ANKI_R_LOGV("Intializing VRS SRI generation. SRI resolution %ux%u", rez.x(), rez.y());

	// Create textures
	const TextureUsageBit texUsage =
		TextureUsageBit::kFramebufferShadingRate | TextureUsageBit::kImageComputeWrite | TextureUsageBit::kAllSampled;
	TextureInitInfo sriInitInfo =
		m_r->create2DRenderTargetInitInfo(rez.x(), rez.y(), Format::kR8_Uint, texUsage, "VrsSri");
	m_sriTex = m_r->createAndClearRenderTarget(sriInitInfo, TextureUsageBit::kFramebufferShadingRate);

	const UVec2 rezDownscaled = (m_r->getInternalResolution() / 2 + m_sriTexelDimension - 1) / m_sriTexelDimension;
	sriInitInfo = m_r->create2DRenderTargetInitInfo(rezDownscaled.x(), rezDownscaled.y(), Format::kR8_Uint, texUsage,
													"VrsSriDownscaled");
	m_downscaledSriTex = m_r->createAndClearRenderTarget(sriInitInfo, TextureUsageBit::kFramebufferShadingRate);

	// Load programs
	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/VrsSriGenerationCompute.ankiprogbin", m_prog));
	ShaderProgramResourceVariantInitInfo variantInit(m_prog);
	variantInit.addMutation("SRI_TEXEL_DIMENSION", m_sriTexelDimension);

	if(m_sriTexelDimension == 16 && getGrManager().getDeviceCapabilities().m_minSubgroupSize >= 32)
	{
		// Algorithm's workgroup size is 32, GPU's subgroup size is min 32 -> each workgroup has 1 subgroup -> No need
		// for shared mem
		variantInit.addMutation("SHARED_MEMORY", 0);
	}
	else if(m_sriTexelDimension == 8 && getGrManager().getDeviceCapabilities().m_minSubgroupSize >= 16)
	{
		// Algorithm's workgroup size is 16, GPU's subgroup size is min 16 -> each workgroup has 1 subgroup -> No need
		// for shared mem
		variantInit.addMutation("SHARED_MEMORY", 0);
	}
	else
	{
		variantInit.addMutation("SHARED_MEMORY", 1);
	}

	variantInit.addMutation("LIMIT_RATE_TO_2X2", getConfig().getRVrsLimitTo2x2());

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInit, variant);
	m_grProg = variant->getProgram();

	ANKI_CHECK(
		getResourceManager().loadResource("ShaderBinaries/VrsSriVisualizeRenderTarget.ankiprogbin", m_visualizeProg));
	m_visualizeProg->getOrCreateVariant(variant);
	m_visualizeGrProg = variant->getProgram();

	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/VrsSriDownscale.ankiprogbin", m_downscaleProg));
	m_downscaleProg->getOrCreateVariant(variant);
	m_downscaleGrProg = variant->getProgram();

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
	const Bool enableVrs = getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs();
	if(!enableVrs)
	{
		return;
	}

	if(m_sriTexImportedOnce)
	{
		m_runCtx.m_rt = ctx.m_renderGraphDescr.importRenderTarget(m_sriTex);
		m_runCtx.m_downscaledRt = ctx.m_renderGraphDescr.importRenderTarget(m_downscaledSriTex);
	}
	else
	{
		m_runCtx.m_rt = ctx.m_renderGraphDescr.importRenderTarget(m_sriTex, TextureUsageBit::kFramebufferShadingRate);
		m_runCtx.m_downscaledRt =
			ctx.m_renderGraphDescr.importRenderTarget(m_downscaledSriTex, TextureUsageBit::kFramebufferShadingRate);
		m_sriTexImportedOnce = true;
	}
}

void VrsSriGeneration::populateRenderGraph(RenderingContext& ctx)
{
	const Bool enableVrs = getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs();
	if(!enableVrs)
	{
		return;
	}

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// SRI generation
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("VRS SRI generation");

		pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kImageComputeWrite);
		pass.newTextureDependency(m_r->getLightShading().getRt(), TextureUsageBit::kSampledCompute);

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

			cmdb->bindShaderProgram(m_grProg);

			rgraphCtx.bindColorTexture(0, 0, m_r->getLightShading().getRt());
			cmdb->bindSampler(0, 1, m_r->getSamplers().m_nearestNearestClamp);
			rgraphCtx.bindImage(0, 2, m_runCtx.m_rt);
			const Vec4 pc(1.0f / Vec2(m_r->getInternalResolution()), getConfig().getRVrsThreshold(), 0.0f);
			cmdb->setPushConstants(&pc, sizeof(pc));

			const U32 fakeWorkgroupSizeXorY = m_sriTexelDimension;
			dispatchPPCompute(cmdb, fakeWorkgroupSizeXorY, fakeWorkgroupSizeXorY, m_r->getInternalResolution().x(),
							  m_r->getInternalResolution().y());
		});
	}

	// Downscale
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("VRS SRI downscale");

		pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kSampledCompute);
		pass.newTextureDependency(m_runCtx.m_downscaledRt, TextureUsageBit::kImageComputeWrite);

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			const UVec2 rezDownscaled =
				(m_r->getInternalResolution() / 2 + m_sriTexelDimension - 1) / m_sriTexelDimension;

			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

			cmdb->bindShaderProgram(m_downscaleGrProg);

			rgraphCtx.bindColorTexture(0, 0, m_runCtx.m_rt);
			cmdb->bindSampler(0, 1, m_r->getSamplers().m_nearestNearestClamp);
			rgraphCtx.bindImage(0, 2, m_runCtx.m_downscaledRt);
			const Vec4 pc(1.0f / Vec2(rezDownscaled), 0.0f, 0.0f);
			cmdb->setPushConstants(&pc, sizeof(pc));

			dispatchPPCompute(cmdb, 8, 8, rezDownscaled.x(), rezDownscaled.y());
		});
	}
}

} // end namespace anki
