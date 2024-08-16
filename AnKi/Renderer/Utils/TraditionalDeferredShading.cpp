// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Utils/TraditionalDeferredShading.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Shaders/Include/TraditionalDeferredShadingTypes.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>

namespace anki {

Error TraditionalDeferredLightShading::init()
{
	// Init progs
	for(MutatorValue specular = 0; specular <= 1; ++specular)
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/TraditionalDeferredShading.ankiprogbin", {{"SPECULAR", specular}}, m_lightProg,
									 m_lightGrProg[specular]));
	}

	// Shadow sampler
	{
		SamplerInitInfo inf;
		inf.m_compareOperation = CompareOperation::kLessEqual;
		inf.m_addressing = SamplingAddressing::kClamp;
		inf.m_mipmapFilter = SamplingFilter::kNearest;
		inf.m_minMagFilter = SamplingFilter::kLinear;
		m_shadowSampler = GrManager::getSingleton().newSampler(inf);
	}

	// Skybox
	for(MutatorValue i = 0; i < m_skyboxGrProgs.getSize(); ++i)
	{
		ANKI_CHECK(
			loadShaderProgram("ShaderBinaries/TraditionalDeferredShadingSkybox.ankiprogbin", {{"METHOD", i}}, m_skyboxProg, m_skyboxGrProgs[i]));
	}

	return Error::kNone;
}

void TraditionalDeferredLightShading::drawLights(TraditionalDeferredLightShadingDrawInfo& info)
{
	CommandBuffer& cmdb = *info.m_renderpassContext->m_commandBuffer;
	RenderPassWorkContext& rgraphCtx = *info.m_renderpassContext;

	// Set common state for all
	cmdb.setViewport(info.m_viewport.x(), info.m_viewport.y(), info.m_viewport.z(), info.m_viewport.w());

	// Skybox first
	const SkyboxComponent* skyc = SceneGraph::getSingleton().getSkybox();
	const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();
	if(skyc && !(skyc->getSkyboxType() == SkyboxType::kGenerated && !dirLightc))
	{
		cmdb.bindShaderProgram(m_skyboxGrProgs[skyc->getSkyboxType()].get());

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
		rgraphCtx.bindSrv(0, 0, info.m_gbufferDepthRenderTarget, info.m_gbufferDepthRenderTargetSubresource);

		TraditionalDeferredSkyboxConstants consts = {};
		consts.m_invertedViewProjectionMat = info.m_invViewProjectionMatrix;
		consts.m_cameraPos = info.m_cameraPosWSpace.xyz();
		consts.m_scale = skyc->getImageScale();
		consts.m_bias = skyc->getImageBias();

		if(skyc->getSkyboxType() == SkyboxType::kSolidColor)
		{
			consts.m_solidColor = skyc->getSolidColor();
		}
		else if(skyc->getSkyboxType() == SkyboxType::kImage2D)
		{
			cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearRepeatAniso.get());
			cmdb.bindSrv(1, 0, TextureView(&skyc->getImageResource().getTexture(), TextureSubresourceDesc::all()));
		}
		else
		{
			cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(1, 0, info.m_skyLutRenderTarget);
		}

		cmdb.bindConstantBuffer(0, 0, info.m_globalRendererConsts);

		cmdb.setFastConstants(&consts, sizeof(consts));

		drawQuad(cmdb);
	}

	// Light shading
	{
		TraditionalDeferredShadingConstants* consts = allocateAndBindConstants<TraditionalDeferredShadingConstants>(cmdb, 0, 0);

		consts->m_invViewProjMat = info.m_invViewProjectionMatrix;
		consts->m_cameraPos = info.m_cameraPosWSpace.xyz();

		if(dirLightc)
		{
			consts->m_dirLight.m_effectiveShadowDistance = info.m_effectiveShadowDistance;
			consts->m_dirLight.m_lightMatrix = info.m_dirLightMatrix;
		}

		cmdb.bindSrv(0, 0, info.m_visibleLightsBuffer);
		if(GpuSceneArrays::Light::getSingleton().getElementCount() > 0)
		{
			cmdb.bindSrv(1, 0, GpuSceneArrays::Light::getSingleton().getBufferView());
		}
		else
		{
			// Set something random
			cmdb.bindSrv(1, 0, GpuSceneBuffer::getSingleton().getBufferView());
		}

		// NOTE: Use nearest sampler because we don't want the result to sample the near tiles
		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

		rgraphCtx.bindSrv(2, 0, info.m_gbufferRenderTargets[0], info.m_gbufferRenderTargetSubresource[0]);
		rgraphCtx.bindSrv(3, 0, info.m_gbufferRenderTargets[1], info.m_gbufferRenderTargetSubresource[1]);
		rgraphCtx.bindSrv(4, 0, info.m_gbufferRenderTargets[2], info.m_gbufferRenderTargetSubresource[2]);
		rgraphCtx.bindSrv(5, 0, info.m_gbufferDepthRenderTarget, info.m_gbufferDepthRenderTargetSubresource);

		cmdb.bindSampler(1, 0, m_shadowSampler.get());
		if(dirLightc && dirLightc->getShadowEnabled())
		{
			ANKI_ASSERT(info.m_directionalLightShadowmapRenderTarget.isValid());
			rgraphCtx.bindSrv(6, 0, info.m_directionalLightShadowmapRenderTarget, info.m_directionalLightShadowmapRenderTargetSubresource);
		}
		else
		{
			// No shadows for the dir light, bind a random depth texture (need depth because validation complains)
			rgraphCtx.bindSrv(6, 0, info.m_gbufferDepthRenderTarget, info.m_gbufferDepthRenderTargetSubresource);
		}

		cmdb.bindConstantBuffer(1, 0, info.m_globalRendererConsts);

		cmdb.bindShaderProgram(m_lightGrProg[info.m_computeSpecular].get());

		drawQuad(cmdb);
	}
}

} // end namespace anki
