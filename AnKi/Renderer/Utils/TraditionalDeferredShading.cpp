// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
		inf.m_mipmapFilter = SamplingFilter::kBase;
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
		rgraphCtx.bindTexture(0, 1, info.m_gbufferDepthRenderTarget, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

		TraditionalDeferredSkyboxConstants unis = {};
		unis.m_invertedViewProjectionMat = info.m_invViewProjectionMatrix;
		unis.m_cameraPos = info.m_cameraPosWSpace.xyz();
		unis.m_scale = skyc->getImageScale();
		unis.m_bias = skyc->getImageBias();

		if(skyc->getSkyboxType() == SkyboxType::kSolidColor)
		{
			unis.m_solidColor = skyc->getSolidColor();
		}
		else if(skyc->getSkyboxType() == SkyboxType::kImage2D)
		{
			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearRepeatAniso.get());
			cmdb.bindTexture(0, 3, &skyc->getImageResource().getTextureView());
		}
		else
		{
			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindColorTexture(0, 3, info.m_skyLutRenderTarget);
		}

		cmdb.bindConstantBuffer(0, 4, info.m_globalRendererConsts);

		cmdb.setPushConstants(&unis, sizeof(unis));

		drawQuad(cmdb);
	}

	// Light shading
	{
		TraditionalDeferredShadingConstants* unis = allocateAndBindConstants<TraditionalDeferredShadingConstants>(cmdb, 0, 0);

		unis->m_invViewProjMat = info.m_invViewProjectionMatrix;
		unis->m_cameraPos = info.m_cameraPosWSpace.xyz();

		if(dirLightc)
		{
			unis->m_dirLight.m_effectiveShadowDistance = info.m_effectiveShadowDistance;
			unis->m_dirLight.m_lightMatrix = info.m_dirLightMatrix;
		}

		cmdb.bindUavBuffer(0, 1, info.m_visibleLightsBuffer);
		if(GpuSceneArrays::Light::getSingleton().getElementCount() > 0)
		{
			cmdb.bindUavBuffer(0, 2, GpuSceneArrays::Light::getSingleton().getBufferOffsetRange());
		}
		else
		{
			// Set something random
			cmdb.bindUavBuffer(0, 2, GpuSceneBuffer::getSingleton().getBufferOffsetRange());
		}

		// NOTE: Use nearest sampler because we don't want the result to sample the near tiles
		cmdb.bindSampler(0, 3, getRenderer().getSamplers().m_nearestNearestClamp.get());

		rgraphCtx.bindTexture(0, 4, info.m_gbufferRenderTargets[0], info.m_gbufferRenderTargetSubresourceInfos[0]);
		rgraphCtx.bindTexture(0, 5, info.m_gbufferRenderTargets[1], info.m_gbufferRenderTargetSubresourceInfos[1]);
		rgraphCtx.bindTexture(0, 6, info.m_gbufferRenderTargets[2], info.m_gbufferRenderTargetSubresourceInfos[2]);
		rgraphCtx.bindTexture(0, 7, info.m_gbufferDepthRenderTarget, info.m_gbufferDepthRenderTargetSubresourceInfo);

		cmdb.bindSampler(0, 8, m_shadowSampler.get());
		if(dirLightc && dirLightc->getShadowEnabled())
		{
			ANKI_ASSERT(info.m_directionalLightShadowmapRenderTarget.isValid());
			rgraphCtx.bindTexture(0, 9, info.m_directionalLightShadowmapRenderTarget, info.m_directionalLightShadowmapRenderTargetSubresourceInfo);
		}
		else
		{
			// No shadows for the dir light, bind a random depth texture (need depth because validation complains)
			rgraphCtx.bindTexture(0, 9, info.m_gbufferDepthRenderTarget, info.m_gbufferDepthRenderTargetSubresourceInfo);
		}

		cmdb.bindConstantBuffer(0, 10, info.m_globalRendererConsts);

		cmdb.bindShaderProgram(m_lightGrProg[info.m_computeSpecular].get());

		drawQuad(cmdb);
	}
}

} // end namespace anki
