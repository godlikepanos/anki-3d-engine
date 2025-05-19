// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/TraditionalDeferredShading.h>
#include <AnKi/Collision/Forward.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline BoolCVar g_rtIndirectDiffuseClipmapsCVar("R", "RtIndirectDiffuseClipmaps", false);

constexpr U32 kDefaultClipmapProbeCountXZ = 32;
constexpr U32 kDefaultClipmapProbeCountY = 12;
constexpr F32 kDefaultClipmap0ProbeSize = 1.5f;
constexpr F32 kDefaultClipmap1ProbeSize = 3.0f;
constexpr F32 kDefaultClipmap2ProbeSize = 6.0f;

inline NumericCVar<U32> g_indirectDiffuseClipmapProbesXZCVar("R", "IndirectDiffuseClipmapProbesXZ", kDefaultClipmapProbeCountXZ, 10, 100,
															 "The cell count of each dimension of 1st clipmap");
inline NumericCVar<U32> g_indirectDiffuseClipmapProbesYCVar("R", "IndirectDiffuseClipmapProbesY", kDefaultClipmapProbeCountY, 4, 100,
															"The cell count of each dimension of 1st clipmap");

inline NumericCVar<F32> g_indirectDiffuseClipmap0XZSizeCVar("R", "IndirectDiffuseClipmap0XZSize",
															F32(kDefaultClipmapProbeCountXZ) * kDefaultClipmap0ProbeSize, 10.0, 1000.0,
															"The clipmap size in meters");
inline NumericCVar<F32> g_indirectDiffuseClipmap0YSizeCVar("R", "IndirectDiffuseClipmap0YSize",
														   F32(kDefaultClipmapProbeCountY) * kDefaultClipmap0ProbeSize, 10.0, 1000.0,
														   "The clipmap size in meters");

inline NumericCVar<F32> g_indirectDiffuseClipmap1XZSizeCVar("R", "IndirectDiffuseClipmap1XZSize",
															F32(kDefaultClipmapProbeCountXZ) * kDefaultClipmap1ProbeSize, 10.0, 1000.0,
															"The clipmap size in meters");
inline NumericCVar<F32> g_indirectDiffuseClipmap1YSizeCVar("R", "IndirectDiffuseClipmap1YSize",
														   F32(kDefaultClipmapProbeCountY) * kDefaultClipmap1ProbeSize, 10.0, 1000.0,
														   "The clipmap size in meters");

inline NumericCVar<F32> g_indirectDiffuseClipmap2XZSizeCVar("R", "IndirectDiffuseClipmap2XZSize",
															F32(kDefaultClipmapProbeCountXZ) * kDefaultClipmap2ProbeSize, 10.0, 1000.0,
															"The clipmap size in meters");
inline NumericCVar<F32> g_indirectDiffuseClipmap2YSizeCVar("R", "IndirectDiffuseClipmap2YSize",
														   F32(kDefaultClipmapProbeCountY) * kDefaultClipmap2ProbeSize, 10.0, 1000.0,
														   "The clipmap size in meters");

inline NumericCVar<U32> g_indirectDiffuseClipmapRadianceOctMapSize(
	"R", "IndirectDiffuseClipmapRadianceOctMapSize", 10,
	[](U32 val) {
		return val >= 4 && val <= 30 && val % 2 == 0;
	},
	"Size of the octahedral for the light cache");
inline NumericCVar<U32> g_indirectDiffuseClipmapIrradianceOctMapSize("R", "IndirectDiffuseClipmapIrradianceOctMapSize", 5, 4, 20,
																	 "Size of the octahedral for the irradiance");

/// Indirect diffuse based on clipmaps of probes.
class IndirectDiffuseClipmaps : public RendererObject
{
public:
	IndirectDiffuseClipmaps()
	{
		registerDebugRenderTarget("IndirectDiffuseClipmapsTest");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override
	{
		handles[0] = m_runCtx.m_appliedGiRt;
	}

	const Array<Clipmap, kIndirectDiffuseClipmapCount>& getClipmapsInfo() const
	{
		return m_clipmapInfo;
	}

	void drawDebugProbes(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx) const;

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_appliedGiRt;
	}

private:
	Array<TexturePtr, kIndirectDiffuseClipmapCount> m_radianceVolumes;
	Array<TexturePtr, kIndirectDiffuseClipmapCount> m_irradianceVolumes;
	Array<TexturePtr, kIndirectDiffuseClipmapCount> m_distanceMomentsVolumes;

	RenderTargetDesc m_rtResultRtDesc;
	Array<RenderTargetDesc, kIndirectDiffuseClipmapCount> m_probeValidityRtDescs;
	RenderTargetDesc m_appliedGiRtDesc;

	Array<Clipmap, kIndirectDiffuseClipmapCount> m_clipmapInfo;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramResourcePtr m_missProg;
	ShaderProgramResourcePtr m_sbtProg;
	ShaderProgramPtr m_libraryGrProg;
	ShaderProgramPtr m_populateCachesGrProg;
	ShaderProgramPtr m_computeIrradianceGrProg;
	ShaderProgramPtr m_applyGiGrProg;
	ShaderProgramPtr m_sbtBuildGrProg;
	ShaderProgramPtr m_visProbesGrProg;

	ImageResourcePtr m_blueNoiseImg;

	U32 m_sbtRecordSize = 0;
	U32 m_rayGenShaderGroupIdx = kMaxU32;
	U32 m_missShaderGroupIdx = kMaxU32;

	Bool m_texturesImportedOnce = false;

	class
	{
	public:
		RenderTargetHandle m_appliedGiRt;
		Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_probeValidityRts;
	} m_runCtx;
};
/// @}

} // end namespace anki
