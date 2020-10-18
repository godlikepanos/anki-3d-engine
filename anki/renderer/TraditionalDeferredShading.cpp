// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/TraditionalDeferredShading.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/MeshResource.h>
#include <anki/shaders/include/TraditionalDeferredShadingTypes.h>

namespace anki
{

TraditionalDeferredLightShading::TraditionalDeferredLightShading(Renderer* r)
	: RendererObject(r)
{
}

TraditionalDeferredLightShading::~TraditionalDeferredLightShading()
{
}

Error TraditionalDeferredLightShading::init()
{
	// Init progs
	{
		ANKI_CHECK(getResourceManager().loadResource("shaders/TraditionalDeferredShading.ankiprog", m_lightProg));

		for(U32 specular = 0; specular <= 1; ++specular)
		{
			ShaderProgramResourceVariantInitInfo variantInitInfo(m_lightProg);
			variantInitInfo.addMutation("LIGHT_TYPE", 0);
			variantInitInfo.addMutation("SPECULAR", specular);

			const ShaderProgramResourceVariant* variant;
			m_lightProg->getOrCreateVariant(variantInitInfo, variant);
			m_plightGrProg[specular] = variant->getProgram();

			variantInitInfo.addMutation("LIGHT_TYPE", 1);
			m_lightProg->getOrCreateVariant(variantInitInfo, variant);
			m_slightGrProg[specular] = variant->getProgram();

			variantInitInfo.addMutation("LIGHT_TYPE", 2);
			m_lightProg->getOrCreateVariant(variantInitInfo, variant);
			m_dirLightGrProg[specular] = variant->getProgram();
		}
	}

	// Init meshes
	ANKI_CHECK(getResourceManager().loadResource("engine_data/Plight.ankimesh", m_plightMesh, false));
	ANKI_CHECK(getResourceManager().loadResource("engine_data/Slight.ankimesh", m_slightMesh, false));

	return Error::NONE;
}

void TraditionalDeferredLightShading::bindVertexIndexBuffers(MeshResourcePtr& mesh, CommandBufferPtr& cmdb,
															 U32& indexCount)
{
	// Attrib
	U32 bufferBinding;
	Format fmt;
	PtrSize relativeOffset;
	mesh->getVertexAttributeInfo(VertexAttributeLocation::POSITION, bufferBinding, fmt, relativeOffset);

	cmdb->setVertexAttribute(0, 0, fmt, relativeOffset);

	// Vert buff
	BufferPtr buff;
	PtrSize offset, stride;
	mesh->getVertexBufferInfo(bufferBinding, buff, offset, stride);

	cmdb->bindVertexBuffer(0, buff, offset, stride);

	// Idx buff
	IndexType idxType;
	mesh->getIndexBufferInfo(buff, offset, indexCount, idxType);

	cmdb->bindIndexBuffer(buff, offset, idxType);
}

void TraditionalDeferredLightShading::drawLights(TraditionalDeferredLightShadingDrawInfo& info)
{
	// Set common state for all lights
	CommandBufferPtr& cmdb = info.m_commandBuffer;
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);
	cmdb->setViewport(info.m_viewport.x(), info.m_viewport.y(), info.m_viewport.z(), info.m_viewport.w());

	// Dir light
	if(info.m_directionalLight)
	{
		ANKI_ASSERT(info.m_directionalLight->m_uuid && info.m_directionalLight->m_shadowCascadeCount == 1);

		cmdb->bindShaderProgram(m_dirLightGrProg[info.m_computeSpecular]);

		DeferredDirectionalLightUniforms* unis = allocateAndBindUniforms<DeferredDirectionalLightUniforms*>(
			sizeof(DeferredDirectionalLightUniforms), cmdb, 0, 1);

		unis->m_inputTexUvScale = info.m_gbufferTexCoordsScale;
		unis->m_inputTexUvBias = info.m_gbufferTexCoordsBias;
		unis->m_fbUvScale = info.m_lightbufferTexCoordsScale;
		unis->m_fbUvBias = info.m_lightbufferTexCoordsBias;
		unis->m_invViewProjMat = info.m_invViewProjectionMatrix;
		unis->m_camPos = info.m_cameraPosWSpace.xyz();

		unis->m_diffuseColor = info.m_directionalLight->m_diffuseColor;
		unis->m_lightDir = info.m_directionalLight->m_direction;
		unis->m_lightMatrix = info.m_directionalLight->m_textureMatrices[0];

		unis->m_near = info.m_cameraNear;
		unis->m_far = info.m_cameraFar;

		if(info.m_directionalLight->m_shadowCascadeCount > 0)
		{
			unis->m_effectiveShadowDistance =
				info.m_directionalLight->m_shadowRenderQueues[0]->m_effectiveShadowDistance;
		}
		else
		{
			unis->m_effectiveShadowDistance = 0.0f;
		}

		drawQuad(cmdb);
	}

	// Set other light state
	cmdb->setCullMode(FaceSelectionBit::FRONT);

	// Do point lights
	U32 indexCount;
	bindVertexIndexBuffers(m_plightMesh, cmdb, indexCount);
	cmdb->bindShaderProgram(m_plightGrProg[info.m_computeSpecular]);

	for(const PointLightQueueElement& plightEl : info.m_pointLights)
	{
		// Update uniforms
		DeferredVertexUniforms* vert =
			allocateAndBindUniforms<DeferredVertexUniforms*>(sizeof(DeferredVertexUniforms), cmdb, 0, 0);

		Mat4 modelM(plightEl.m_worldPosition.xyz1(), Mat3::getIdentity(), plightEl.m_radius);

		vert->m_mvp = info.m_viewProjectionMatrix * modelM;

		DeferredPointLightUniforms* light =
			allocateAndBindUniforms<DeferredPointLightUniforms*>(sizeof(DeferredPointLightUniforms), cmdb, 0, 1);

		light->m_inputTexUvScale = info.m_gbufferTexCoordsScale;
		light->m_inputTexUvBias = info.m_gbufferTexCoordsBias;
		light->m_fbUvScale = info.m_lightbufferTexCoordsScale;
		light->m_fbUvBias = info.m_lightbufferTexCoordsBias;
		light->m_invViewProjMat = info.m_invViewProjectionMatrix;
		light->m_camPos = info.m_cameraPosWSpace.xyz();
		light->m_position = plightEl.m_worldPosition;
		light->m_oneOverSquareRadius = 1.0f / (plightEl.m_radius * plightEl.m_radius);
		light->m_diffuseColor = plightEl.m_diffuseColor;

		// Draw
		cmdb->drawElements(PrimitiveTopology::TRIANGLES, indexCount);
	}

	// Do spot lights
	bindVertexIndexBuffers(m_slightMesh, cmdb, indexCount);
	cmdb->bindShaderProgram(m_slightGrProg[info.m_computeSpecular]);

	for(const SpotLightQueueElement& splightEl : info.m_spotLights)
	{
		// Compute the model matrix
		//
		Mat4 modelM(splightEl.m_worldTransform.getTranslationPart().xyz1(),
					splightEl.m_worldTransform.getRotationPart(), 1.0f);

		// Calc the scale of the cone
		Mat4 scaleM(Mat4::getIdentity());
		scaleM(0, 0) = tan(splightEl.m_outerAngle / 2.0f) * splightEl.m_distance;
		scaleM(1, 1) = scaleM(0, 0);
		scaleM(2, 2) = splightEl.m_distance;

		modelM = modelM * scaleM;

		// Update vertex uniforms
		DeferredVertexUniforms* vert =
			allocateAndBindUniforms<DeferredVertexUniforms*>(sizeof(DeferredVertexUniforms), cmdb, 0, 0);
		vert->m_mvp = info.m_viewProjectionMatrix * modelM;

		// Update fragment uniforms
		DeferredSpotLightUniforms* light =
			allocateAndBindUniforms<DeferredSpotLightUniforms*>(sizeof(DeferredSpotLightUniforms), cmdb, 0, 1);

		light->m_inputTexUvScale = info.m_gbufferTexCoordsScale;
		light->m_inputTexUvBias = info.m_gbufferTexCoordsBias;
		light->m_fbUvScale = info.m_lightbufferTexCoordsScale;
		light->m_fbUvBias = info.m_lightbufferTexCoordsBias;
		light->m_invViewProjMat = info.m_invViewProjectionMatrix;
		light->m_camPos = info.m_cameraPosWSpace.xyz();

		light->m_position = splightEl.m_worldTransform.getTranslationPart().xyz();
		light->m_oneOverSquareRadius = 1.0f / (splightEl.m_distance * splightEl.m_distance);

		light->m_diffuseColor = splightEl.m_diffuseColor;
		light->m_outerCos = cos(splightEl.m_outerAngle / 2.0f);

		light->m_lightDir = -splightEl.m_worldTransform.getZAxis().xyz();
		light->m_innerCos = cos(splightEl.m_innerAngle / 2.0f);

		// Draw
		cmdb->drawElements(PrimitiveTopology::TRIANGLES, indexCount);
	}

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setCullMode(FaceSelectionBit::BACK);
}

} // end namespace anki
