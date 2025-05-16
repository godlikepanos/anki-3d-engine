// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Sky.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>

namespace anki {

Error GeneratedSky::init()
{
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Sky.ankiprogbin", {}, m_prog, m_transmittanceLutGrProg, "SkyTransmittanceLut"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Sky.ankiprogbin", {}, m_prog, m_multipleScatteringLutGrProg, "SkyMultipleScatteringLut"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Sky.ankiprogbin", {}, m_prog, m_skyLutGrProg, "SkyLut"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Sky.ankiprogbin", {}, m_prog, m_computeSunColorGrProg, "ComputeSunColor"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Sky.ankiprogbin", {}, m_prog, m_computeEnvMapGrProg, "ComputeEnvMap"));

	const TextureUsageBit usage = TextureUsageBit::kAllCompute;
	const TextureUsageBit initialUsage = TextureUsageBit::kSrvCompute;
	const Format formatB = getRenderer().getHdrFormat();

	m_transmittanceLut = getRenderer().createAndClearRenderTarget(
		getRenderer().create2DRenderTargetInitInfo(kTransmittanceLutSize.x(), kTransmittanceLutSize.y(), formatB, usage, "SkyTransmittanceLut"),
		initialUsage);

	m_multipleScatteringLut = getRenderer().createAndClearRenderTarget(
		getRenderer().create2DRenderTargetInitInfo(kMultipleScatteringLutSize.x(), kMultipleScatteringLutSize.y(), formatB, usage,
												   "SkyMultipleScatteringLut"),
		initialUsage);

	m_skyLut = getRenderer().createAndClearRenderTarget(
		getRenderer().create2DRenderTargetInitInfo(kSkyLutSize.x(), kSkyLutSize.y(), formatB, usage | TextureUsageBit::kSrvPixel, "SkyLut"),
		initialUsage);

	m_envMap = getRenderer().createAndClearRenderTarget(getRenderer().create2DRenderTargetInitInfo(kEnvMapSize.x(), kEnvMapSize.y(),
																								   getRenderer().getHdrFormat(),
																								   usage | TextureUsageBit::kAllSrv, "SkyEnvMap"),
														initialUsage);

	return Error::kNone;
}

void GeneratedSky::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Sky);

	if(!isEnabled())
	{
		m_runCtx = {};
		return;
	}

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();
	ANKI_ASSERT(dirLightc);
	const F32 sunPower = dirLightc->getLightPower();
	const Bool renderSkyLut = dirLightc->getDirection() != m_sunDir || m_sunPower != sunPower;
	const Bool renderTransAndMultiScatLuts = !m_transmittanceAndMultiScatterLutsGenerated;

	m_sunDir = dirLightc->getDirection();
	m_sunPower = sunPower;

	// Create render targets
	RenderTargetHandle transmittanceLutRt;
	RenderTargetHandle multipleScatteringLutRt;
	if(renderTransAndMultiScatLuts)
	{
		transmittanceLutRt = rgraph.importRenderTarget(m_transmittanceLut.get(), TextureUsageBit::kSrvCompute);
		multipleScatteringLutRt = rgraph.importRenderTarget(m_multipleScatteringLut.get(), TextureUsageBit::kSrvCompute);
		m_transmittanceAndMultiScatterLutsGenerated = true;
	}
	else
	{
		transmittanceLutRt = rgraph.importRenderTarget(m_transmittanceLut.get(), TextureUsageBit::kSrvCompute);
		multipleScatteringLutRt = rgraph.importRenderTarget(m_multipleScatteringLut.get(), TextureUsageBit::kSrvCompute);
	}

	if(m_skyLutImportedOnce) [[likely]]
	{
		m_runCtx.m_skyLutRt = rgraph.importRenderTarget(m_skyLut.get());
		m_runCtx.m_envMapRt = rgraph.importRenderTarget(m_envMap.get());
	}
	else
	{
		m_runCtx.m_skyLutRt = rgraph.importRenderTarget(m_skyLut.get(), TextureUsageBit::kSrvCompute);
		m_runCtx.m_envMapRt = rgraph.importRenderTarget(m_envMap.get(), TextureUsageBit::kSrvCompute);
		m_skyLutImportedOnce = true;
	}

	// Transmittance LUT
	if(renderTransAndMultiScatLuts)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("SkyTransmittanceLut");

		rpass.newTextureDependency(transmittanceLutRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, transmittanceLutRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SkyTransmittanceLut);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_transmittanceLutGrProg.get());

			rgraphCtx.bindUav(0, 0, transmittanceLutRt);

			dispatchPPCompute(cmdb, 8, 8, kTransmittanceLutSize.x(), kTransmittanceLutSize.y());
		});
	}

	// Multiple scattering LUT
	if(renderTransAndMultiScatLuts)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("SkyMultipleScatteringLut");

		rpass.newTextureDependency(transmittanceLutRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(multipleScatteringLutRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, transmittanceLutRt, multipleScatteringLutRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SkyMultipleScatteringLut);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_multipleScatteringLutGrProg.get());

			rgraphCtx.bindSrv(0, 0, transmittanceLutRt);
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindUav(0, 0, multipleScatteringLutRt);

			dispatchPPCompute(cmdb, 8, 8, kMultipleScatteringLutSize.x(), kMultipleScatteringLutSize.y());
		});
	}

	// Sky LUT
	if(renderSkyLut)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("SkyLut");

		rpass.newTextureDependency(transmittanceLutRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(multipleScatteringLutRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(m_runCtx.m_skyLutRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, transmittanceLutRt, multipleScatteringLutRt, &ctx](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SkyLut);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_skyLutGrProg.get());

			rgraphCtx.bindSrv(0, 0, transmittanceLutRt);
			rgraphCtx.bindSrv(1, 0, multipleScatteringLutRt);
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindUav(0, 0, m_runCtx.m_skyLutRt);
			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			dispatchPPCompute(cmdb, 8, 8, kSkyLutSize.x(), kSkyLutSize.y());
		});
	}

	// Sky env map
	if(renderSkyLut)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("SkyLutEnvMap");

		rpass.newTextureDependency(m_runCtx.m_skyLutRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(m_runCtx.m_envMapRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(SkyLut);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_computeEnvMapGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, m_runCtx.m_skyLutRt);
			rgraphCtx.bindUav(0, 0, m_runCtx.m_envMapRt);
			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			dispatchPPCompute(cmdb, 8, 8, kEnvMapSize.x(), kEnvMapSize.y());
		});
	}

	// Compute sun color always
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ComputeSunColor");

		rpass.newTextureDependency(transmittanceLutRt, TextureUsageBit::kSrvCompute);

		rpass.setWork([this, transmittanceLutRt, &ctx](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ComputeSunColor);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_computeSunColorGrProg.get());

			rgraphCtx.bindSrv(0, 0, transmittanceLutRt);
			cmdb.bindUav(0, 0, ctx.m_globalRenderingConstantsBuffer);

			const UVec4 consts(offsetof(GlobalRendererConstants, m_directionalLight));
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.dispatchCompute(1, 1, 1);
		});
	}
}

Bool GeneratedSky::isEnabled() const
{
	const SkyboxComponent* skyc = SceneGraph::getSingleton().getSkybox();
	const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();

	return skyc && skyc->getSkyboxType() == SkyboxType::kGenerated && dirLightc;
}

} // end namespace anki
