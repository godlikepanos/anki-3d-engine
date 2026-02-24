// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/TraditionalDeferredShading.h>
#include <AnKi/Collision/Forward.h>

namespace anki {

ANKI_CVAR(BoolCVar, Render, Idc, true, "Enable ray traced indirect diffuse clipmaps")
ANKI_CVAR2(BoolCVar, Render, Idc, InlineRt, false, "Use a cheap and less accurate path with inline RT");
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

ANKI_CVAR2(NumericCVar<F32>, Render, Idc, FirstBounceRayDistance, (ANKI_PLATFORM_MOBILE) ? 0.0f : 10.0f, 0.0f, 10000.0f,
		   "For the 1st bounce shoot rays instead of sampling the clipmaps")
ANKI_CVAR2(BoolCVar, Render, Idc, ApplyHighQuality, false, "If true use 1/2 resolution else use 1/4")
ANKI_CVAR2(NumericCVar<U8>, Render, Idc, RayCountPerTexelOfNewProbe, kDefaultRayCountPerTexelOfNewProbe, 1, 16,
		   "The number of rays for a single texel of the oct map that will be cast for probes that are seen for the 1st time")

ANKI_CVAR2(NumericCVar<U32>, Render, Idc, ProbeRayBudget, kDefaultProbeRayBudget, 1024, 100 * 1024 * 1024,
		   "The number of rays for a single texel of the oct map that will be cast for probes that are seen for the 1st time")

class IndirectDiffuseClipmapsRenderTargetHandles
{
public:
	RenderTargetHandle m_appliedIrradiance;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_radianceVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_irradianceVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_distanceMomentsVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_probeValidityVolumes;
	Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_avgIrradianceVolumes;
};

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
	IndirectDiffuseClipmaps();

	~IndirectDiffuseClipmaps();

	Error init();

	void populateRenderGraph();

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  [[maybe_unused]] DebugRenderTargetDrawStyle& drawStyle) const override
	{
		handles[0] = m_runCtx.m_handles.m_appliedIrradiance;
		drawStyle = DebugRenderTargetDrawStyle::kTonemap;
	}

	const IndirectDiffuseClipmapConstants& getClipmapConsts() const
	{
		return m_consts;
	}

	void drawDebugProbes(RenderPassWorkContext& rgraphCtx, U8 clipmap, IndirectDiffuseClipmapsProbeType probeType, F32 colorScale) const;

	// Set the dependencies before calling drawDebugProbes()
	void setDependenciesForDrawDebugProbes(RenderPassBase& pass);

	const IndirectDiffuseClipmapsRenderTargetHandles& getRts() const
	{
		return m_runCtx.m_handles;
	}

	// Output of IndirectDiffuseClipmaps is hidden and bindless so have this function to set dependencies
	void setDependencies(RenderPassBase& pass, TextureUsageBit usage) const
	{
		ANKI_ASSERT(!(usage & ~TextureUsageBit::kAllSrv) && "Only SRV allowed");
		// Cheat and only wait for the final RT. The rest will have been waited anyway
		pass.newTextureDependency(m_runCtx.m_handles.m_appliedIrradiance, usage);
	}

private:
	Array<RendererTexture, kIndirectDiffuseClipmapCount> m_radianceVolumes;
	Array<RendererTexture, kIndirectDiffuseClipmapCount> m_irradianceVolumes;
	Array<RendererTexture, kIndirectDiffuseClipmapCount> m_distanceMomentsVolumes;
	Array<RendererTexture, kIndirectDiffuseClipmapCount> m_probeValidityVolumes;
	Array<RendererTexture, kIndirectDiffuseClipmapCount> m_avgIrradianceVolumes;

	RenderTargetDesc m_rtResultRtDesc;
	RenderTargetDesc m_lowRezRtDesc;
	RenderTargetDesc m_fullRtDesc;

	Array<RendererTexture, 2> m_irradianceRts;

	IndirectDiffuseClipmapConstants m_consts;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramResourcePtr m_missProg;
	ShaderProgramPtr m_rtLibraryGrProg;
	ShaderProgramPtr m_rtMaterialFetchInlineRtGrProg;
	ShaderProgramPtr m_populateCachesGrProg;
	ShaderProgramPtr m_computeIrradianceGrProg;
	ShaderProgramPtr m_applyGiGrProg;
	ShaderProgramPtr m_applyGiUsingInlineRtGrProg;
	ShaderProgramPtr m_visProbesGrProg;
	ShaderProgramPtr m_temporalDenoiseGrProg;
	ShaderProgramPtr m_spatialReconstructGrProg;
	ShaderProgramPtr m_bilateralDenoiseGrProg;

	BufferView m_shaderGroupHandlesBuff;

	ImageResourcePtr m_blueNoiseImg;

	U32 m_sbtRecordSize = 0;
	Array<U32, 2> m_rayGenShaderGroupIndices = {kMaxU32, kMaxU32};
	U32 m_missShaderGroupIdx = kMaxU32;

	Bool m_texturesImportedOnce = false;

	class
	{
	public:
		IndirectDiffuseClipmapsRenderTargetHandles m_handles;
	} m_runCtx;
};

} // end namespace anki
