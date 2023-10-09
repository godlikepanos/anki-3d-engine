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
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/TraditionalDeferredShading.ankiprogbin", m_lightProg));

		for(U32 specular = 0; specular <= 1; ++specular)
		{
			ShaderProgramResourceVariantInitInfo variantInitInfo(m_lightProg);
			variantInitInfo.addMutation("SPECULAR", specular);

			const ShaderProgramResourceVariant* variant;
			m_lightProg->getOrCreateVariant(variantInitInfo, variant);
			m_lightGrProg[specular].reset(&variant->getProgram());
		}
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
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/TraditionalDeferredShadingSkybox.ankiprogbin", m_skyboxProg));

		for(U32 i = 0; i < m_skyboxGrProgs.getSize(); ++i)
		{
			ShaderProgramResourceVariantInitInfo variantInitInfo(m_skyboxProg);
			variantInitInfo.addMutation("METHOD", i);
			const ShaderProgramResourceVariant* variant;
			m_skyboxProg->getOrCreateVariant(variantInitInfo, variant);
			m_skyboxGrProgs[i].reset(&variant->getProgram());
		}
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
	if(skyc)
	{
		const Bool isSolidColor = (skyc->getSkyboxType() == SkyboxType::kSolidColor);

		cmdb.bindShaderProgram(m_skyboxGrProgs[!isSolidColor].get());

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
		rgraphCtx.bindTexture(0, 1, info.m_gbufferDepthRenderTarget, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

		if(!isSolidColor)
		{
			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearRepeatAniso.get());
			cmdb.bindTexture(0, 3, &skyc->getImageResource().getTextureView());
		}

		TraditionalDeferredSkyboxConstants unis;
		unis.m_solidColor = (isSolidColor) ? skyc->getSolidColor() : Vec3(0.0f);
		unis.m_inputTexUvBias = info.m_gbufferTexCoordsBias;
		unis.m_inputTexUvScale = info.m_gbufferTexCoordsScale;
		unis.m_invertedViewProjectionMat = info.m_invViewProjectionMatrix;
		unis.m_cameraPos = info.m_cameraPosWSpace.xyz();
		cmdb.setPushConstants(&unis, sizeof(unis));

		drawQuad(cmdb);
	}

	// Light shading
	{
		const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();

		TraditionalDeferredShadingConstants* unis = allocateAndBindConstants<TraditionalDeferredShadingConstants>(cmdb, 0, 0);

		unis->m_inputTexUvScale = info.m_gbufferTexCoordsScale;
		unis->m_inputTexUvBias = info.m_gbufferTexCoordsBias;
		unis->m_fbUvScale = info.m_lightbufferTexCoordsScale;
		unis->m_fbUvBias = info.m_lightbufferTexCoordsBias;
		unis->m_invViewProjMat = info.m_invViewProjectionMatrix;
		unis->m_cameraPos = info.m_cameraPosWSpace.xyz();

		if(dirLightc)
		{
			unis->m_dirLight.m_diffuseColor = dirLightc->getDiffuseColor().xyz();
			unis->m_dirLight.m_active = 1;
			unis->m_dirLight.m_direction = dirLightc->getDirection();
			unis->m_dirLight.m_effectiveShadowDistance = info.m_effectiveShadowDistance;
			unis->m_dirLight.m_lightMatrix = info.m_dirLightMatrix;
		}
		else
		{
			unis->m_dirLight.m_active = 0;
		}

		cmdb.bindUavBuffer(0, 1, info.m_visibleLightsBuffer.m_buffer, info.m_visibleLightsBuffer.m_offset, info.m_visibleLightsBuffer.m_range);
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

		rgraphCtx.bindColorTexture(0, 4, info.m_gbufferRenderTargets[0]);
		rgraphCtx.bindColorTexture(0, 5, info.m_gbufferRenderTargets[1]);
		rgraphCtx.bindColorTexture(0, 6, info.m_gbufferRenderTargets[2]);
		rgraphCtx.bindTexture(0, 7, info.m_gbufferDepthRenderTarget, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

		cmdb.bindSampler(0, 8, m_shadowSampler.get());
		if(dirLightc && dirLightc->getShadowEnabled())
		{
			ANKI_ASSERT(info.m_directionalLightShadowmapRenderTarget.isValid());
			rgraphCtx.bindTexture(0, 9, info.m_directionalLightShadowmapRenderTarget, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		}
		else
		{
			// No shadows for the dir light, bind a random depth texture (need depth because validation complains)
			rgraphCtx.bindTexture(0, 9, info.m_gbufferDepthRenderTarget, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		}

		cmdb.bindShaderProgram(m_lightGrProg[info.m_computeSpecular].get());

		drawQuad(cmdb);
	}
}

} // end namespace anki
