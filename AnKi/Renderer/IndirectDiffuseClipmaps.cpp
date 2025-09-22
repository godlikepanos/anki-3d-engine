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

ANKI_SVAR(IdcRays, StatCategory::kRenderer, "IDC ray count", StatFlag::kZeroEveryFrame)

class ProbeRange
{
public:
	IVec3 m_begin;
	IVec3 m_end;
};

/// Given the clipmap's position of this and the previous frame it splits the clipmap into regions that contain new probes (thus they need a full
/// update) or regions of probes that need a less frequent update.
static void findClipmapInUpdateRanges(Vec3 newClipmapMin, Vec3 oldClipmapMin, Vec3 probeSize, UVec3 probeCountsu,
									  Array<ProbeRange, 3>& fullUpdateProbeRanges, U32& fullUpdateProbeRangeCount,
									  ProbeRange& partialUpdateProbeRange)
{
	fullUpdateProbeRangeCount = 0;

	const IVec3 probeCounts(probeCountsu);

	const IVec3 delta = IVec3((newClipmapMin - oldClipmapMin) / probeSize);
	const IVec3 absDelta = delta.abs();

	if(absDelta.x() >= probeCounts.x() || absDelta.y() >= probeCounts.y() || absDelta.z() >= probeCounts.z())
	{
		// No overlap between the old and new clipmap positions, full update

		fullUpdateProbeRanges[fullUpdateProbeRangeCount++] = {IVec3(0), probeCounts};
		partialUpdateProbeRange = {IVec3(0), IVec3(0)};
	}
	else
	{
		IVec3 partialUpdateProbeRangeBegin(0);
		IVec3 partialUpdateProbeRangeEnd = probeCounts;

		IVec3 fullUpdateProbeRangeBegin(0);
		IVec3 fullUpdateProbeRangeEnd(0);
		if(delta.x() > 0)
		{
			// New AABB on the right of old
			fullUpdateProbeRangeBegin =
				IVec3(partialUpdateProbeRangeEnd.x() - delta.x(), partialUpdateProbeRangeBegin.y(), partialUpdateProbeRangeBegin.z());
			fullUpdateProbeRangeEnd = partialUpdateProbeRangeEnd;

			partialUpdateProbeRangeEnd.x() -= delta.x();
		}
		else if(delta.x() < 0)
		{
			// New AABB on the left of old
			fullUpdateProbeRangeBegin = partialUpdateProbeRangeBegin;
			fullUpdateProbeRangeEnd = IVec3(-delta.x(), partialUpdateProbeRangeEnd.y(), partialUpdateProbeRangeEnd.z());

			partialUpdateProbeRangeBegin.x() += -delta.x();
		}

		if(delta.x() != 0)
		{
			fullUpdateProbeRanges[fullUpdateProbeRangeCount++] = {fullUpdateProbeRangeBegin, fullUpdateProbeRangeEnd};
		}

		fullUpdateProbeRangeBegin = fullUpdateProbeRangeEnd = IVec3(0);
		if(delta.y() > 0)
		{
			// New AABB on the top of old
			fullUpdateProbeRangeBegin =
				IVec3(partialUpdateProbeRangeBegin.x(), partialUpdateProbeRangeEnd.y() - delta.y(), partialUpdateProbeRangeBegin.z());
			fullUpdateProbeRangeEnd = partialUpdateProbeRangeEnd;

			partialUpdateProbeRangeEnd.y() -= delta.y();
		}
		else if(delta.y() < 0)
		{
			// New AABB at the bottom of old
			fullUpdateProbeRangeBegin = partialUpdateProbeRangeBegin;
			fullUpdateProbeRangeEnd = IVec3(partialUpdateProbeRangeEnd.x(), -delta.y(), partialUpdateProbeRangeEnd.z());

			partialUpdateProbeRangeBegin.y() += -delta.y();
		}

		if(delta.y() != 0)
		{
			fullUpdateProbeRanges[fullUpdateProbeRangeCount++] = {fullUpdateProbeRangeBegin, fullUpdateProbeRangeEnd};
		}

		fullUpdateProbeRangeBegin = fullUpdateProbeRangeEnd = IVec3(0);
		if(delta.z() > 0)
		{
			// New AABB on the front of old
			fullUpdateProbeRangeBegin =
				IVec3(partialUpdateProbeRangeBegin.x(), partialUpdateProbeRangeBegin.y(), partialUpdateProbeRangeEnd.z() - delta.z());
			fullUpdateProbeRangeEnd = partialUpdateProbeRangeEnd;

			partialUpdateProbeRangeEnd.z() -= delta.z();
		}
		else if(delta.z() < 0)
		{
			// New AABB on the back of old
			fullUpdateProbeRangeBegin = partialUpdateProbeRangeBegin;
			fullUpdateProbeRangeEnd = IVec3(partialUpdateProbeRangeEnd.x(), partialUpdateProbeRangeEnd.y(), -delta.z());

			partialUpdateProbeRangeBegin.z() += -delta.z();
		}

		if(delta.z() != 0)
		{
			fullUpdateProbeRanges[fullUpdateProbeRangeCount++] = {fullUpdateProbeRangeBegin, fullUpdateProbeRangeEnd};
		}

		partialUpdateProbeRange = {partialUpdateProbeRangeBegin, partialUpdateProbeRangeEnd};

		// Validation
		[[maybe_unused]] I32 totalProbeCount = 0;
		for(U32 i = 0; i < fullUpdateProbeRangeCount; ++i)
		{
			const IVec3 end = fullUpdateProbeRanges[i].m_end;
			const IVec3 begin = fullUpdateProbeRanges[i].m_begin;
			const IVec3 diff = end - begin;
			ANKI_ASSERT(diff.x() * diff.y() * diff.z() > 0);
			totalProbeCount += diff.x() * diff.y() * diff.z();
		}

		{
			const IVec3 end = partialUpdateProbeRange.m_end;
			const IVec3 begin = partialUpdateProbeRange.m_begin;
			const IVec3 diff = end - begin;
			ANKI_ASSERT(diff.x() * diff.y() * diff.z() > 0);
			totalProbeCount += diff.x() * diff.y() * diff.z();
		}
		ANKI_ASSERT(totalProbeCount == probeCounts.x() * probeCounts.y() * probeCounts.z());
	}
}

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

	const Bool firstBounceUsesRt = g_cvarRenderIdcFirstBounceRayDistance > 0.0f;

	m_lowRezRtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x() / 2,
																   getRenderer().getInternalResolution().y() / (!g_cvarRenderIdcApplyHighQuality + 1),
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

	m_consts.m_probeCounts = UVec3(g_cvarRenderIdcProbesXZ, g_cvarRenderIdcProbesY, g_cvarRenderIdcProbesXZ);
	m_consts.m_totalProbeCount = m_consts.m_probeCounts.x() * m_consts.m_probeCounts.y() * m_consts.m_probeCounts.z();

	m_consts.m_sizes[0] = Vec3(g_cvarRenderIdcClipmap0XZSize, g_cvarRenderIdcClipmap0YSize, g_cvarRenderIdcClipmap0XZSize).xyz0();
	m_consts.m_sizes[1] = Vec3(g_cvarRenderIdcClipmap1XZSize, g_cvarRenderIdcClipmap1YSize, g_cvarRenderIdcClipmap1XZSize).xyz0();
	m_consts.m_sizes[2] = Vec3(g_cvarRenderIdcClipmap2XZSize, g_cvarRenderIdcClipmap2YSize, g_cvarRenderIdcClipmap2XZSize).xyz0();

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
	const U32 texelsPerProbe = square<U32>(g_cvarRenderIdcRadianceOctMapSize);
	m_rtResultRtDesc =
		getRenderer().create2DRenderTargetDescription(m_consts.m_totalProbeCount, texelsPerProbe * g_cvarRenderIdcRayCountPerTexelOfNewProbe,
													  Format::kR16G16B16A16_Sfloat, "IndirectDiffuseClipmap: RT result");
	m_rtResultRtDesc.bake();

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_consts.m_probeCounts.x() * (g_cvarRenderIdcRadianceOctMapSize + 2),
			m_consts.m_probeCounts.z() * (g_cvarRenderIdcRadianceOctMapSize + 2), Format::kB10G11R11_Ufloat_Pack32,
			TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Radiance #%u", clipmap));
		volumeInit.m_depth = m_consts.m_probeCounts.y();
		volumeInit.m_type = TextureType::k3D;

		m_radianceVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_consts.m_probeCounts.x() * (g_cvarRenderIdcIrradianceOctMapSize + 2),
			m_consts.m_probeCounts.z() * (g_cvarRenderIdcIrradianceOctMapSize + 2), Format::kB10G11R11_Ufloat_Pack32,
			TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Irradiance #%u", clipmap));
		volumeInit.m_depth = m_consts.m_probeCounts.y();
		volumeInit.m_type = TextureType::k3D;

		m_irradianceVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_consts.m_probeCounts.x() * (g_cvarRenderIdcRadianceOctMapSize + 2),
			m_consts.m_probeCounts.z() * (g_cvarRenderIdcRadianceOctMapSize + 2), Format::kR16G16_Sfloat, TextureUsageBit::kAllShaderResource,
			generateTempPassName("IndirectDiffuseClipmap: Dist moments #%u", clipmap));
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
											 {"RADIANCE_OCTAHEDRON_MAP_SIZE", MutatorValue(g_cvarRenderIdcRadianceOctMapSize)},
											 {"IRRADIANCE_OCTAHEDRON_MAP_SIZE", MutatorValue(g_cvarRenderIdcIrradianceOctMapSize)},
											 {"RT_MATERIAL_FETCH_CLIPMAP", 0},
											 {"SPATIAL_RECONSTRUCT_TYPE", !g_cvarRenderIdcApplyHighQuality}}};

	constexpr CString kProgFname = "ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin";
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_prog, m_applyGiGrProg, "Apply"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_prog, m_visProbesGrProg, "VisualizeProbes"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_prog, m_populateCachesGrProg, "PopulateCaches"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_prog, m_computeIrradianceGrProg, "ComputeIrradiance"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_prog, m_temporalDenoiseGrProg, "TemporalDenoise"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_prog, m_spatialReconstructGrProg, "SpatialReconstruct"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_prog, m_bilateralDenoiseGrProg, "BilateralDenoise"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_prog, m_rtMaterialFetchInlineRtGrProg, "RtMaterialFetchInlineRt"));

	for(MutatorValue rtMaterialFetchClipmap = 0; rtMaterialFetchClipmap < 2; ++rtMaterialFetchClipmap)
	{
		ShaderProgramResourcePtr tmpProg;
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(kProgFname, tmpProg));
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

		m_consts.m_previousFrameAabbMins[i] = 100000.0f * m_consts.m_sizes[i] / Vec4(Vec3(m_consts.m_probeCounts), 1.0f);
		m_consts.m_aabbMins[i] = m_consts.m_previousFrameAabbMins[i];
	}

	return Error::kNone;
}

void IndirectDiffuseClipmaps::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuse);

	const Bool firstBounceUsesRt = g_cvarRenderIdcFirstBounceRayDistance > 0.0f;

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
	if(!g_cvarRenderIdcInlineRt)
	{
		buildShaderBindingTablePass("IndirectDiffuseClipmaps: Build SBT", m_rtLibraryGrProg.get(), m_rayGenShaderGroupIndices[1],
									m_missShaderGroupIdx, m_sbtRecordSize, rgraph, sbtHandle, sbtBuffer);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		// Compute probe ranges and ray budgets and stuff
		Array<ProbeRange, 3> fullUpdateRanges;
		U32 fullUpdateRangeCount = 0;
		ProbeRange partialUpdateRange;
		const Vec3 probeSize = m_consts.m_sizes[clipmap].xyz() / Vec3(m_consts.m_probeCounts);
		findClipmapInUpdateRanges(m_consts.m_aabbMins[clipmap].xyz(), m_consts.m_previousFrameAabbMins[clipmap].xyz(), probeSize,
								  UVec3(m_consts.m_probeCounts), fullUpdateRanges, fullUpdateRangeCount, partialUpdateRange);

		U32 fullUpdateRayCount = 0;
		for(U32 i = 0; i < fullUpdateRangeCount; ++i)
		{
			const UVec3 counts = UVec3(fullUpdateRanges[i].m_end - fullUpdateRanges[i].m_begin);
			const U32 count = counts.x() * counts.y() * counts.z();
			fullUpdateRayCount += square<U32>(g_cvarRenderIdcRadianceOctMapSize) * g_cvarRenderIdcRayCountPerTexelOfNewProbe * count;
		}

		const U32 remainingRayCount = (g_cvarRenderIdcProbeRayBudget / kIndirectDiffuseClipmapCount > fullUpdateRayCount)
										  ? g_cvarRenderIdcProbeRayBudget / kIndirectDiffuseClipmapCount - fullUpdateRayCount
										  : 0;

		const UVec3 partialUpdateProbeCounts = UVec3(partialUpdateRange.m_end - partialUpdateRange.m_begin);
		U32 partialUpdateProbeCount = remainingRayCount / square<U32>(g_cvarRenderIdcRadianceOctMapSize);
		partialUpdateProbeCount =
			min(partialUpdateProbeCount, partialUpdateProbeCounts.x() * partialUpdateProbeCounts.y() * partialUpdateProbeCounts.z());

		g_svarIdcRays.increment(fullUpdateRayCount + partialUpdateProbeCount * square<U32>(g_cvarRenderIdcRadianceOctMapSize));

		struct ClipmapRegion
		{
			UVec3 m_probesBegin;
			U32 m_partialUpdate;

			UVec3 m_probeCounts;
			U32 m_probeCount;
		};

		struct ProbeUpdateConsts
		{
			U32 m_clipmapIdx;
			U32 m_radianceOctMapSize; // Have it here as well as well as a mutator. Can't use the mutator cause it will create may raygen variants
			U32 m_rayCountPerTexel; // Ray count per oct map texel
			U32 m_maxProbesToUpdate;

			ClipmapRegion m_clipmapRegion;
		};

		// Do ray tracing around the probes
		{
			NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("IndirectDiffuseClipmaps: RT (clipmap %u)", clipmap));

			pass.newTextureDependency(rtResultHandle, (g_cvarRenderIdcInlineRt) ? TextureUsageBit::kUavCompute : TextureUsageBit::kUavDispatchRays);
			if(!g_cvarRenderIdcInlineRt)
			{
				pass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
			}
			setRgenSpace2Dependencies(pass, g_cvarRenderIdcInlineRt);

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				pass.newTextureDependency(irradianceVolumes[clipmap], TextureUsageBit::kSrvCompute);
			}

			pass.setWork([this, rtResultHandle, &ctx, sbtBuffer, fullUpdateRangeCount, clipmap, fullUpdateRanges, partialUpdateRange,
						  partialUpdateProbeCount](RenderPassWorkContext& rgraphCtx) {
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram((g_cvarRenderIdcInlineRt) ? m_rtMaterialFetchInlineRtGrProg.get() : m_rtLibraryGrProg.get());

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

				ProbeUpdateConsts consts;
				consts.m_clipmapIdx = clipmap;
				consts.m_radianceOctMapSize = g_cvarRenderIdcRadianceOctMapSize;

				// Do full updates
				for(U32 i = 0; i < fullUpdateRangeCount; ++i)
				{
					cmdb.pushDebugMarker("Full update", Vec3(0.0f, 1.0f, 1.0f));

					consts.m_rayCountPerTexel = g_cvarRenderIdcRayCountPerTexelOfNewProbe;
					consts.m_clipmapRegion.m_probesBegin = UVec3(fullUpdateRanges[i].m_begin);
					consts.m_clipmapRegion.m_probeCounts = UVec3(fullUpdateRanges[i].m_end - fullUpdateRanges[i].m_begin);
					consts.m_clipmapRegion.m_probeCount = consts.m_clipmapRegion.m_probeCounts.x() * consts.m_clipmapRegion.m_probeCounts.y()
														  * consts.m_clipmapRegion.m_probeCounts.z();
					consts.m_maxProbesToUpdate = consts.m_clipmapRegion.m_probeCount;
					consts.m_clipmapRegion.m_partialUpdate = 0;

					cmdb.setFastConstants(&consts, sizeof(consts));

					const U32 threadCount =
						consts.m_clipmapRegion.m_probeCount * square<U32>(g_cvarRenderIdcRadianceOctMapSize) * consts.m_rayCountPerTexel;
					if(g_cvarRenderIdcInlineRt)
					{
						cmdb.dispatchCompute((threadCount + 64 - 1) / 64, 1, 1);
					}
					else
					{
						cmdb.dispatchRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
										  threadCount, 1, 1);
					}

					cmdb.popDebugMarker();
				}

				// Do partial updates
				if(partialUpdateProbeCount)
				{
					cmdb.pushDebugMarker("Partial update", Vec3(0.0f, 1.0f, 1.0f));

					consts.m_rayCountPerTexel = 1;
					consts.m_clipmapRegion.m_probesBegin = UVec3(partialUpdateRange.m_begin);
					consts.m_clipmapRegion.m_probeCounts = UVec3(partialUpdateRange.m_end - partialUpdateRange.m_begin);
					consts.m_clipmapRegion.m_probeCount = consts.m_clipmapRegion.m_probeCounts.x() * consts.m_clipmapRegion.m_probeCounts.y()
														  * consts.m_clipmapRegion.m_probeCounts.z();
					consts.m_maxProbesToUpdate = partialUpdateProbeCount;
					consts.m_clipmapRegion.m_partialUpdate = 1;

					cmdb.setFastConstants(&consts, sizeof(consts));

					const U32 threadCount = partialUpdateProbeCount * square<U32>(g_cvarRenderIdcRadianceOctMapSize);
					if(g_cvarRenderIdcInlineRt)
					{
						cmdb.dispatchCompute((threadCount + 64 - 1) / 64, 1, 1);
					}
					else
					{
						cmdb.dispatchRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
										  threadCount, 1, 1);
					}

					cmdb.popDebugMarker();
				}
			});
		}

		// Populate caches
		{
			NonGraphicsRenderPass& pass =
				rgraph.newNonGraphicsRenderPass(generateTempPassName("IndirectDiffuseClipmaps: Populate caches (clipmap %u)", clipmap));

			pass.newTextureDependency(rtResultHandle, TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(radianceVolumes[clipmap], TextureUsageBit::kUavCompute);
			pass.newTextureDependency(probeValidityVolumes[clipmap], TextureUsageBit::kUavCompute);
			pass.newTextureDependency(distanceMomentsVolumes[clipmap], TextureUsageBit::kUavCompute);

			pass.setWork([this, &ctx, rtResultHandle, radianceVolumes, probeValidityVolumes, distanceMomentsVolumes, clipmap, fullUpdateRanges,
						  partialUpdateRange, partialUpdateProbeCount, fullUpdateRangeCount](RenderPassWorkContext& rgraphCtx) {
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram(m_populateCachesGrProg.get());

				rgraphCtx.bindSrv(0, 0, rtResultHandle);

				cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

				rgraphCtx.bindUav(0, 0, radianceVolumes[clipmap]);
				rgraphCtx.bindUav(1, 0, distanceMomentsVolumes[clipmap]);
				rgraphCtx.bindUav(2, 0, probeValidityVolumes[clipmap]);

				ProbeUpdateConsts consts;
				consts.m_clipmapIdx = clipmap;
				consts.m_radianceOctMapSize = g_cvarRenderIdcRadianceOctMapSize;

				// Do full updates
				const U32 numthreads = 64;
				for(U32 i = 0; i < fullUpdateRangeCount; ++i)
				{
					cmdb.pushDebugMarker("Full update", Vec3(0.0f, 1.0f, 1.0f));

					consts.m_rayCountPerTexel = g_cvarRenderIdcRayCountPerTexelOfNewProbe;
					consts.m_clipmapRegion.m_probesBegin = UVec3(fullUpdateRanges[i].m_begin);
					consts.m_clipmapRegion.m_probeCounts = UVec3(fullUpdateRanges[i].m_end - fullUpdateRanges[i].m_begin);
					consts.m_clipmapRegion.m_probeCount = consts.m_clipmapRegion.m_probeCounts.x() * consts.m_clipmapRegion.m_probeCounts.y()
														  * consts.m_clipmapRegion.m_probeCounts.z();
					consts.m_maxProbesToUpdate = consts.m_clipmapRegion.m_probeCount;
					consts.m_clipmapRegion.m_partialUpdate = 0;

					cmdb.setFastConstants(&consts, sizeof(consts));

					U32 threadCount = consts.m_clipmapRegion.m_probeCount * square<U32>(g_cvarRenderIdcRadianceOctMapSize);
					threadCount = (threadCount + numthreads - 1) / numthreads;
					cmdb.dispatchCompute(threadCount, 1, 1);

					cmdb.popDebugMarker();
				}

				// Do partial updates
				if(partialUpdateProbeCount)
				{
					cmdb.pushDebugMarker("Partial update", Vec3(0.0f, 1.0f, 1.0f));

					consts.m_rayCountPerTexel = 1;
					consts.m_clipmapRegion.m_probesBegin = UVec3(partialUpdateRange.m_begin);
					consts.m_clipmapRegion.m_probeCounts = UVec3(partialUpdateRange.m_end - partialUpdateRange.m_begin);
					consts.m_clipmapRegion.m_probeCount = consts.m_clipmapRegion.m_probeCounts.x() * consts.m_clipmapRegion.m_probeCounts.y()
														  * consts.m_clipmapRegion.m_probeCounts.z();
					consts.m_maxProbesToUpdate = partialUpdateProbeCount;
					consts.m_clipmapRegion.m_partialUpdate = 1;

					cmdb.setFastConstants(&consts, sizeof(consts));

					U32 threadCount = consts.m_maxProbesToUpdate * square<U32>(g_cvarRenderIdcRadianceOctMapSize);
					threadCount = (threadCount + numthreads - 1) / numthreads;
					cmdb.dispatchCompute(threadCount, 1, 1);

					cmdb.popDebugMarker();
				}
			});
		}
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

			const Array<Vec4, 3> consts = {Vec4(g_cvarRenderIdcFirstBounceRayDistance), {}, {}};
			cmdb.setFastConstants(&consts, sizeof(consts));

			const U32 width = getRenderer().getInternalResolution().x() / 2;
			const U32 height = getRenderer().getInternalResolution().y() / (!g_cvarRenderIdcApplyHighQuality + 1);
			cmdb.dispatchRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1, width,
							  height, 1);

			g_svarIdcRays.increment(width * height);
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
							  getRenderer().getInternalResolution().y() / (!g_cvarRenderIdcApplyHighQuality + 1));
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
							  getRenderer().getInternalResolution().y() / (!g_cvarRenderIdcApplyHighQuality + 1));
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
