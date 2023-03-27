// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/TraditionalDeferredShading.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Shaders/Include/TraditionalDeferredShadingTypes.h>

namespace anki {

// Vertices of 2 shapes
// - Use the blend file in EngineAssets to output in .obj and then copy paste here
// - The sphere goes from -1 to 1
// - The cone's length is 1. Its big side goes from -1 to 1. The point of the cone is in 0,0,0. The direction is in +z

inline constexpr F32 kIcosphereVertices[] = {
	0.000000f,  -1.067367f, 0.000000f,  0.772355f,  -0.477347f, 0.561142f,  -0.295008f, -0.477348f, 0.907955f,
	-0.954681f, -0.477343f, 0.000000f,  -0.295008f, -0.477348f, -0.907955f, 0.772355f,  -0.477347f, -0.561142f,
	0.295008f,  0.477348f,  0.907955f,  -0.772355f, 0.477347f,  0.561142f,  -0.772355f, 0.477347f,  -0.561142f,
	0.295008f,  0.477348f,  -0.907955f, 0.954681f,  0.477343f,  0.000000f,  0.000000f,  1.067367f,  0.000000f,
	-0.173400f, -0.907961f, 0.533679f,  0.453976f,  -0.907960f, 0.329829f,  0.280578f,  -0.561155f, 0.863513f,
	0.907954f,  -0.561153f, 0.000000f,  0.453976f,  -0.907960f, -0.329829f, -0.561147f, -0.907958f, 0.000000f,
	-0.734551f, -0.561154f, 0.533680f,  -0.173400f, -0.907961f, -0.533679f, -0.734551f, -0.561154f, -0.533680f,
	0.280578f,  -0.561155f, -0.863513f, 1.015128f,  0.000000f,  0.329830f,  1.015128f,  0.000000f,  -0.329830f,
	0.000000f,  0.000000f,  1.067367f,  0.627383f,  0.000000f,  0.863518f,  -1.015128f, 0.000000f,  0.329830f,
	-0.627383f, 0.000000f,  0.863518f,  -0.627383f, 0.000000f,  -0.863518f, -1.015128f, 0.000000f,  -0.329830f,
	0.627383f,  0.000000f,  -0.863518f, 0.000000f,  0.000000f,  -1.067367f, 0.734551f,  0.561154f,  0.533680f,
	-0.280578f, 0.561155f,  0.863513f,  -0.907954f, 0.561153f,  0.000000f,  -0.280578f, 0.561155f,  -0.863513f,
	0.734551f,  0.561154f,  -0.533680f, 0.173400f,  0.907961f,  0.533679f,  0.561147f,  0.907958f,  0.000000f,
	-0.453976f, 0.907960f,  0.329829f,  -0.453976f, 0.907960f,  -0.329829f, 0.173400f,  0.907961f,  -0.533679f};

inline constexpr U16 kIconsphereIndices[] = {
	0,  13, 12, 1,  13, 15, 0,  12, 17, 0,  17, 19, 0,  19, 16, 1,  15, 22, 2,  14, 24, 3,  18, 26, 4,  20, 28,
	5,  21, 30, 1,  22, 25, 2,  24, 27, 3,  26, 29, 4,  28, 31, 5,  30, 23, 6,  32, 37, 7,  33, 39, 8,  34, 40,
	9,  35, 41, 10, 36, 38, 38, 41, 11, 38, 36, 41, 36, 9,  41, 41, 40, 11, 41, 35, 40, 35, 8,  40, 40, 39, 11,
	40, 34, 39, 34, 7,  39, 39, 37, 11, 39, 33, 37, 33, 6,  37, 37, 38, 11, 37, 32, 38, 32, 10, 38, 23, 36, 10,
	23, 30, 36, 30, 9,  36, 31, 35, 9,  31, 28, 35, 28, 8,  35, 29, 34, 8,  29, 26, 34, 26, 7,  34, 27, 33, 7,
	27, 24, 33, 24, 6,  33, 25, 32, 6,  25, 22, 32, 22, 10, 32, 30, 31, 9,  30, 21, 31, 21, 4,  31, 28, 29, 8,
	28, 20, 29, 20, 3,  29, 26, 27, 7,  26, 18, 27, 18, 2,  27, 24, 25, 6,  24, 14, 25, 14, 1,  25, 22, 23, 10,
	22, 15, 23, 15, 5,  23, 16, 21, 5,  16, 19, 21, 19, 4,  21, 19, 20, 4,  19, 17, 20, 17, 3,  20, 17, 18, 3,
	17, 12, 18, 12, 2,  18, 15, 16, 5,  15, 13, 16, 13, 0,  16, 12, 14, 2,  12, 13, 14, 13, 1,  14};

inline constexpr F32 kConeVertices[] = {
	0.000000f,  1.000000f,  -1.000000f, 0.000000f,  0.000000f,  0.000000f,  0.500000f,  0.866026f,  -1.000000f,
	0.866025f,  0.500000f,  -1.000000f, 1.000000f,  0.000000f,  -1.000000f, 0.866025f,  -0.500000f, -1.000000f,
	0.500000f,  -0.866025f, -1.000000f, 0.000000f,  -1.000000f, -1.000000f, -0.500000f, -0.866025f, -1.000000f,
	-0.866025f, -0.500000f, -1.000000f, -1.000000f, -0.000000f, -1.000000f, -0.866026f, 0.500000f,  -1.000000f,
	-0.500001f, 0.866025f,  -1.000000f, 0.000000f,  0.000000f,  -1.000000f};

inline constexpr U16 kConeIndices[] = {0, 1,  2, 2,  1,  3,  3,  1,  4,  4,  1,  5,  5,  1,  6,  6,  1,  7,
									   7, 1,  8, 8,  1,  9,  9,  1,  10, 10, 1,  11, 11, 1,  12, 12, 1,  0,
									   2, 13, 0, 0,  13, 12, 3,  13, 2,  4,  13, 3,  5,  13, 4,  6,  13, 5,
									   7, 13, 6, 12, 13, 11, 11, 13, 10, 10, 13, 9,  9,  13, 8,  7,  8,  13};

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
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/TraditionalDeferredShading.ankiprogbin",
																m_lightProg));

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

	createProxyMeshes();

	// Shadow sampler
	{
		SamplerInitInfo inf;
		inf.m_compareOperation = CompareOperation::kLessEqual;
		inf.m_addressing = SamplingAddressing::kClamp;
		inf.m_mipmapFilter = SamplingFilter::kBase;
		inf.m_minMagFilter = SamplingFilter::kLinear;
		m_shadowSampler = getExternalSubsystems().m_grManager->newSampler(inf);
	}

	// Skybox
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(
			"ShaderBinaries/TraditionalDeferredShadingSkybox.ankiprogbin", m_skyboxProg));

		for(U32 i = 0; i < m_skyboxGrProgs.getSize(); ++i)
		{
			ShaderProgramResourceVariantInitInfo variantInitInfo(m_skyboxProg);
			variantInitInfo.addMutation("METHOD", i);
			const ShaderProgramResourceVariant* variant;
			m_skyboxProg->getOrCreateVariant(variantInitInfo, variant);
			m_skyboxGrProgs[i] = variant->getProgram();
		}
	}

	return Error::kNone;
}

void TraditionalDeferredLightShading::createProxyMeshes()
{
	constexpr PtrSize bufferSize =
		sizeof(kIcosphereVertices) + sizeof(kIcosphereVertices) + sizeof(kConeVertices) + sizeof(kConeIndices);

	BufferInitInfo buffInit("DeferredShadingPoxies");
	buffInit.m_size = bufferSize;
	buffInit.m_usage = BufferUsageBit::kTransferDestination | BufferUsageBit::kIndex | BufferUsageBit::kVertex;

	m_proxyVolumesBuffer = getExternalSubsystems().m_grManager->newBuffer(buffInit);

	buffInit.setName("TempTransfer");
	buffInit.m_usage = BufferUsageBit::kTransferSource;
	buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
	BufferPtr transferBuffer = getExternalSubsystems().m_grManager->newBuffer(buffInit);

	U8* mappedMem = static_cast<U8*>(transferBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

	// Write verts
	memcpy(mappedMem, kIcosphereVertices, sizeof(kIcosphereVertices));
	mappedMem += sizeof(kIcosphereVertices);
	memcpy(mappedMem, kConeVertices, sizeof(kConeVertices));
	mappedMem += sizeof(kConeVertices);
	memcpy(mappedMem, kIconsphereIndices, sizeof(kIconsphereIndices));
	mappedMem += sizeof(kIconsphereIndices);
	memcpy(mappedMem, kConeIndices, sizeof(kConeIndices));

	transferBuffer->unmap();

	CommandBufferInitInfo cmdbInit("Temp");
	cmdbInit.m_flags = CommandBufferFlag::kSmallBatch | CommandBufferFlag::kGeneralWork;

	CommandBufferPtr cmdb = getExternalSubsystems().m_grManager->newCommandBuffer(cmdbInit);
	cmdb->copyBufferToBuffer(transferBuffer, 0, m_proxyVolumesBuffer, 0, bufferSize);
	cmdb->flush();

	getExternalSubsystems().m_grManager->finish();
}

void TraditionalDeferredLightShading::bindVertexIndexBuffers(ProxyType proxyType, CommandBufferPtr& cmdb,
															 U32& indexCount) const
{
	// Attrib
	cmdb->setVertexAttribute(0, 0, Format::kR32G32B32_Sfloat, 0);

	// Vert buff
	PtrSize offset;
	if(proxyType == ProxyType::kProxySphere)
	{
		offset = 0;
	}
	else
	{
		offset = sizeof(kIcosphereVertices);
	}
	cmdb->bindVertexBuffer(0, m_proxyVolumesBuffer, offset, sizeof(Vec3));

	// Idx buff
	if(proxyType == ProxyType::kProxySphere)
	{
		offset = sizeof(kIcosphereVertices) + sizeof(kConeVertices);
		indexCount = sizeof(kIconsphereIndices) / sizeof(U16);
	}
	else
	{
		offset = sizeof(kIcosphereVertices) + sizeof(kConeVertices) + sizeof(kIconsphereIndices);
		indexCount = sizeof(kConeIndices) / sizeof(U16);
	}

	cmdb->bindIndexBuffer(m_proxyVolumesBuffer, offset, IndexType::kU16);
}

void TraditionalDeferredLightShading::drawLights(TraditionalDeferredLightShadingDrawInfo& info)
{
	CommandBufferPtr& cmdb = info.m_commandBuffer;
	RenderPassWorkContext& rgraphCtx = *info.m_renderpassContext;

	// Set common state for all
	cmdb->setViewport(info.m_viewport.x(), info.m_viewport.y(), info.m_viewport.z(), info.m_viewport.w());

	// Skybox first
	{
		const Bool isSolidColor = info.m_skybox->m_skyboxTexture == nullptr;

		cmdb->bindShaderProgram(m_skyboxGrProgs[!isSolidColor]);

		cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);
		rgraphCtx.bindTexture(0, 1, info.m_gbufferDepthRenderTarget,
							  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

		if(!isSolidColor)
		{
			cmdb->bindSampler(0, 2, m_r->getSamplers().m_trilinearRepeatAniso);
			cmdb->bindTexture(0, 3, TextureViewPtr(const_cast<TextureView*>(info.m_skybox->m_skyboxTexture)));
		}

		DeferredSkyboxUniforms unis;
		unis.m_solidColor = info.m_skybox->m_solidColor;
		unis.m_inputTexUvBias = info.m_gbufferTexCoordsBias;
		unis.m_inputTexUvScale = info.m_gbufferTexCoordsScale;
		unis.m_invertedViewProjectionMat = info.m_invViewProjectionMatrix;
		unis.m_cameraPos = info.m_cameraPosWSpace.xyz();
		cmdb->setPushConstants(&unis, sizeof(unis));

		drawQuad(cmdb);
	}

	// Set common state for all light drawcalls
	{
		cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kOne);

		// NOTE: Use nearest sampler because we don't want the result to sample the near tiles
		cmdb->bindSampler(0, 2, m_r->getSamplers().m_nearestNearestClamp);

		rgraphCtx.bindColorTexture(0, 3, info.m_gbufferRenderTargets[0]);
		rgraphCtx.bindColorTexture(0, 4, info.m_gbufferRenderTargets[1]);
		rgraphCtx.bindColorTexture(0, 5, info.m_gbufferRenderTargets[2]);

		rgraphCtx.bindTexture(0, 6, info.m_gbufferDepthRenderTarget,
							  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

		// Set shadowmap resources
		cmdb->bindSampler(0, 7, m_shadowSampler);

		if(info.m_directionalLight && info.m_directionalLight->hasShadow())
		{
			ANKI_ASSERT(info.m_directionalLightShadowmapRenderTarget.isValid());

			rgraphCtx.bindTexture(0, 8, info.m_directionalLightShadowmapRenderTarget,
								  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		}
		else
		{
			// No shadows for the dir light, bind something random
			rgraphCtx.bindColorTexture(0, 8, info.m_gbufferRenderTargets[0]);
		}
	}

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
				info.m_directionalLight->m_shadowCascadesDistances[info.m_directionalLight->m_shadowCascadeCount - 1];
		}
		else
		{
			unis->m_effectiveShadowDistance = 0.0f;
		}

		drawQuad(cmdb);
	}

	// Set other light state
	cmdb->setCullMode(FaceSelectionBit::kFront);

	// Do point lights
	U32 indexCount;
	bindVertexIndexBuffers(ProxyType::kProxySphere, cmdb, indexCount);
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
		cmdb->drawElements(PrimitiveTopology::kTriangles, indexCount);
	}

	// Do spot lights
	bindVertexIndexBuffers(ProxyType::kProxyCone, cmdb, indexCount);
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
		cmdb->drawElements(PrimitiveTopology::kTriangles, indexCount);
	}

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	cmdb->setCullMode(FaceSelectionBit::kBack);
}

} // end namespace anki
