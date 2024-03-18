// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Sky.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>

namespace anki {

Error Sky::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize sky passes");
	}

	return Error::kNone;
}

Error Sky::initInternal()
{
	ANKI_R_LOGV("Initializing sky passes");

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Sky.ankiprogbin", {}, m_prog, m_transmittanceLutGrProg, "SkyTransmittanceLut"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Sky.ankiprogbin", {}, m_prog, m_multipleScatteringLutGrProg, "SkyMultipleScatteringLut"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Sky.ankiprogbin", {}, m_prog, m_skyLutGrProg, "SkyLut"));

	const TextureUsageBit usage = TextureUsageBit::kAllCompute;
	const TextureUsageBit initialUsage = TextureUsageBit::kAllCompute;
	const Format formatB =
		(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR16G16B16_Unorm : Format::kR16G16B16A16_Unorm;

	m_transmittanceLut = getRenderer().createAndClearRenderTarget(
		getRenderer().create2DRenderTargetInitInfo(kTransmittanceLutSize.x(), kTransmittanceLutSize.y(), formatB, usage, "SkyTransmittanceLut"),
		initialUsage);

	// TODO check formats
	m_multipleScatteringLut = getRenderer().createAndClearRenderTarget(
		getRenderer().create2DRenderTargetInitInfo(kMultipleScatteringLutSize.x(), kMultipleScatteringLutSize.y(), formatB, usage,
												   "SkyMultipleScatteringLut"),
		initialUsage);

	m_skyLut = getRenderer().createAndClearRenderTarget(
		getRenderer().create2DRenderTargetInitInfo(kSkyLutSize.x(), kSkyLutSize.y(), formatB, usage | TextureUsageBit::kSampledFragment, "SkyLut"),
		initialUsage);

	return Error::kNone;
}

void Sky::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Sky);

	if(!isEnabled())
	{
		m_runCtx = {};
		return;
	}

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();
	ANKI_ASSERT(dirLightc);
	if(dirLightc->getDirection() == m_sunDir)
	{
		// Same light dir, no need to generate the sky LUT

		m_runCtx.m_skyLutRt = rgraph.importRenderTarget(m_skyLut.get());
		return;
	}

	// Create render targets
	const Bool generateLuts = !m_transmittanceAndMultiScatterLutsGenerated;
	RenderTargetHandle transmittanceLutRt;
	RenderTargetHandle multipleScatteringLutRt;
	if(generateLuts)
	{
		transmittanceLutRt = rgraph.importRenderTarget(m_transmittanceLut.get(), TextureUsageBit::kAllCompute);
		multipleScatteringLutRt = rgraph.importRenderTarget(m_multipleScatteringLut.get(), TextureUsageBit::kAllCompute);
		m_transmittanceAndMultiScatterLutsGenerated = true;
	}
	else
	{
		transmittanceLutRt = rgraph.importRenderTarget(m_transmittanceLut.get(), TextureUsageBit::kSampledCompute);
		multipleScatteringLutRt = rgraph.importRenderTarget(m_multipleScatteringLut.get(), TextureUsageBit::kSampledCompute);
	}

	if(m_skyLutImportedOnce) [[likely]]
	{
		m_runCtx.m_skyLutRt = rgraph.importRenderTarget(m_skyLut.get());
	}
	else
	{
		m_runCtx.m_skyLutRt = rgraph.importRenderTarget(m_skyLut.get(), TextureUsageBit::kAllCompute);
		m_skyLutImportedOnce = true;
	}

	// Transmittance LUT
	if(generateLuts)
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SkyTransmittanceLut");

		rpass.newTextureDependency(transmittanceLutRt, TextureUsageBit::kUavComputeWrite);

		rpass.setWork([this, transmittanceLutRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SkyTransmittanceLut);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_transmittanceLutGrProg.get());

			rgraphCtx.bindUavTexture(0, 0, transmittanceLutRt);

			dispatchPPCompute(cmdb, 8, 8, kTransmittanceLutSize.x(), kTransmittanceLutSize.y());
		});
	}

	//  Multiple scattering LUT
	if(generateLuts)
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SkyMultipleScatteringLut");

		rpass.newTextureDependency(transmittanceLutRt, TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(multipleScatteringLutRt, TextureUsageBit::kUavComputeWrite);

		rpass.setWork([this, transmittanceLutRt, multipleScatteringLutRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SkyMultipleScatteringLut);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_multipleScatteringLutGrProg.get());

			rgraphCtx.bindColorTexture(0, 0, transmittanceLutRt);
			cmdb.bindSampler(0, 1, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindUavTexture(0, 2, multipleScatteringLutRt);

			dispatchPPCompute(cmdb, 8, 8, kMultipleScatteringLutSize.x(), kMultipleScatteringLutSize.y());
		});
	}

	// Sky LUT
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SkyLut");

		rpass.newTextureDependency(transmittanceLutRt, TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(multipleScatteringLutRt, TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(m_runCtx.m_skyLutRt, TextureUsageBit::kUavComputeWrite);

		rpass.setWork([this, transmittanceLutRt, multipleScatteringLutRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SkyLut);

			const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_skyLutGrProg.get());

			rgraphCtx.bindColorTexture(0, 0, transmittanceLutRt);
			rgraphCtx.bindColorTexture(0, 1, multipleScatteringLutRt);
			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindUavTexture(0, 3, m_runCtx.m_skyLutRt);

			const Vec4 dir = -dirLightc->getDirection().xyz0();
			cmdb.setPushConstants(&dir, sizeof(dir));

			dispatchPPCompute(cmdb, 8, 8, kSkyLutSize.x(), kSkyLutSize.y());
		});
	}
}

Bool Sky::isEnabled() const
{
	const SkyboxComponent* skyc = SceneGraph::getSingleton().getSkybox();
	const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();

	return skyc && skyc->getSkyboxType() == SkyboxType::kGenerated && dirLightc;
}

} // end namespace anki
