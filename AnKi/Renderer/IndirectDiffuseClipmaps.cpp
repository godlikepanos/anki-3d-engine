// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuseClipmaps.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/Sky.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>

namespace anki {

static void computeClipmapBounds(Vec3 cameraPos, Vec3 lookDir, Clipmap& clipmap)
{
	const Vec3 offset = lookDir * kIndirectDiffuseClipmapForwardBias * F32(clipmap.m_index + 1);
	cameraPos += offset;

	const Vec3 halfSize = clipmap.m_size * 0.5;
	const Vec3 probeSize = clipmap.m_size / Vec3(clipmap.m_probeCounts);
	const Vec3 roundedPos = (cameraPos / probeSize).round() * probeSize;
	clipmap.m_aabbMin = roundedPos - halfSize;
	[[maybe_unused]] const Vec3 aabbMax = roundedPos + halfSize;
	ANKI_ASSERT(aabbMax - clipmap.m_aabbMin == clipmap.m_size);
}

Error IndirectDiffuseClipmaps::init()
{
	ANKI_CHECK(RtMaterialFetchRendererObject::init());

	m_appliedGiRtDesc =
		getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
													  getRenderer().getHdrFormat(), "IndirectDiffuseClipmap: Final");
	m_appliedGiRtDesc.bake();

	m_clipmapInfo[0].m_probeCounts =
		UVec3(g_indirectDiffuseClipmapProbesXZCVar, g_indirectDiffuseClipmapProbesYCVar, g_indirectDiffuseClipmapProbesXZCVar);
	m_clipmapInfo[1].m_probeCounts = m_clipmapInfo[0].m_probeCounts;
	m_clipmapInfo[2].m_probeCounts = m_clipmapInfo[0].m_probeCounts;
	m_clipmapInfo[0].m_size = Vec3(g_indirectDiffuseClipmap0XZSizeCVar, g_indirectDiffuseClipmap0YSizeCVar, g_indirectDiffuseClipmap0XZSizeCVar);
	m_clipmapInfo[1].m_size = Vec3(g_indirectDiffuseClipmap1XZSizeCVar, g_indirectDiffuseClipmap1YSizeCVar, g_indirectDiffuseClipmap1XZSizeCVar);
	m_clipmapInfo[2].m_size = Vec3(g_indirectDiffuseClipmap2XZSizeCVar, g_indirectDiffuseClipmap2YSizeCVar, g_indirectDiffuseClipmap2XZSizeCVar);

	U32 probesPerClipmap = 0;
	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		const U32 count = m_clipmapInfo[i].m_probeCounts.x() * m_clipmapInfo[i].m_probeCounts.y() * m_clipmapInfo[i].m_probeCounts.z();
		m_clipmapInfo[i].m_probeCountTotal = count;
		m_clipmapInfo[i].m_index = i;
		if(i == 0)
		{
			probesPerClipmap = count;
		}
		else
		{
			ANKI_ASSERT(probesPerClipmap == count);
		}
	}

	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		m_probeValidityRtDescs[i] =
			getRenderer().create2DRenderTargetDescription(m_clipmapInfo[i].m_probeCounts.x(), m_clipmapInfo[i].m_probeCounts.z(), Format::kR8_Unorm,
														  generateTempPassName("IndirectDiffuseClipmap: Probe validity #%u", i));
		m_probeValidityRtDescs[i].m_depth = m_clipmapInfo[i].m_probeCounts.y();
		m_probeValidityRtDescs[i].m_type = TextureType::k3D;
		m_probeValidityRtDescs[i].bake();
	}

	// Create the lighting result texture
	const U32 raysPerProbePerFrame = square(g_indirectDiffuseClipmapRadianceOctMapSize / 2);
	m_rtResultRtDesc = getRenderer().create2DRenderTargetDescription(probesPerClipmap, raysPerProbePerFrame * kIndirectDiffuseClipmapCount,
																	 Format::kR16G16B16A16_Sfloat, "IndirectDiffuseClipmap: RT result");
	m_rtResultRtDesc.bake();

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_clipmapInfo[clipmap].m_probeCounts.x() * (g_indirectDiffuseClipmapRadianceOctMapSize + 2),
			m_clipmapInfo[clipmap].m_probeCounts.z() * (g_indirectDiffuseClipmapRadianceOctMapSize + 2), Format::kB10G11R11_Ufloat_Pack32,
			TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Radiance #%u", clipmap));
		volumeInit.m_depth = m_clipmapInfo[clipmap].m_probeCounts.y();
		volumeInit.m_type = TextureType::k3D;

		m_radianceVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_clipmapInfo[clipmap].m_probeCounts.x() * (g_indirectDiffuseClipmapIrradianceOctMapSize + 2),
			m_clipmapInfo[clipmap].m_probeCounts.z() * (g_indirectDiffuseClipmapIrradianceOctMapSize + 2), Format::kB10G11R11_Ufloat_Pack32,
			TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Irradiance #%u", clipmap));
		volumeInit.m_depth = m_clipmapInfo[clipmap].m_probeCounts.y();
		volumeInit.m_type = TextureType::k3D;

		m_irradianceVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_clipmapInfo[clipmap].m_probeCounts.x() * (g_indirectDiffuseClipmapRadianceOctMapSize + 2),
			m_clipmapInfo[clipmap].m_probeCounts.z() * (g_indirectDiffuseClipmapRadianceOctMapSize + 2), Format::kR16G16_Sfloat,
			TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Dist moments #%u", clipmap));
		volumeInit.m_depth = m_clipmapInfo[clipmap].m_probeCounts.y();
		volumeInit.m_type = TextureType::k3D;

		m_distanceMomentsVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	const Array<SubMutation, 4> mutation = {{{"GPU_WAVE_SIZE", MutatorValue(GrManager::getSingleton().getDeviceCapabilities().m_maxWaveSize)},
											 {"RADIANCE_OCTAHEDRON_MAP_SIZE", MutatorValue(g_indirectDiffuseClipmapRadianceOctMapSize)},
											 {"IRRADIANCE_OCTAHEDRON_MAP_SIZE", MutatorValue(g_indirectDiffuseClipmapIrradianceOctMapSize)},
											 {"RT_MATERIAL_FETCH_CLIPMAP", 0}}};

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_applyGiGrProg, "Apply"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_visProbesGrProg, "VisualizeProbes"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_populateCachesGrProg, "PopulateCaches"));
	ANKI_CHECK(
		loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_computeIrradianceGrProg, "ComputeIrradiance"));

	for(MutatorValue rtMaterialFetchClipmap = 0; rtMaterialFetchClipmap < 2; ++rtMaterialFetchClipmap)
	{
		ShaderProgramResourcePtr tmpProg;
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", tmpProg));
		ANKI_ASSERT(tmpProg == m_prog);

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kRayGen, "RtMaterialFetch");
		for(const SubMutation& s : mutation)
		{
			variantInitInfo.addMutation(s.m_mutatorName, s.m_value);
		}

		variantInitInfo.addMutation("RT_MATERIAL_FETCH_CLIPMAP", rtMaterialFetchClipmap);

		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_rtLibraryGrProg.reset(&variant->getProgram());
		m_rayGenShaderGroupIndices[rtMaterialFetchClipmap] = variant->getShaderGroupHandleIndex();
	}

	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/RtMaterialFetchMiss.ankiprogbin", m_missProg));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_missProg);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kMiss, "RtMaterialFetch");
		const ShaderProgramResourceVariant* variant;
		m_missProg->getOrCreateVariant(variantInitInfo, variant);
		m_missShaderGroupIdx = variant->getShaderGroupHandleIndex();
	}

	m_sbtRecordSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment,
										GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize + U32(sizeof(UVec4)));

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_blueNoiseImg));

	return Error::kNone;
}

void IndirectDiffuseClipmaps::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuse);

	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		m_clipmapInfo[i].m_prevFrameAabbMin = m_clipmapInfo[i].m_aabbMin;

		computeClipmapBounds(ctx.m_matrices.m_cameraTransform.getTranslationPart(),
							 -ctx.m_matrices.m_cameraTransform.getRotationPart().getZAxis().normalize(), m_clipmapInfo[i]);
	}

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	const RenderTargetHandle rtResultHandle = rgraph.newRenderTarget(m_rtResultRtDesc);
	m_runCtx.m_appliedGiRt = rgraph.newRenderTarget(m_appliedGiRtDesc);

	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> radianceVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> irradianceVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> distanceMomentsVolumes;
	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		if(m_texturesImportedOnce)
		{
			radianceVolumes[i] = rgraph.importRenderTarget(m_radianceVolumes[i].get());
			irradianceVolumes[i] = rgraph.importRenderTarget(m_irradianceVolumes[i].get());
			distanceMomentsVolumes[i] = rgraph.importRenderTarget(m_distanceMomentsVolumes[i].get());
		}
		else
		{
			radianceVolumes[i] = rgraph.importRenderTarget(m_radianceVolumes[i].get(), TextureUsageBit::kSrvCompute);
			irradianceVolumes[i] = rgraph.importRenderTarget(m_irradianceVolumes[i].get(), TextureUsageBit::kSrvCompute);
			distanceMomentsVolumes[i] = rgraph.importRenderTarget(m_distanceMomentsVolumes[i].get(), TextureUsageBit::kSrvCompute);
		}
	}

	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> probeValidityVolumes;
	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		probeValidityVolumes[i] = rgraph.newRenderTarget(m_probeValidityRtDescs[i]);
	}
	m_runCtx.m_probeValidityRts = probeValidityVolumes;

	// SBT build
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	buildShaderBindingTablePass("IndirectDiffuseClipmaps: Build SBT", m_rtLibraryGrProg.get(), m_rayGenShaderGroupIndices[1], m_missShaderGroupIdx,
								m_sbtRecordSize, rgraph, sbtHandle, sbtBuffer);

	// Do ray tracing around the probes
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: RT");

		pass.newTextureDependency(rtResultHandle, TextureUsageBit::kUavCompute);
		pass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		if(getRenderer().getGeneratedSky().isEnabled())
		{
			pass.newTextureDependency(getRenderer().getGeneratedSky().getEnvironmentMapRt(), TextureUsageBit::kSrvTraceRays);
		}
		pass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvTraceRays);
		pass.newAccelerationStructureDependency(getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												AccelerationStructureUsageBit::kTraceRaysSrv);

		for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
		{
			pass.newTextureDependency(irradianceVolumes[clipmap], TextureUsageBit::kSrvCompute);
		}

		pass.setWork([this, rtResultHandle, &ctx, sbtBuffer, irradianceVolumes, probeValidityVolumes,
					  distanceMomentsVolumes](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_rtLibraryGrProg.get());

			// More globals
			cmdb.bindSampler(ANKI_MATERIAL_REGISTER_TILINEAR_REPEAT_SAMPLER, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_GPU_SCENE, 0, GpuSceneBuffer::getSingleton().getBufferView());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_MESH_LODS, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_TRANSFORMS, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());

#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType, reg) \
	cmdb.bindSrv( \
		reg, 0, \
		BufferView(&UnifiedGeometryBuffer::getSingleton().getBuffer(), 0, \
				   getAlignedRoundDown(getFormatInfo(Format::k##fmt).m_texelSize, UnifiedGeometryBuffer::getSingleton().getBuffer().getSize())), \
		Format::k##fmt);
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.def.h>

			cmdb.bindConstantBuffer(0, 2, ctx.m_globalRenderingConstantsBuffer);

			U32 srv = 0;
			rgraphCtx.bindSrv(srv++, 2, getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle());

			const LightComponent* dirLight = SceneGraph::getSingleton().getDirectionalLight();
			const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();
			const Bool bSkySolidColor =
				(!sky || sky->getSkyboxType() == SkyboxType::kSolidColor || (!dirLight && sky->getSkyboxType() == SkyboxType::kGenerated));
			if(bSkySolidColor)
			{
				cmdb.bindSrv(srv++, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			}
			else if(sky->getSkyboxType() == SkyboxType::kImage2D)
			{
				cmdb.bindSrv(srv++, 2, TextureView(&sky->getImageResource().getTexture(), TextureSubresourceDesc::all()));
			}
			else
			{
				rgraphCtx.bindSrv(srv++, 2, getRenderer().getGeneratedSky().getEnvironmentMapRt());
			}

			rgraphCtx.bindSrv(srv++, 2, getRenderer().getShadowMapping().getShadowmapRt());

			cmdb.bindSrv(srv++, 2, BufferView(getDummyGpuResources().m_buffer.get(), 0, sizeof(U32)));
			cmdb.bindSrv(srv++, 2, BufferView(getDummyGpuResources().m_buffer.get(), 0, sizeof(U32)));

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				rgraphCtx.bindSrv(srv++, 2, irradianceVolumes[clipmap]);
			}
			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				rgraphCtx.bindSrv(srv++, 2, probeValidityVolumes[clipmap]);
			}
			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				rgraphCtx.bindSrv(srv++, 2, distanceMomentsVolumes[clipmap]);
			}

			for(U32 i = 0; i < 3; ++i)
			{
				cmdb.bindSrv(srv++, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			}

			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			cmdb.bindSampler(1, 2, getRenderer().getSamplers().m_trilinearClampShadow.get());
			cmdb.bindSampler(2, 2, getRenderer().getSamplers().m_trilinearRepeat.get());

			rgraphCtx.bindUav(0, 2, rtResultHandle);
			cmdb.bindUav(1, 2, TextureView(getDummyGpuResources().m_texture2DUav.get(), TextureSubresourceDesc::firstSurface()));

			ANKI_ASSERT(g_indirectDiffuseClipmapRadianceOctMapSize % 2 == 0);
			const U32 raysPerProbePerFrame = square(g_indirectDiffuseClipmapRadianceOctMapSize / 2);

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				const UVec4 consts(clipmap, g_indirectDiffuseClipmapRadianceOctMapSize, 0, 0);
				cmdb.setFastConstants(&consts, sizeof(consts));

				const U32 probeCount = m_clipmapInfo[0].m_probeCountTotal;
				cmdb.traceRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
							   probeCount * raysPerProbePerFrame, 1, 1);
			}
		});
	}

	// Populate caches
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: Populate caches");

		pass.newTextureDependency(rtResultHandle, TextureUsageBit::kSrvCompute);
		for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
		{
			pass.newTextureDependency(radianceVolumes[clipmap], TextureUsageBit::kUavCompute);
			pass.newTextureDependency(probeValidityVolumes[clipmap], TextureUsageBit::kUavCompute);
			pass.newTextureDependency(distanceMomentsVolumes[clipmap], TextureUsageBit::kUavCompute);
		}

		pass.setWork([this, &ctx, rtResultHandle, radianceVolumes, probeValidityVolumes, distanceMomentsVolumes](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_populateCachesGrProg.get());

			rgraphCtx.bindSrv(0, 0, rtResultHandle);

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				rgraphCtx.bindUav(0, 0, radianceVolumes[clipmap]);
				rgraphCtx.bindUav(1, 0, distanceMomentsVolumes[clipmap]);
				rgraphCtx.bindUav(2, 0, probeValidityVolumes[clipmap]);

				const UVec4 consts(clipmap);
				cmdb.setFastConstants(&consts, sizeof(consts));

				const U32 raysPerProbePerFrame = square(g_indirectDiffuseClipmapRadianceOctMapSize / 2);
				const U32 threadCount = 64;
				cmdb.dispatchCompute((raysPerProbePerFrame * m_clipmapInfo[clipmap].m_probeCountTotal + threadCount - 1) / threadCount, 1, 1);
			}
		});
	}

	// Compute irradiance
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: Irradiance");

		for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
		{
			pass.newTextureDependency(radianceVolumes[clipmap], TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(irradianceVolumes[clipmap], TextureUsageBit::kUavCompute);
		}

		pass.setWork([this, &ctx, radianceVolumes, irradianceVolumes](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_computeIrradianceGrProg.get());

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				rgraphCtx.bindSrv(clipmap, 0, radianceVolumes[clipmap]);
				rgraphCtx.bindUav(clipmap, 0, irradianceVolumes[clipmap]);
			}

			cmdb.dispatchCompute(m_clipmapInfo[0].m_probeCounts[0] * kIndirectDiffuseClipmapCount, m_clipmapInfo[0].m_probeCounts[1],
								 m_clipmapInfo[0].m_probeCounts[2]);
		});
	}

	// Apply GI
	if(1)
	{
		patchShaderBindingTablePass("IndirectDiffuseClipmaps: Patch SBT", m_rtLibraryGrProg.get(), m_rayGenShaderGroupIndices[0],
									m_missShaderGroupIdx, m_sbtRecordSize, rgraph, sbtHandle, sbtBuffer);

		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: RTApply");

		pass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		if(getRenderer().getGeneratedSky().isEnabled())
		{
			pass.newTextureDependency(getRenderer().getGeneratedSky().getEnvironmentMapRt(), TextureUsageBit::kSrvTraceRays);
		}
		pass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvTraceRays);
		pass.newAccelerationStructureDependency(getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												AccelerationStructureUsageBit::kTraceRaysSrv);
		pass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSrvTraceRays);
		pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvTraceRays);

		for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
		{
			pass.newTextureDependency(irradianceVolumes[clipmap], TextureUsageBit::kSrvTraceRays);
			pass.newTextureDependency(probeValidityVolumes[clipmap], TextureUsageBit::kSrvTraceRays);
			pass.newTextureDependency(distanceMomentsVolumes[clipmap], TextureUsageBit::kSrvTraceRays);
		}

		pass.newTextureDependency(m_runCtx.m_appliedGiRt, TextureUsageBit::kUavTraceRays);

		pass.setWork([this, &ctx, sbtBuffer, irradianceVolumes, probeValidityVolumes, distanceMomentsVolumes](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_rtLibraryGrProg.get());

			// More globals
			cmdb.bindSampler(ANKI_MATERIAL_REGISTER_TILINEAR_REPEAT_SAMPLER, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_GPU_SCENE, 0, GpuSceneBuffer::getSingleton().getBufferView());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_MESH_LODS, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_TRANSFORMS, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());

#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType, reg) \
	cmdb.bindSrv( \
		reg, 0, \
		BufferView(&UnifiedGeometryBuffer::getSingleton().getBuffer(), 0, \
				   getAlignedRoundDown(getFormatInfo(Format::k##fmt).m_texelSize, UnifiedGeometryBuffer::getSingleton().getBuffer().getSize())), \
		Format::k##fmt);
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.def.h>

			cmdb.bindConstantBuffer(0, 2, ctx.m_globalRenderingConstantsBuffer);

			U32 srv = 0;
			rgraphCtx.bindSrv(srv++, 2, getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle());

			const LightComponent* dirLight = SceneGraph::getSingleton().getDirectionalLight();
			const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();
			const Bool bSkySolidColor =
				(!sky || sky->getSkyboxType() == SkyboxType::kSolidColor || (!dirLight && sky->getSkyboxType() == SkyboxType::kGenerated));
			if(bSkySolidColor)
			{
				cmdb.bindSrv(srv++, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			}
			else if(sky->getSkyboxType() == SkyboxType::kImage2D)
			{
				cmdb.bindSrv(srv++, 2, TextureView(&sky->getImageResource().getTexture(), TextureSubresourceDesc::all()));
			}
			else
			{
				rgraphCtx.bindSrv(srv++, 2, getRenderer().getGeneratedSky().getEnvironmentMapRt());
			}

			rgraphCtx.bindSrv(srv++, 2, getRenderer().getShadowMapping().getShadowmapRt());

			cmdb.bindSrv(srv++, 2, BufferView(getDummyGpuResources().m_buffer.get(), 0, sizeof(U32)));
			cmdb.bindSrv(srv++, 2, BufferView(getDummyGpuResources().m_buffer.get(), 0, sizeof(U32)));

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				rgraphCtx.bindSrv(srv++, 2, irradianceVolumes[clipmap]);
			}
			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				rgraphCtx.bindSrv(srv++, 2, probeValidityVolumes[clipmap]);
			}
			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				rgraphCtx.bindSrv(srv++, 2, distanceMomentsVolumes[clipmap]);
			}

			rgraphCtx.bindSrv(srv++, 2, getRenderer().getGBuffer().getDepthRt());
			cmdb.bindSrv(srv++, 2, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			rgraphCtx.bindSrv(srv++, 2, getRenderer().getGBuffer().getColorRt(2));

			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			cmdb.bindSampler(1, 2, getRenderer().getSamplers().m_trilinearClampShadow.get());
			cmdb.bindSampler(2, 2, getRenderer().getSamplers().m_trilinearRepeat.get());

			rgraphCtx.bindUav(0, 2, m_runCtx.m_appliedGiRt);
			cmdb.bindUav(1, 2, TextureView(getDummyGpuResources().m_texture2DUav.get(), TextureSubresourceDesc::firstSurface()));

			const Vec3 probeSizes = m_clipmapInfo[0].m_size / Vec3(m_clipmapInfo[0].m_probeCounts);
			const F32 rayTMax = max(probeSizes.x(), max(probeSizes.y(), probeSizes.z())) * 10.0f;
			const Vec4 consts(rayTMax);
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.traceRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
						   getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), 1);
		});
	}
	else
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps composite");

		pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
		{
			pass.newTextureDependency(irradianceVolumes[i], TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(probeValidityVolumes[i], TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(distanceMomentsVolumes[i], TextureUsageBit::kSrvCompute);
		}
		pass.newTextureDependency(m_runCtx.m_appliedGiRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, &ctx, irradianceVolumes, probeValidityVolumes, distanceMomentsVolumes](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_applyGiGrProg.get());

			rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(1, 0, getRenderer().getGBuffer().getColorRt(2));
			cmdb.bindSrv(2, 0, TextureView(&m_blueNoiseImg->getTexture(), TextureSubresourceDesc::firstSurface()));

			U32 srvReg = 3;
			for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
			{
				rgraphCtx.bindSrv(srvReg++, 0, irradianceVolumes[i]);
			}

			for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
			{
				rgraphCtx.bindSrv(srvReg++, 0, probeValidityVolumes[i]);
			}

			for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
			{
				rgraphCtx.bindSrv(srvReg++, 0, distanceMomentsVolumes[i]);
			}

			rgraphCtx.bindUav(0, 0, m_runCtx.m_appliedGiRt);

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}
}

void IndirectDiffuseClipmaps::drawDebugProbes(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx) const
{
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	const U32 clipmap = 0;

	cmdb.bindShaderProgram(m_visProbesGrProg.get());

	const UVec4 consts(clipmap);
	cmdb.setFastConstants(&consts, sizeof(consts));

	cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

	Texture* visVolume = m_irradianceVolumes[clipmap].get();
	cmdb.bindSrv(0, 0, TextureView(visVolume, TextureSubresourceDesc::all()));
	rgraphCtx.bindSrv(1, 0, m_runCtx.m_probeValidityRts[clipmap]);
	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

	cmdb.draw(PrimitiveTopology::kTriangles, 36, m_clipmapInfo[clipmap].m_probeCountTotal);
}

} // end namespace anki
