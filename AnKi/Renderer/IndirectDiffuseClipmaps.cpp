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
#include <AnKi/Renderer/HistoryLength.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>

namespace anki {

static void computeClipmapBounds(Vec3 cameraPos, Vec3 lookDir, U32 clipmapIdx, IndirectDiffuseClipmapConstants& consts)
{
	const Vec3 offset = lookDir * kIndirectDiffuseClipmapForwardBias * F32(clipmapIdx + 1);
	cameraPos += offset;

	const Vec3 halfSize = consts.m_sizes[clipmapIdx].xyz() * 0.5;
	const Vec3 probeSize = consts.m_sizes[clipmapIdx].xyz() / Vec3(consts.m_probeCounts);
	const Vec3 roundedPos = (cameraPos / probeSize).round() * probeSize;
	consts.m_aabbMins[clipmapIdx] = (roundedPos - halfSize).xyz0();
	[[maybe_unused]] const Vec3 aabbMax = roundedPos + halfSize;
	ANKI_ASSERT(aabbMax - consts.m_aabbMins[clipmapIdx].xyz() == consts.m_sizes[clipmapIdx].xyz());
}

Error IndirectDiffuseClipmaps::init()
{
	ANKI_CHECK(RtMaterialFetchRendererObject::init());

	const Bool firstBounceUsesRt = g_indirectDiffuseClipmapFirstBounceRayDistance > 0.0f;

	m_lowRezRtDesc = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / (!g_indirectDiffuseClipmapApplyHighQuality + 1),
		getRenderer().getHdrFormat(), "IndirectDiffuseClipmap: Apply rez");
	m_lowRezRtDesc.bake();

	m_fullRtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
																 getRenderer().getHdrFormat(), "IndirectDiffuseClipmap: Full");
	m_fullRtDesc.bake();

	if(firstBounceUsesRt)
	{
		for(U32 i = 0; i < 2; ++i)
		{
			const TextureInitInfo init = getRenderer().create2DRenderTargetInitInfo(
				getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), Format::kR16G16B16A16_Sfloat,
				TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Final #%u", i));

			m_irradianceRts[i] = getRenderer().createAndClearRenderTarget(init, TextureUsageBit::kSrvCompute);
		}
	}

	m_consts.m_probeCounts = UVec3(g_indirectDiffuseClipmapProbesXZCVar, g_indirectDiffuseClipmapProbesYCVar, g_indirectDiffuseClipmapProbesXZCVar);
	m_consts.m_totalProbeCount = m_consts.m_probeCounts.x() * m_consts.m_probeCounts.y() * m_consts.m_probeCounts.z();

	m_consts.m_sizes[0] = Vec3(g_indirectDiffuseClipmap0XZSizeCVar, g_indirectDiffuseClipmap0YSizeCVar, g_indirectDiffuseClipmap0XZSizeCVar).xyz0();
	m_consts.m_sizes[1] = Vec3(g_indirectDiffuseClipmap1XZSizeCVar, g_indirectDiffuseClipmap1YSizeCVar, g_indirectDiffuseClipmap1XZSizeCVar).xyz0();
	m_consts.m_sizes[2] = Vec3(g_indirectDiffuseClipmap2XZSizeCVar, g_indirectDiffuseClipmap2YSizeCVar, g_indirectDiffuseClipmap2XZSizeCVar).xyz0();

	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		TextureInitInfo init = getRenderer().create2DRenderTargetInitInfo(m_consts.m_probeCounts.x(), m_consts.m_probeCounts.z(), Format::kR8_Unorm,
																		  TextureUsageBit::kUavCompute | TextureUsageBit::kAllSrv,
																		  generateTempPassName("IndirectDiffuseClipmap: Probe validity #%u", i));

		init.m_depth = m_consts.m_probeCounts.y();
		init.m_type = TextureType::k3D;

		m_probeValidityVolumes[i] = getRenderer().createAndClearRenderTarget(init, TextureUsageBit::kSrvCompute);
	}

	// Create the RT result texture
	const U32 raysPerProbePerFrame = square<U32>(g_indirectDiffuseClipmapRadianceOctMapSize);
	m_rtResultRtDesc = getRenderer().create2DRenderTargetDescription(m_consts.m_totalProbeCount, raysPerProbePerFrame * kIndirectDiffuseClipmapCount,
																	 Format::kR16G16B16A16_Sfloat, "IndirectDiffuseClipmap: RT result");
	m_rtResultRtDesc.bake();

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_consts.m_probeCounts.x() * (g_indirectDiffuseClipmapRadianceOctMapSize + 2),
			m_consts.m_probeCounts.z() * (g_indirectDiffuseClipmapRadianceOctMapSize + 2), Format::kB10G11R11_Ufloat_Pack32,
			TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Radiance #%u", clipmap));
		volumeInit.m_depth = m_consts.m_probeCounts.y();
		volumeInit.m_type = TextureType::k3D;

		m_radianceVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_consts.m_probeCounts.x() * (g_indirectDiffuseClipmapIrradianceOctMapSize + 2),
			m_consts.m_probeCounts.z() * (g_indirectDiffuseClipmapIrradianceOctMapSize + 2), Format::kB10G11R11_Ufloat_Pack32,
			TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Irradiance #%u", clipmap));
		volumeInit.m_depth = m_consts.m_probeCounts.y();
		volumeInit.m_type = TextureType::k3D;

		m_irradianceVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_consts.m_probeCounts.x() * (g_indirectDiffuseClipmapRadianceOctMapSize + 2),
			m_consts.m_probeCounts.z() * (g_indirectDiffuseClipmapRadianceOctMapSize + 2), Format::kR16G16_Sfloat,
			TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Dist moments #%u", clipmap));
		volumeInit.m_depth = m_consts.m_probeCounts.y();
		volumeInit.m_type = TextureType::k3D;

		m_distanceMomentsVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_consts.m_probeCounts.x(), m_consts.m_probeCounts.z(), Format::kB10G11R11_Ufloat_Pack32, TextureUsageBit::kAllShaderResource,
			generateTempPassName("IndirectDiffuseClipmap: Avg light #%u", clipmap));
		volumeInit.m_depth = m_consts.m_probeCounts.y();
		volumeInit.m_type = TextureType::k3D;

		m_avgIrradianceVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	const Array<SubMutation, 5> mutation = {{{"GPU_WAVE_SIZE", MutatorValue(GrManager::getSingleton().getDeviceCapabilities().m_maxWaveSize)},
											 {"RADIANCE_OCTAHEDRON_MAP_SIZE", MutatorValue(g_indirectDiffuseClipmapRadianceOctMapSize)},
											 {"IRRADIANCE_OCTAHEDRON_MAP_SIZE", MutatorValue(g_indirectDiffuseClipmapIrradianceOctMapSize)},
											 {"RT_MATERIAL_FETCH_CLIPMAP", 0},
											 {"SPATIAL_RECONSTRUCT_TYPE", !g_indirectDiffuseClipmapApplyHighQuality}}};

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_applyGiGrProg, "Apply"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_visProbesGrProg, "VisualizeProbes"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_populateCachesGrProg, "PopulateCaches"));
	ANKI_CHECK(
		loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_computeIrradianceGrProg, "ComputeIrradiance"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_temporalDenoiseGrProg, "TemporalDenoise"));
	ANKI_CHECK(
		loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_spatialReconstructGrProg, "SpatialReconstruct"));
	ANKI_CHECK(
		loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", mutation, m_prog, m_bilateralDenoiseGrProg, "BilateralDenoise"));

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

	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		m_consts.m_textures[i].m_radianceTexture = m_radianceVolumes[i]->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());
		m_consts.m_textures[i].m_irradianceTexture = m_irradianceVolumes[i]->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());
		m_consts.m_textures[i].m_distanceMomentsTexture = m_distanceMomentsVolumes[i]->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());
		m_consts.m_textures[i].m_probeValidityTexture = m_probeValidityVolumes[i]->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());
		m_consts.m_textures[i].m_averageIrradianceTexture = m_avgIrradianceVolumes[i]->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());

		m_consts.m_textures[i].m_distanceMomentsOctMapSize = (m_distanceMomentsVolumes[i]->getWidth() / m_consts.m_probeCounts.x()) - 2;
		m_consts.m_textures[i].m_irradianceOctMapSize = (m_irradianceVolumes[i]->getWidth() / m_consts.m_probeCounts.x()) - 2;
		m_consts.m_textures[i].m_radianceOctMapSize = (m_radianceVolumes[i]->getWidth() / m_consts.m_probeCounts.x()) - 2;
	}

	return Error::kNone;
}

void IndirectDiffuseClipmaps::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuse);

	const Bool firstBounceUsesRt = g_indirectDiffuseClipmapFirstBounceRayDistance > 0.0f;

	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		m_consts.m_previousFrameAabbMins[i] = m_consts.m_aabbMins[i];

		computeClipmapBounds(ctx.m_matrices.m_cameraTransform.getTranslationPart(),
							 -ctx.m_matrices.m_cameraTransform.getRotationPart().getZAxis().normalize(), i, m_consts);
	}

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	const RenderTargetHandle rtResultHandle = rgraph.newRenderTarget(m_rtResultRtDesc);
	const RenderTargetHandle lowRezRt = rgraph.newRenderTarget(m_lowRezRtDesc);
	const RenderTargetHandle fullRtTmp = rgraph.newRenderTarget(m_fullRtDesc);

	Array<RenderTargetHandle, 2> fullRts;
	if(firstBounceUsesRt)
	{
		for(U32 i = 0; i < 2; ++i)
		{
			if(m_texturesImportedOnce) [[likely]]
			{
				fullRts[i] = rgraph.importRenderTarget(m_irradianceRts[i].get());
			}
			else
			{
				fullRts[i] = rgraph.importRenderTarget(m_irradianceRts[i].get(), TextureUsageBit::kSrvCompute);
			}
		}
	}

	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount>& radianceVolumes = m_runCtx.m_handles.m_radianceVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount>& irradianceVolumes = m_runCtx.m_handles.m_irradianceVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount>& distanceMomentsVolumes = m_runCtx.m_handles.m_distanceMomentsVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount>& probeValidityVolumes = m_runCtx.m_handles.m_probeValidityVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount>& avgIrradianceVolumes = m_runCtx.m_handles.m_avgIrradianceVolumes;
	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		if(m_texturesImportedOnce) [[likely]]
		{
			radianceVolumes[i] = rgraph.importRenderTarget(m_radianceVolumes[i].get());
			irradianceVolumes[i] = rgraph.importRenderTarget(m_irradianceVolumes[i].get());
			distanceMomentsVolumes[i] = rgraph.importRenderTarget(m_distanceMomentsVolumes[i].get());
			probeValidityVolumes[i] = rgraph.importRenderTarget(m_probeValidityVolumes[i].get());
			avgIrradianceVolumes[i] = rgraph.importRenderTarget(m_avgIrradianceVolumes[i].get());
		}
		else
		{
			radianceVolumes[i] = rgraph.importRenderTarget(m_radianceVolumes[i].get(), TextureUsageBit::kSrvCompute);
			irradianceVolumes[i] = rgraph.importRenderTarget(m_irradianceVolumes[i].get(), TextureUsageBit::kSrvCompute);
			distanceMomentsVolumes[i] = rgraph.importRenderTarget(m_distanceMomentsVolumes[i].get(), TextureUsageBit::kSrvCompute);
			probeValidityVolumes[i] = rgraph.importRenderTarget(m_probeValidityVolumes[i].get(), TextureUsageBit::kSrvCompute);
			avgIrradianceVolumes[i] = rgraph.importRenderTarget(m_avgIrradianceVolumes[i].get(), TextureUsageBit::kSrvCompute);
		}
	}

	m_texturesImportedOnce = true;

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
		setRgenSpace2Dependencies(pass);

		for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
		{
			pass.newTextureDependency(irradianceVolumes[clipmap], TextureUsageBit::kSrvCompute);
		}

		pass.setWork([this, rtResultHandle, &ctx, sbtBuffer](RenderPassWorkContext& rgraphCtx) {
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

			bindRgenSpace2Resources(ctx, rgraphCtx);

			rgraphCtx.bindUav(0, 2, rtResultHandle);

			const U32 raysPerProbePerFrame = square<U32>(g_indirectDiffuseClipmapRadianceOctMapSize);

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				const UVec4 consts(clipmap, g_indirectDiffuseClipmapRadianceOctMapSize, 0, 0);
				cmdb.setFastConstants(&consts, sizeof(consts));

				cmdb.dispatchRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
								  m_consts.m_totalProbeCount * raysPerProbePerFrame, 1, 1);
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

				const U32 raysPerProbePerFrame = square<U32>(g_indirectDiffuseClipmapRadianceOctMapSize);
				const U32 threadCount = 64;
				cmdb.dispatchCompute((raysPerProbePerFrame * m_consts.m_totalProbeCount + threadCount - 1) / threadCount, 1, 1);
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
			pass.newTextureDependency(avgIrradianceVolumes[clipmap], TextureUsageBit::kUavCompute);
		}

		pass.setWork([this, &ctx, radianceVolumes, irradianceVolumes, avgIrradianceVolumes](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_computeIrradianceGrProg.get());

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			U32 uav = 0;
			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				rgraphCtx.bindSrv(clipmap, 0, radianceVolumes[clipmap]);
				rgraphCtx.bindUav(uav++, 0, irradianceVolumes[clipmap]);
			}

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				rgraphCtx.bindUav(uav++, 0, avgIrradianceVolumes[clipmap]);
			}

			cmdb.dispatchCompute(m_consts.m_probeCounts[0] * kIndirectDiffuseClipmapCount, m_consts.m_probeCounts[1], m_consts.m_probeCounts[2]);
		});
	}

	// Apply GI
	if(firstBounceUsesRt)
	{
		patchShaderBindingTablePass("IndirectDiffuseClipmaps: Patch SBT", m_rtLibraryGrProg.get(), m_rayGenShaderGroupIndices[0],
									m_missShaderGroupIdx, m_sbtRecordSize, rgraph, sbtHandle, sbtBuffer);

		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: RTApply");

		pass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);

		for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
		{
			pass.newTextureDependency(irradianceVolumes[clipmap], TextureUsageBit::kSrvDispatchRays);
			pass.newTextureDependency(probeValidityVolumes[clipmap], TextureUsageBit::kSrvDispatchRays);
			pass.newTextureDependency(distanceMomentsVolumes[clipmap], TextureUsageBit::kSrvDispatchRays);
		}

		pass.newTextureDependency(lowRezRt, TextureUsageBit::kUavDispatchRays);
		setRgenSpace2Dependencies(pass);

		pass.setWork([this, &ctx, sbtBuffer, lowRezRt](RenderPassWorkContext& rgraphCtx) {
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

			bindRgenSpace2Resources(ctx, rgraphCtx);
			rgraphCtx.bindUav(0, 2, lowRezRt);

			const Vec4 consts(g_indirectDiffuseClipmapFirstBounceRayDistance);
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.dispatchRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
							  getRenderer().getInternalResolution().x() / 2,
							  getRenderer().getInternalResolution().y() / (!g_indirectDiffuseClipmapApplyHighQuality + 1), 1);
		});
	}
	else
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: Apply irradiance");

		pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
		{
			pass.newTextureDependency(irradianceVolumes[i], TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(probeValidityVolumes[i], TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(distanceMomentsVolumes[i], TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(avgIrradianceVolumes[i], TextureUsageBit::kSrvCompute);
		}
		pass.newTextureDependency(lowRezRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, &ctx, lowRezRt](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_applyGiGrProg.get());

			rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(1, 0, getGBuffer().getColorRt(2));
			cmdb.bindSrv(2, 0, TextureView(&m_blueNoiseImg->getTexture(), TextureSubresourceDesc::firstSurface()));

			rgraphCtx.bindUav(0, 0, lowRezRt);

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x() / 2,
							  getRenderer().getInternalResolution().y() / (!g_indirectDiffuseClipmapApplyHighQuality + 1));
		});
	}

	// Spatial reconstruct
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: Spatial reconstruct");

		pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(lowRezRt, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(fullRtTmp, TextureUsageBit::kUavCompute);

		pass.setWork([this, lowRezRt, fullRtTmp](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_spatialReconstructGrProg.get());

			rgraphCtx.bindSrv(0, 0, lowRezRt);
			rgraphCtx.bindSrv(1, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindUav(0, 0, fullRtTmp);

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x() / 2,
							  getRenderer().getInternalResolution().y() / (!g_indirectDiffuseClipmapApplyHighQuality + 1));
		});
	}

	if(!firstBounceUsesRt)
	{
		m_runCtx.m_handles.m_appliedIrradiance = fullRtTmp;
		return;
	}

	const RenderTargetHandle historyRt = fullRts[0];
	const RenderTargetHandle outRt = fullRts[1];

	// Temporal denoise
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: Temporal denoise");

		pass.newTextureDependency(fullRtTmp, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(historyRt, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getHistoryLength().getRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getMotionVectors().getMotionVectorsRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(outRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, &ctx, fullRtTmp, historyRt, outRt](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_temporalDenoiseGrProg.get());

			rgraphCtx.bindSrv(0, 0, getHistoryLength().getRt());
			rgraphCtx.bindSrv(1, 0, getMotionVectors().getMotionVectorsRt());
			rgraphCtx.bindSrv(2, 0, historyRt);
			rgraphCtx.bindSrv(3, 0, fullRtTmp);

			rgraphCtx.bindUav(0, 0, outRt);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}

	// Bilateral denoise
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: Bilateral denoise");

		pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(outRt, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(historyRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, outRt, historyRt](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_bilateralDenoiseGrProg.get());

			rgraphCtx.bindSrv(0, 0, outRt);
			rgraphCtx.bindSrv(1, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindUav(0, 0, historyRt);

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}

	m_runCtx.m_handles.m_appliedIrradiance = historyRt;
}

void IndirectDiffuseClipmaps::drawDebugProbes(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx) const
{
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	const U32 clipmap = 0;

	cmdb.bindShaderProgram(m_visProbesGrProg.get());

	const UVec4 consts(clipmap);
	cmdb.setFastConstants(&consts, sizeof(consts));

	cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

	const RenderTargetHandle visVolume = m_runCtx.m_handles.m_avgIrradianceVolumes[clipmap];
	rgraphCtx.bindSrv(0, 0, visVolume);
	rgraphCtx.bindSrv(1, 0, m_runCtx.m_handles.m_probeValidityVolumes[clipmap]);
	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

	cmdb.draw(PrimitiveTopology::kTriangles, 36, m_consts.m_totalProbeCount);
}

} // end namespace anki
