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
#include <AnKi/Renderer/Ssao.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
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

// Given the clipmap's position of this and the previous frame it splits the clipmap into regions that contain new probes (thus they need a full
// update) or regions of probes that need a less frequent update.
static void findClipmapInUpdateRanges(Vec3 newClipmapMin, Vec3 oldClipmapMin, Vec3 probeSize, UVec3 probeCountsu,
									  Array<ProbeRange, 3>& fullUpdateProbeRanges, U32& fullUpdateProbeRangeCount,
									  ProbeRange& partialUpdateProbeRange)
{
	fullUpdateProbeRangeCount = 0;

	const IVec3 probeCounts(probeCountsu);

	const IVec3 delta = IVec3((newClipmapMin - oldClipmapMin) / probeSize);
	const IVec3 absDelta = delta.abs();

	if(absDelta.x >= probeCounts.x || absDelta.y >= probeCounts.y || absDelta.z >= probeCounts.z)
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
		if(delta.x > 0)
		{
			// New AABB on the right of old
			fullUpdateProbeRangeBegin = IVec3(partialUpdateProbeRangeEnd.x - delta.x, partialUpdateProbeRangeBegin.y, partialUpdateProbeRangeBegin.z);
			fullUpdateProbeRangeEnd = partialUpdateProbeRangeEnd;

			partialUpdateProbeRangeEnd.x -= delta.x;
		}
		else if(delta.x < 0)
		{
			// New AABB on the left of old
			fullUpdateProbeRangeBegin = partialUpdateProbeRangeBegin;
			fullUpdateProbeRangeEnd = IVec3(-delta.x, partialUpdateProbeRangeEnd.y, partialUpdateProbeRangeEnd.z);

			partialUpdateProbeRangeBegin.x += -delta.x;
		}

		if(delta.x != 0)
		{
			fullUpdateProbeRanges[fullUpdateProbeRangeCount++] = {fullUpdateProbeRangeBegin, fullUpdateProbeRangeEnd};
		}

		fullUpdateProbeRangeBegin = fullUpdateProbeRangeEnd = IVec3(0);
		if(delta.y > 0)
		{
			// New AABB on the top of old
			fullUpdateProbeRangeBegin = IVec3(partialUpdateProbeRangeBegin.x, partialUpdateProbeRangeEnd.y - delta.y, partialUpdateProbeRangeBegin.z);
			fullUpdateProbeRangeEnd = partialUpdateProbeRangeEnd;

			partialUpdateProbeRangeEnd.y -= delta.y;
		}
		else if(delta.y < 0)
		{
			// New AABB at the bottom of old
			fullUpdateProbeRangeBegin = partialUpdateProbeRangeBegin;
			fullUpdateProbeRangeEnd = IVec3(partialUpdateProbeRangeEnd.x, -delta.y, partialUpdateProbeRangeEnd.z);

			partialUpdateProbeRangeBegin.y += -delta.y;
		}

		if(delta.y != 0)
		{
			fullUpdateProbeRanges[fullUpdateProbeRangeCount++] = {fullUpdateProbeRangeBegin, fullUpdateProbeRangeEnd};
		}

		fullUpdateProbeRangeBegin = fullUpdateProbeRangeEnd = IVec3(0);
		if(delta.z > 0)
		{
			// New AABB on the front of old
			fullUpdateProbeRangeBegin = IVec3(partialUpdateProbeRangeBegin.x, partialUpdateProbeRangeBegin.y, partialUpdateProbeRangeEnd.z - delta.z);
			fullUpdateProbeRangeEnd = partialUpdateProbeRangeEnd;

			partialUpdateProbeRangeEnd.z -= delta.z;
		}
		else if(delta.z < 0)
		{
			// New AABB on the back of old
			fullUpdateProbeRangeBegin = partialUpdateProbeRangeBegin;
			fullUpdateProbeRangeEnd = IVec3(partialUpdateProbeRangeEnd.x, partialUpdateProbeRangeEnd.y, -delta.z);

			partialUpdateProbeRangeBegin.z += -delta.z;
		}

		if(delta.z != 0)
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
			ANKI_ASSERT(diff.x * diff.y * diff.z > 0);
			totalProbeCount += diff.x * diff.y * diff.z;
		}

		{
			const IVec3 end = partialUpdateProbeRange.m_end;
			const IVec3 begin = partialUpdateProbeRange.m_begin;
			const IVec3 diff = end - begin;
			ANKI_ASSERT(diff.x * diff.y * diff.z > 0);
			totalProbeCount += diff.x * diff.y * diff.z;
		}
		ANKI_ASSERT(totalProbeCount == probeCounts.x * probeCounts.y * probeCounts.z);
	}
}

static void computeClipmapBounds(Vec3 cameraPos, Vec3 lookDir, U32 clipmapIdx, IndirectDiffuseClipmapConstants& consts)
{
	const Vec3 offset = lookDir * kIndirectDiffuseClipmapForwardBias * F32(clipmapIdx + 1);
	cameraPos += offset;

	const Vec3 halfSize = consts.m_sizes[clipmapIdx].xyz * 0.5f;
	const Vec3 probeSize = consts.m_sizes[clipmapIdx].xyz / Vec3(consts.m_probeCounts);
	const Vec3 roundedPos = (cameraPos / probeSize).round() * probeSize;
	consts.m_aabbMins[clipmapIdx] = (roundedPos - halfSize).xyz0;
	[[maybe_unused]] const Vec3 aabbMax = roundedPos + halfSize;
	ANKI_ASSERT(aabbMax - consts.m_aabbMins[clipmapIdx].xyz == consts.m_sizes[clipmapIdx].xyz);
}

IndirectDiffuseClipmaps::IndirectDiffuseClipmaps()
{
	registerDebugRenderTarget("IndirectDiffuseClipmaps");
}

IndirectDiffuseClipmaps::~IndirectDiffuseClipmaps()
{
}

Error IndirectDiffuseClipmaps::init()
{
	ANKI_CHECK(RtMaterialFetchRendererObject::init());

	UVec2 operatingRez = getRenderer().getInternalResolution();
	if(g_cvarRenderIdcQuarterRez)
	{
		operatingRez /= 2;
	}

	m_colorAndDepthRtDesc1 = getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR16G16B16A16_Sfloat,
																		   "IndirectDiffuseClipmap: Irradiance #1");
	m_colorAndDepthRtDesc1.bake();

	m_colorAndDepthRtDesc2 = getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR16G16B16A16_Sfloat,
																		   "IndirectDiffuseClipmap: Irradiance #2");
	m_colorAndDepthRtDesc2.bake();

	const TextureInitInfo init = getRenderer().create2DRenderTargetInitInfo(
		getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, Format::kR16G16B16A16_Sfloat,
		TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Final"));
	m_finalTex = getRenderer().createAndClearRenderTarget(init, TextureUsageBit::kSrvCompute);

	m_consts.m_probeCounts = UVec3(g_cvarRenderIdcProbesXZ, g_cvarRenderIdcProbesY, g_cvarRenderIdcProbesXZ);
	m_consts.m_totalProbeCount = m_consts.m_probeCounts.x * m_consts.m_probeCounts.y * m_consts.m_probeCounts.z;

	m_consts.m_sizes[0] = Vec3(g_cvarRenderIdcClipmap0XZSize, g_cvarRenderIdcClipmap0YSize, g_cvarRenderIdcClipmap0XZSize).xyz0;
	m_consts.m_sizes[1] = Vec3(g_cvarRenderIdcClipmap1XZSize, g_cvarRenderIdcClipmap1YSize, g_cvarRenderIdcClipmap1XZSize).xyz0;
	m_consts.m_sizes[2] = Vec3(g_cvarRenderIdcClipmap2XZSize, g_cvarRenderIdcClipmap2YSize, g_cvarRenderIdcClipmap2XZSize).xyz0;

	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		TextureInitInfo init = getRenderer().create2DRenderTargetInitInfo(m_consts.m_probeCounts.x, m_consts.m_probeCounts.z, Format::kR8_Unorm,
																		  TextureUsageBit::kUavCompute | TextureUsageBit::kAllSrv,
																		  generateTempPassName("IndirectDiffuseClipmap: Probe validity #%u", i));

		init.m_depth = m_consts.m_probeCounts.y;
		init.m_type = TextureType::k3D;

		m_probeValidityVolumes[i] = getRenderer().createAndClearRenderTarget(init, TextureUsageBit::kSrvCompute);
	}

	// Create the RT result texture
	const U32 texelsPerProbe = square<U32>(g_cvarRenderIdcRadianceOctMapSize);
	m_probeRtResultRtDesc =
		getRenderer().create2DRenderTargetDescription(m_consts.m_totalProbeCount, texelsPerProbe * g_cvarRenderIdcRayCountPerTexelOfNewProbe,
													  Format::kR16G16B16A16_Sfloat, "IndirectDiffuseClipmap: Probe RT result");
	m_probeRtResultRtDesc.bake();

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_consts.m_probeCounts.x * (g_cvarRenderIdcRadianceOctMapSize + 2), m_consts.m_probeCounts.z * (g_cvarRenderIdcRadianceOctMapSize + 2),
			Format::kB10G11R11_Ufloat_Pack32, TextureUsageBit::kAllShaderResource,
			generateTempPassName("IndirectDiffuseClipmap: Radiance #%u", clipmap));
		volumeInit.m_depth = m_consts.m_probeCounts.y;
		volumeInit.m_type = TextureType::k3D;

		m_radianceVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_consts.m_probeCounts.x * (g_cvarRenderIdcIrradianceOctMapSize + 2),
			m_consts.m_probeCounts.z * (g_cvarRenderIdcIrradianceOctMapSize + 2), Format::kB10G11R11_Ufloat_Pack32,
			TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Irradiance #%u", clipmap));
		volumeInit.m_depth = m_consts.m_probeCounts.y;
		volumeInit.m_type = TextureType::k3D;

		m_irradianceVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_consts.m_probeCounts.x * (g_cvarRenderIdcRadianceOctMapSize + 2), m_consts.m_probeCounts.z * (g_cvarRenderIdcRadianceOctMapSize + 2),
			Format::kR16G16_Sfloat, TextureUsageBit::kAllShaderResource, generateTempPassName("IndirectDiffuseClipmap: Dist moments #%u", clipmap));
		volumeInit.m_depth = m_consts.m_probeCounts.y;
		volumeInit.m_type = TextureType::k3D;

		m_distanceMomentsVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
			m_consts.m_probeCounts.x, m_consts.m_probeCounts.z, Format::kB10G11R11_Ufloat_Pack32, TextureUsageBit::kAllShaderResource,
			generateTempPassName("IndirectDiffuseClipmap: Avg light #%u", clipmap));
		volumeInit.m_depth = m_consts.m_probeCounts.y;
		volumeInit.m_type = TextureType::k3D;

		m_avgIrradianceVolumes[clipmap] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);
	}

	Array<SubMutation, 6> mutation = {{{"RT_MATERIAL_FETCH_TYPE", 0}, // Keep it first
									   {"GPU_WAVE_SIZE", MutatorValue(GrManager::getSingleton().getDeviceCapabilities().m_maxWaveSize)},
									   {"RADIANCE_OCTAHEDRON_MAP_SIZE", MutatorValue(g_cvarRenderIdcRadianceOctMapSize)},
									   {"IRRADIANCE_OCTAHEDRON_MAP_SIZE", MutatorValue(g_cvarRenderIdcIrradianceOctMapSize)},
									   {"QUARTER_REZ", g_cvarRenderIdcQuarterRez},
									   {"IRRADIANCE_USE_SH_L2", g_cvarRenderIdcUseSHL2}}};

	constexpr CString kProgFname = "ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin";
	ShaderProgramResourcePtr mainProgRsrc;
	ANKI_CHECK(ResourceManager::getSingleton().loadResource(kProgFname, mainProgRsrc)); // Keep it alive to avoid reloading

	ANKI_CHECK(m_applyProbeIrradianceProg.load(kProgFname, mutation, "ApplyProbeIrradiance"));
	ANKI_CHECK(m_visProbesProg.load(kProgFname, mutation, "VisualizeProbes"));
	ANKI_CHECK(m_populateCachesProg.load(kProgFname, mutation, "PopulateCaches"));
	ANKI_CHECK(m_probeIrradianceProg.load(kProgFname, mutation, "ProbeIrradiance"));
	ANKI_CHECK(m_temporalDenoiseProg.load(kProgFname, mutation, "TemporalDenoise"));
	ANKI_CHECK(m_bilateralDenoiseProg.load(kProgFname, mutation, "BilateralDenoise"));
	ANKI_CHECK(m_probeInlineRtProg.load(kProgFname, mutation, "ProbeInlineRt"));
	ANKI_CHECK(m_applyGiUsingInlineRtProg.load(kProgFname, mutation, "ApplyInlineRt"));
	ANKI_CHECK(m_upscaleProg.load(kProgFname, mutation, "Upscale"));

	mutation[0].m_value = MutatorValue(RtMaterialFetchType::kApply);
	ANKI_CHECK(m_rtMaterialFetchProg[RtMaterialFetchType::kApply].load(kProgFname, mutation, "RtMaterialFetch", ShaderTypeBit::kRayGen));

	mutation[0].m_value = MutatorValue(RtMaterialFetchType::kProbe);
	ANKI_CHECK(m_rtMaterialFetchProg[RtMaterialFetchType::kProbe].load(kProgFname, mutation, "RtMaterialFetch", ShaderTypeBit::kRayGen));

	ANKI_CHECK(m_missProg.load("ShaderBinaries/RtMaterialFetchMiss.ankiprogbin", {}, "RtMaterialFetch", ShaderTypeBit::kMiss));

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

		m_consts.m_textures[i].m_distanceMomentsOctMapSize = (m_distanceMomentsVolumes[i]->getWidth() / m_consts.m_probeCounts.x) - 2;
		m_consts.m_textures[i].m_irradianceOctMapSize = (m_irradianceVolumes[i]->getWidth() / m_consts.m_probeCounts.x) - 2;
		m_consts.m_textures[i].m_radianceOctMapSize = (m_radianceVolumes[i]->getWidth() / m_consts.m_probeCounts.x) - 2;

		m_consts.m_previousFrameAabbMins[i] = 100000.0f * m_consts.m_sizes[i] / Vec4(Vec3(m_consts.m_probeCounts), 1.0f);
		m_consts.m_aabbMins[i] = m_consts.m_previousFrameAabbMins[i];
	}

	return Error::kNone;
}

void IndirectDiffuseClipmaps::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseClipmaps);

	const Bool firstBounceUsesRt = g_cvarRenderIdcFirstBounceRayDistance > 0.0f;
	const Bool bQuarterRez = g_cvarRenderIdcQuarterRez;

	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		m_consts.m_previousFrameAabbMins[i] = m_consts.m_aabbMins[i];

		computeClipmapBounds(getRenderingContext().m_matrices.m_cameraTransform.getTranslationPart(),
							 -getRenderingContext().m_matrices.m_cameraTransform.getRotationPart().getZAxis().normalize(), i, m_consts);
	}

	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	const RenderTargetHandle probeRtResultRt = rgraph.newRenderTarget(m_probeRtResultRtDesc);
	const RenderTargetHandle tmpRt1 = rgraph.newRenderTarget(m_colorAndDepthRtDesc1);
	RenderTargetHandle tmpRt2;
	if(firstBounceUsesRt)
	{
		tmpRt2 = rgraph.newRenderTarget(m_colorAndDepthRtDesc2);
	}

	const RenderTargetHandle finalRt = rgraph.importRenderTarget(m_finalTex.get(), !m_texturesImportedOnce, TextureUsageBit::kSrvCompute);

	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount>& radianceVolumes = m_runCtx.m_handles.m_radianceVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount>& irradianceVolumes = m_runCtx.m_handles.m_irradianceVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount>& distanceMomentsVolumes = m_runCtx.m_handles.m_distanceMomentsVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount>& probeValidityVolumes = m_runCtx.m_handles.m_probeValidityVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount>& avgIrradianceVolumes = m_runCtx.m_handles.m_avgIrradianceVolumes;
	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		radianceVolumes[i] = rgraph.importRenderTarget(m_radianceVolumes[i].get(), !m_texturesImportedOnce, TextureUsageBit::kSrvCompute);
		irradianceVolumes[i] = rgraph.importRenderTarget(m_irradianceVolumes[i].get(), !m_texturesImportedOnce, TextureUsageBit::kSrvCompute);
		distanceMomentsVolumes[i] =
			rgraph.importRenderTarget(m_distanceMomentsVolumes[i].get(), !m_texturesImportedOnce, TextureUsageBit::kSrvCompute);
		probeValidityVolumes[i] = rgraph.importRenderTarget(m_probeValidityVolumes[i].get(), !m_texturesImportedOnce, TextureUsageBit::kSrvCompute);
		avgIrradianceVolumes[i] = rgraph.importRenderTarget(m_avgIrradianceVolumes[i].get(), !m_texturesImportedOnce, TextureUsageBit::kSrvCompute);
	}

	m_texturesImportedOnce = true;

	// SBT build
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	if(!g_cvarRenderIdcInlineRt)
	{
		buildShaderBindingTablePass("IndirectDiffuseClipmaps: Build SBT",
									m_rtMaterialFetchProg[RtMaterialFetchType::kProbe].getShaderGroupHandlesBuffer(),
									m_rtMaterialFetchProg[RtMaterialFetchType::kProbe].getShaderGroupHandleIndex(),
									m_missProg.getShaderGroupHandleIndex(), m_sbtRecordSize, rgraph, sbtHandle, sbtBuffer);
	}

	for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
	{
		// Compute probe ranges and ray budgets and stuff
		Array<ProbeRange, 3> fullUpdateRanges;
		U32 fullUpdateRangeCount = 0;
		ProbeRange partialUpdateRange;
		const Vec3 probeSize = m_consts.m_sizes[clipmap].xyz / Vec3(m_consts.m_probeCounts);
		findClipmapInUpdateRanges(m_consts.m_aabbMins[clipmap].xyz, m_consts.m_previousFrameAabbMins[clipmap].xyz, probeSize,
								  UVec3(m_consts.m_probeCounts), fullUpdateRanges, fullUpdateRangeCount, partialUpdateRange);

		U32 fullUpdateRayCount = 0;
		for(U32 i = 0; i < fullUpdateRangeCount; ++i)
		{
			const UVec3 counts = UVec3(fullUpdateRanges[i].m_end - fullUpdateRanges[i].m_begin);
			const U32 count = counts.x * counts.y * counts.z;
			fullUpdateRayCount += square<U32>(g_cvarRenderIdcRadianceOctMapSize) * g_cvarRenderIdcRayCountPerTexelOfNewProbe * count;
		}

		const U32 remainingRayCount = (g_cvarRenderIdcProbeRayBudget / kIndirectDiffuseClipmapCount > fullUpdateRayCount)
										  ? g_cvarRenderIdcProbeRayBudget / kIndirectDiffuseClipmapCount - fullUpdateRayCount
										  : 0;

		const UVec3 partialUpdateProbeCounts = UVec3(partialUpdateRange.m_end - partialUpdateRange.m_begin);
		U32 partialUpdateProbeCount = remainingRayCount / square<U32>(g_cvarRenderIdcRadianceOctMapSize);
		partialUpdateProbeCount = min(partialUpdateProbeCount, partialUpdateProbeCounts.x * partialUpdateProbeCounts.y * partialUpdateProbeCounts.z);

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

			pass.newTextureDependency(probeRtResultRt, (g_cvarRenderIdcInlineRt) ? TextureUsageBit::kUavCompute : TextureUsageBit::kUavDispatchRays);
			if(!g_cvarRenderIdcInlineRt)
			{
				pass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
			}
			setRgenSpace2Dependencies(pass, g_cvarRenderIdcInlineRt);

			for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
			{
				pass.newTextureDependency(irradianceVolumes[clipmap], TextureUsageBit::kSrvCompute);
			}

			pass.setWork([this, probeRtResultRt, sbtBuffer, fullUpdateRangeCount, clipmap, fullUpdateRanges, partialUpdateRange,
						  partialUpdateProbeCount](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(IdcProbeRt);
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram((g_cvarRenderIdcInlineRt) ? m_probeInlineRtProg.get()
																 : m_rtMaterialFetchProg[RtMaterialFetchType::kProbe].get());

				// More globals
#include <AnKi/Shaders/Include/MaterialBindings.def.h>

				bindRgenSpace2Resources(rgraphCtx);

				rgraphCtx.bindUav(0, 2, probeRtResultRt);

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
					consts.m_clipmapRegion.m_probeCount =
						consts.m_clipmapRegion.m_probeCounts.x * consts.m_clipmapRegion.m_probeCounts.y * consts.m_clipmapRegion.m_probeCounts.z;
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
					consts.m_clipmapRegion.m_probeCount =
						consts.m_clipmapRegion.m_probeCounts.x * consts.m_clipmapRegion.m_probeCounts.y * consts.m_clipmapRegion.m_probeCounts.z;
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

			pass.newTextureDependency(probeRtResultRt, TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(radianceVolumes[clipmap], TextureUsageBit::kUavCompute);
			pass.newTextureDependency(probeValidityVolumes[clipmap], TextureUsageBit::kUavCompute);
			pass.newTextureDependency(distanceMomentsVolumes[clipmap], TextureUsageBit::kUavCompute);

			pass.setWork([this, probeRtResultRt, radianceVolumes, probeValidityVolumes, distanceMomentsVolumes, clipmap, fullUpdateRanges,
						  partialUpdateRange, partialUpdateProbeCount, fullUpdateRangeCount](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(IdcPopulateCaches);
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram(m_populateCachesProg.get());

				rgraphCtx.bindSrv(0, 0, probeRtResultRt);

				cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

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
					consts.m_clipmapRegion.m_probeCount =
						consts.m_clipmapRegion.m_probeCounts.x * consts.m_clipmapRegion.m_probeCounts.y * consts.m_clipmapRegion.m_probeCounts.z;
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
					consts.m_clipmapRegion.m_probeCount =
						consts.m_clipmapRegion.m_probeCounts.x * consts.m_clipmapRegion.m_probeCounts.y * consts.m_clipmapRegion.m_probeCounts.z;
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

		pass.setWork([this, radianceVolumes, irradianceVolumes, avgIrradianceVolumes](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IdcComputeIrradiance);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_probeIrradianceProg.get());

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

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
		if(!g_cvarRenderIdcInlineRt)
		{
			patchShaderBindingTablePass("IndirectDiffuseClipmaps: Patch SBT",
										m_rtMaterialFetchProg[RtMaterialFetchType::kApply].getShaderGroupHandlesBuffer(),
										m_rtMaterialFetchProg[RtMaterialFetchType::kApply].getShaderGroupHandleIndex(),
										m_missProg.getShaderGroupHandleIndex(), m_sbtRecordSize, rgraph, sbtHandle, sbtBuffer);
		}

		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: Apply RT irradiance");

		if(!g_cvarRenderIdcInlineRt)
		{
			pass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		}

		const TextureUsageBit readUsage = (g_cvarRenderIdcInlineRt) ? TextureUsageBit::kSrvCompute : TextureUsageBit::kSrvDispatchRays;
		const TextureUsageBit writeUsage = (g_cvarRenderIdcInlineRt) ? TextureUsageBit::kUavCompute : TextureUsageBit::kUavDispatchRays;

		for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
		{
			pass.newTextureDependency(irradianceVolumes[clipmap], readUsage);
			pass.newTextureDependency(probeValidityVolumes[clipmap], readUsage);
			pass.newTextureDependency(distanceMomentsVolumes[clipmap], readUsage);
		}
		pass.newTextureDependency(getSsao().getRt(), readUsage);
		if(bQuarterRez)
		{
			pass.newTextureDependency(getDepthDownscale().getDepthRt(), readUsage);
		}

		pass.newTextureDependency(tmpRt1, writeUsage);
		setRgenSpace2Dependencies(pass, g_cvarRenderIdcInlineRt);

		pass.setWork([this, sbtBuffer, tmpRt1, bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IdcApplyRtIrradiance);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			// Space 0 globals
#include <AnKi/Shaders/Include/MaterialBindings.def.h>

			bindRgenSpace2Resources(rgraphCtx);
			rgraphCtx.bindSrv(6, 2, getSsao().getRt());
			rgraphCtx.bindUav(0, 2, tmpRt1);
			if(bQuarterRez)
			{
				rgraphCtx.bindSrv(4, 2, getDepthDownscale().getDepthRt());
			}

			const Array<Vec4, 3> consts = {Vec4(g_cvarRenderIdcFirstBounceRayDistance), {}, {}};
			cmdb.setFastConstants(&consts, sizeof(consts));

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			if(g_cvarRenderIdcInlineRt)
			{
				cmdb.bindShaderProgram(m_applyGiUsingInlineRtProg.get());

				dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
			}
			else
			{
				cmdb.bindShaderProgram(m_rtMaterialFetchProg[RtMaterialFetchType::kApply].get());

				cmdb.dispatchRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1, rez.x,
								  rez.y, 1);
			}

			g_svarIdcRays.increment(rez.x * rez.y);
		});
	}
	else
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: Apply probe irradiance");

		pass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getSsao().getRt(), TextureUsageBit::kSrvCompute);
		for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
		{
			pass.newTextureDependency(irradianceVolumes[i], TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(probeValidityVolumes[i], TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(distanceMomentsVolumes[i], TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(avgIrradianceVolumes[i], TextureUsageBit::kSrvCompute);
		}
		pass.newTextureDependency((bQuarterRez) ? tmpRt1 : finalRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, tmpRt1, bQuarterRez, finalRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IdcApplyProbeIrradiance);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_applyProbeIrradianceProg.get());

			rgraphCtx.bindSrv(0, 0, (bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(1, 0, getSsao().getRt());
			cmdb.bindSrv(2, 0, TextureView(&m_blueNoiseImg->getTexture(), TextureSubresourceDesc::firstSurface()));

			rgraphCtx.bindUav(0, 0, (bQuarterRez) ? tmpRt1 : finalRt);

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	// Temporal denoise
	if(firstBounceUsesRt)
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: Temporal denoise");

		pass.newTextureDependency(tmpRt1, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(finalRt, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getAdjustedMotionVectorsRt() : getMotionVectors().getAdjustedMotionVectorsRt(),
								  TextureUsageBit::kSrvCompute);
		pass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(tmpRt2, TextureUsageBit::kUavCompute);

		pass.setWork([this, tmpRt1, tmpRt2, finalRt, bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IdcTemporalDenoise);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_temporalDenoiseProg.get());

			rgraphCtx.bindSrv(0, 0, tmpRt1);
			rgraphCtx.bindSrv(1, 0, finalRt);
			rgraphCtx.bindSrv(2, 0,
							  (bQuarterRez) ? getDepthDownscale().getAdjustedMotionVectorsRt() : getMotionVectors().getAdjustedMotionVectorsRt());
			rgraphCtx.bindSrv(3, 0, (bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt());

			rgraphCtx.bindUav(0, 0, tmpRt2);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	// Bilateral denoise
	if(firstBounceUsesRt)
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: Bilateral denoise");

		pass.newTextureDependency(tmpRt2, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getHistoryLength().getRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency((bQuarterRez) ? tmpRt1 : finalRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, tmpRt1, tmpRt2, bQuarterRez, finalRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IdcDenoise);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_bilateralDenoiseProg.get());

			rgraphCtx.bindSrv(0, 0, tmpRt2);
			rgraphCtx.bindSrv(1, 0, getHistoryLength().getRt());
			rgraphCtx.bindUav(0, 0, (bQuarterRez) ? tmpRt1 : finalRt);

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	// Upscale
	if(bQuarterRez)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps: Upscale");

		rpass.newTextureDependency(tmpRt1, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(finalRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, tmpRt1, finalRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(IdcUpscale);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_upscaleProg.get());

			rgraphCtx.bindSrv(0, 0, tmpRt1);
			rgraphCtx.bindSrv(1, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(2, 0, getGBuffer().getColorRt(2));

			rgraphCtx.bindUav(0, 0, finalRt);

			const UVec2 rez = getRenderer().getInternalResolution() / 2;

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	m_runCtx.m_handles.m_appliedIrradiance = finalRt;
}

void IndirectDiffuseClipmaps::drawDebugProbes(RenderPassWorkContext& rgraphCtx, U8 clipmap, IndirectDiffuseClipmapsProbeType probeType,
											  F32 colorScale) const
{
	ANKI_ASSERT(clipmap < kIndirectDiffuseClipmapCount);
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	cmdb.bindShaderProgram(m_visProbesProg.get());

	const UVec4 consts(clipmap, asU32(colorScale), 0, 0);
	cmdb.setFastConstants(&consts, sizeof(consts));

	cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

	RenderTargetHandle visVolume;
	if(probeType == IndirectDiffuseClipmapsProbeType::kRadiance)
	{
		visVolume = m_runCtx.m_handles.m_radianceVolumes[clipmap];
	}
	else if(probeType == IndirectDiffuseClipmapsProbeType::kIrradiance)
	{
		visVolume = m_runCtx.m_handles.m_irradianceVolumes[clipmap];
	}
	else
	{
		ANKI_ASSERT(probeType == IndirectDiffuseClipmapsProbeType::kAverageIrradiance);
		visVolume = m_runCtx.m_handles.m_avgIrradianceVolumes[clipmap];
	}

	rgraphCtx.bindSrv(0, 0, visVolume);
	rgraphCtx.bindSrv(1, 0, m_runCtx.m_handles.m_probeValidityVolumes[clipmap]);
	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

	cmdb.draw(PrimitiveTopology::kTriangles, 36, m_consts.m_totalProbeCount);
}

void IndirectDiffuseClipmaps::setDependenciesForDrawDebugProbes(RenderPassBase& pass)
{
	for(RenderTargetHandle& handle : m_runCtx.m_handles.m_radianceVolumes)
	{
		pass.newTextureDependency(handle, TextureUsageBit::kSrvPixel);
	}

	for(RenderTargetHandle& handle : m_runCtx.m_handles.m_irradianceVolumes)
	{
		pass.newTextureDependency(handle, TextureUsageBit::kSrvPixel);
	}

	for(RenderTargetHandle& handle : m_runCtx.m_handles.m_avgIrradianceVolumes)
	{
		pass.newTextureDependency(handle, TextureUsageBit::kSrvPixel);
	}
}

} // end namespace anki
