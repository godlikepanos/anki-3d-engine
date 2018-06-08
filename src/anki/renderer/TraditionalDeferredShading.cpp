// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/TraditionalDeferredShading.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/ShaderProgramResource.h>
#include <anki/resource/MeshResource.h>
#include <shaders/glsl_cpp_common/TraditionalDeferredShading.h>

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
		ANKI_CHECK(getResourceManager().loadResource("shaders/TraditionalDeferredShading.glslp", m_lightProg));

		ShaderProgramResourceMutationInitList<1> mutators(m_lightProg);
		mutators.add("LIGHT_TYPE", 0);

		const ShaderProgramResourceVariant* variant;
		m_lightProg->getOrCreateVariant(mutators.get(), variant);
		m_plightGrProg = variant->getProgram();

		mutators[0].m_value = 1;
		m_lightProg->getOrCreateVariant(mutators.get(), variant);
		m_slightGrProg = variant->getProgram();
	}

	// Init meshes
	ANKI_CHECK(getResourceManager().loadResource("engine_data/Plight.ankimesh", m_plightMesh, false));
	ANKI_CHECK(getResourceManager().loadResource("engine_data/Slight.ankimesh", m_slightMesh, false));

	return Error::NONE;
}

void TraditionalDeferredLightShading::bindVertexIndexBuffers(
	MeshResourcePtr& mesh, CommandBufferPtr& cmdb, U32& indexCount)
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

void TraditionalDeferredLightShading::drawLights(const Mat4& vpMat,
	const Mat4& invViewProjMat,
	const Vec4& cameraPosWSpace,
	const UVec4& viewport,
	const Vec2& gbufferTexCoordsMin,
	const Vec2& gbufferTexCoordsMax,
	ConstWeakArray<PointLightQueueElement> plights,
	ConstWeakArray<SpotLightQueueElement> slights,
	CommandBufferPtr& cmdb)
{
	ANKI_ASSERT(gbufferTexCoordsMin < gbufferTexCoordsMax);

	// Compute coords
	Vec4 inputTexUvScaleAndOffset;
	inputTexUvScaleAndOffset.x() = gbufferTexCoordsMax.x() - gbufferTexCoordsMin.x();
	inputTexUvScaleAndOffset.y() = gbufferTexCoordsMax.y() - gbufferTexCoordsMin.y();
	inputTexUvScaleAndOffset.z() = gbufferTexCoordsMin.x();
	inputTexUvScaleAndOffset.w() = gbufferTexCoordsMin.y();

	// Set common state for all lights
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);
	cmdb->setCullMode(FaceSelectionBit::FRONT);
	cmdb->setViewport(viewport.x(), viewport.y(), viewport.z(), viewport.w());

	// Do point lights
	U32 indexCount;
	bindVertexIndexBuffers(m_plightMesh, cmdb, indexCount);
	cmdb->bindShaderProgram(m_plightGrProg);

	for(const PointLightQueueElement& plightEl : plights)
	{
		// Update uniforms
		DeferredVertexUniforms* vert =
			allocateAndBindUniforms<DeferredVertexUniforms*>(sizeof(DeferredVertexUniforms), cmdb, 0, 0);

		Mat4 modelM(plightEl.m_worldPosition.xyz1(), Mat3::getIdentity(), plightEl.m_radius);

		vert->m_mvp = vpMat * modelM;

		DeferredPointLightUniforms* light =
			allocateAndBindUniforms<DeferredPointLightUniforms*>(sizeof(DeferredPointLightUniforms), cmdb, 0, 1);

		light->m_inputTexUvScaleAndOffset = inputTexUvScaleAndOffset;
		light->m_invViewProjMat = invViewProjMat;
		light->m_camPosPad1 = cameraPosWSpace.xyz0();
		light->m_fbSizePad2 = Vec4(viewport.z(), viewport.w(), 0.0f, 0.0f);
		light->m_posRadius = Vec4(plightEl.m_worldPosition.xyz(), 1.0f / (plightEl.m_radius * plightEl.m_radius));
		light->m_diffuseColorPad1 = plightEl.m_diffuseColor.xyz0();

		// Draw
		cmdb->drawElements(PrimitiveTopology::TRIANGLES, indexCount);
	}

	// Do spot lights
	bindVertexIndexBuffers(m_slightMesh, cmdb, indexCount);
	cmdb->bindShaderProgram(m_slightGrProg);

	for(const SpotLightQueueElement& splightEl : slights)
	{
		// Compute the model matrix
		//
		Mat4 modelM(
			splightEl.m_worldTransform.getTranslationPart().xyz1(), splightEl.m_worldTransform.getRotationPart(), 1.0f);

		// Calc the scale of the cone
		Mat4 scaleM(Mat4::getIdentity());
		scaleM(0, 0) = tan(splightEl.m_outerAngle / 2.0f) * splightEl.m_distance;
		scaleM(1, 1) = scaleM(0, 0);
		scaleM(2, 2) = splightEl.m_distance;

		modelM = modelM * scaleM;

		// Update vertex uniforms
		DeferredVertexUniforms* vert =
			allocateAndBindUniforms<DeferredVertexUniforms*>(sizeof(DeferredVertexUniforms), cmdb, 0, 0);
		vert->m_mvp = vpMat * modelM;

		// Update fragment uniforms
		DeferredSpotLightUniforms* light =
			allocateAndBindUniforms<DeferredSpotLightUniforms*>(sizeof(DeferredSpotLightUniforms), cmdb, 0, 1);

		light->m_inputTexUvScaleAndOffset = inputTexUvScaleAndOffset;
		light->m_invViewProjMat = invViewProjMat;
		light->m_camPosPad1 = cameraPosWSpace.xyz0();
		light->m_fbSizePad2 = Vec4(viewport.z(), viewport.w(), 0.0f, 0.0f);

		light->m_posRadius = Vec4(splightEl.m_worldTransform.getTranslationPart().xyz(),
			1.0f / (splightEl.m_distance * splightEl.m_distance));

		light->m_diffuseColorOuterCos = Vec4(splightEl.m_diffuseColor, cos(splightEl.m_outerAngle / 2.0f));

		Vec3 lightDir = -splightEl.m_worldTransform.getZAxis().xyz();
		light->m_lightDirInnerCos = Vec4(lightDir, cos(splightEl.m_innerAngle / 2.0f));

		// Draw
		cmdb->drawElements(PrimitiveTopology::TRIANGLES, indexCount);
	}

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setCullMode(FaceSelectionBit::BACK);
}

} // end namespace anki