// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

ANKI_CVAR(BoolCVar, Render, Idc, true, "Enable ray traced indirect diffuse clipmaps")
ANKI_CVAR2(BoolCVar, Render, Idc, InlineRt, true, "Use a cheap and less accurate path with inline RT");
ANKI_CVAR2(BoolCVar, Render, Idc, UseSHL2, !ANKI_PLATFORM_MOBILE, "Use L2 SH for calculations. Else use L1");

constexpr U32 kDefaultClipmapProbeCountXZ = 32;
constexpr U32 kDefaultClipmapProbeCountY = 12;
constexpr F32 kDefaultClipmap0ProbeSize = 1.5f;
constexpr F32 kDefaultClipmap1ProbeSize = 3.0f;
constexpr F32 kDefaultClipmap2ProbeSize = 6.0f;
constexpr U32 kDefaultRadianceOctMapSize = 10;
constexpr U32 kDefaultRayCountPerTexelOfNewProbe = 4;

// As if you are updating 25% of the probes each frame.
constexpr U32 kDefaultProbeRayBudget =
	(kIndirectDiffuseClipmapCount * square(kDefaultClipmapProbeCountXZ) * kDefaultClipmapProbeCountY * square(kDefaultRadianceOctMapSize)) * 25 / 100;

ANKI_CVAR2(NumericCVar<U32>, Render, Idc, ProbesXZ, kDefaultClipmapProbeCountXZ, 10, 100, "The cell count of each dimension of 1st clipmap")
ANKI_CVAR2(NumericCVar<U32>, Render, Idc, ProbesY, kDefaultClipmapProbeCountY, 4, 100, "The cell count of each dimension of 1st clipmap")

ANKI_CVAR2(NumericCVar<F32>, Render, Idc, Clipmap0XZSize, F32(kDefaultClipmapProbeCountXZ) * kDefaultClipmap0ProbeSize, 10.0, 1000.0,
		   "The clipmap size in meters")
ANKI_CVAR2(NumericCVar<F32>, Render, Idc, Clipmap0YSize, F32(kDefaultClipmapProbeCountY) * kDefaultClipmap0ProbeSize, 10.0, 1000.0,
		   "The clipmap size in meters")

ANKI_CVAR2(NumericCVar<F32>, Render, Idc, Clipmap1XZSize, F32(kDefaultClipmapProbeCountXZ) * kDefaultClipmap1ProbeSize, 10.0, 1000.0,
		   "The clipmap size in meters")
ANKI_CVAR2(NumericCVar<F32>, Render, Idc, Clipmap1YSize, F32(kDefaultClipmapProbeCountY) * kDefaultClipmap1ProbeSize, 10.0, 1000.0,
		   "The clipmap size in meters")

ANKI_CVAR2(NumericCVar<F32>, Render, Idc, Clipmap2XZSize, F32(kDefaultClipmapProbeCountXZ) * kDefaultClipmap2ProbeSize, 10.0, 1000.0,
		   "The clipmap size in meters")
ANKI_CVAR2(NumericCVar<F32>, Render, Idc, Clipmap2YSize, F32(kDefaultClipmapProbeCountY) * kDefaultClipmap2ProbeSize, 10.0, 1000.0,
		   "The clipmap size in meters")

ANKI_CVAR2(
	NumericCVar<U32>, Render, Idc, RadianceOctMapSize, kDefaultRadianceOctMapSize,
	[](U32 val) {
		return val >= 4 && val <= 30 && val % 2 == 0;
	},
	"Size of the octahedral for the light cache")
ANKI_CVAR2(NumericCVar<U32>, Render, Idc, IrradianceOctMapSize, 5, 4, 20, "Size of the octahedral for the irradiance")

ANKI_CVAR2(NumericCVar<U8>, Render, Idc, RayCountPerTexelOfNewProbe, kDefaultRayCountPerTexelOfNewProbe, 1, 16,
		   "The number of rays for a single texel of the oct map that will be cast for probes that are seen for the 1st time")

ANKI_CVAR2(NumericCVar<U32>, Render, Idc, ProbeRayBudget, kDefaultProbeRayBudget, 1024, 100 * 1024 * 1024,
		   "The number of rays for a single texel of the oct map that will be cast for probes that are seen for the 1st time")

enum class IndirectDiffuseClipmapsProbeType : U8
{
	kRadiance,
	kIrradiance,
	kAverageIrradiance,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(IndirectDiffuseClipmapsProbeType)

inline constexpr Array<const Char*, U32(IndirectDiffuseClipmapsProbeType::kCount)> kIndirectDiffuseClipmapsProbeTypeNames = {"Radiance", "Irradiance",
																															 "Avg Irradiance"};

// Indirect diffuse based on clipmaps of probes.
class IndirectDiffuseClipmaps : public RtMaterialFetchRendererObject
{
public:
	Error init();

	void populateRenderGraph();

	const IndirectDiffuseClipmapConstants& getClipmapConsts() const
	{
		return m_consts;
	}

	void drawDebugProbes(RenderPassWorkContext& rgraphCtx, U8 clipmap, IndirectDiffuseClipmapsProbeType probeType, F32 colorScale) const;

	// Set the dependencies before calling drawDebugProbes()
	void setDependenciesForDrawDebugProbes(RenderPassBase& pass);

	// Output of IndirectDiffuseClipmaps is hidden and bindless so have this function to set dependencies
	void setDependencies(RenderPassBase& pass, TextureUsageBit usage) const
	{
		ANKI_ASSERT(!(usage & ~TextureUsageBit::kAllSrv) && "Only SRV allowed");
		for(U32 clipmap = 0; clipmap < kIndirectDiffuseClipmapCount; ++clipmap)
		{
			pass.newTextureDependency(m_runCtx.m_irradianceVolumes[clipmap], usage);
			pass.newTextureDependency(m_runCtx.m_probeValidityVolumes[clipmap], usage);
			pass.newTextureDependency(m_runCtx.m_distanceMomentsVolumes[clipmap], usage);
		}
	}

private:
	Array<RendererTexture, kIndirectDiffuseClipmapCount> m_radianceVolumes;
	Array<RendererTexture, kIndirectDiffuseClipmapCount> m_irradianceVolumes;
	Array<RendererTexture, kIndirectDiffuseClipmapCount> m_distanceMomentsVolumes;
	Array<RendererTexture, kIndirectDiffuseClipmapCount> m_probeValidityVolumes;
	Array<RendererTexture, kIndirectDiffuseClipmapCount> m_avgIrradianceVolumes;

	RenderTargetDesc m_probeRtResultRtDesc;

	IndirectDiffuseClipmapConstants m_consts;

	RendererRtShaderProgram m_rtMaterialFetchProg;
	RendererRtShaderProgram m_missProg;
	RendererShaderProgram m_probeInlineRtProg;
	RendererShaderProgram m_populateCachesProg;
	RendererShaderProgram m_probeIrradianceProg;
	RendererShaderProgram m_visProbesProg;

	U32 m_sbtRecordSize = 0;

	Bool m_texturesImportedOnce = false;

	class
	{
	public:
		Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_radianceVolumes;
		Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_irradianceVolumes;
		Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_distanceMomentsVolumes;
		Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_probeValidityVolumes;
		Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_avgIrradianceVolumes;
	} m_runCtx;
};

} // end namespace anki
