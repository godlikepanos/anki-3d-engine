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

inline NumericCVar<U32> g_indirectDiffuseClipmapProbesXZCVar("R", "IndirectDiffuseClipmapProbesXZ", 32, 10, 100,
															 "The cell count of each dimension of 1st clipmap");
inline NumericCVar<U32> g_indirectDiffuseClipmapProbesYCVar("R", "IndirectDiffuseClipmapProbesY", 8, 4, 100,
															"The cell count of each dimension of 1st clipmap");

inline NumericCVar<F32> g_indirectDiffuseClipmap0XZSizeCVar("R", "IndirectDiffuseClipmap0XZSize", 48.0, 10.0, 1000.0, "The clipmap size in meters");
inline NumericCVar<F32> g_indirectDiffuseClipmap0YSizeCVar("R", "IndirectDiffuseClipmap0YSize", 12.0, 10.0, 1000.0, "The clipmap size in meters");

inline NumericCVar<F32> g_indirectDiffuseClipmap1XZSizeCVar("R", "IndirectDiffuseClipmap1XZSize", 96.0, 10.0, 1000.0, "The clipmap size in meters");
inline NumericCVar<F32> g_indirectDiffuseClipmap1YSizeCVar("R", "IndirectDiffuseClipmap1YSize", 24.0, 10.0, 1000.0, "The clipmap size in meters");

inline NumericCVar<F32> g_indirectDiffuseClipmap2XZSizeCVar("R", "IndirectDiffuseClipmap2XZSize", 192.0, 10.0, 1000.0, "The clipmap size in meters");
inline NumericCVar<F32> g_indirectDiffuseClipmap2YSizeCVar("R", "IndirectDiffuseClipmap2YSize", 48.0, 10.0, 1000.0, "The clipmap size in meters");

inline NumericCVar<U32> g_indirectDiffuseClipmapRadianceCacheProbeSize("R", "IndirectDiffuseClipmapLightCacheSize", 10, 5, 30,
																	   "Size of the octahedral for the light cache");
inline NumericCVar<U32> g_indirectDiffuseClipmapDistancesProbeSize("R", "IndirectDiffuseClipmapDistanceSize", 10, 5, 22,
																   "Size of the octahedral for the probe distances");
inline NumericCVar<U32> g_indirectDiffuseClipmapIrradianceProbeSize("R", "IndirectDiffuseClipmapIrradianceSize", 6, 4, 22,
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
		handles[0] = m_runCtx.m_tmpRt;
	}

	const Array<Clipmap, kIndirectDiffuseClipmapCount>& getClipmapsInfo() const
	{
		return m_clipmapInfo;
	}

	void drawDebugProbes(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx) const;

private:
	static constexpr U32 kRaysPerProbePerFrame = 32;

	RenderTargetDesc m_radianceDesc;
	Array<TexturePtr, kIndirectDiffuseClipmapCount> m_radianceVolumes;
	Array<TexturePtr, kIndirectDiffuseClipmapCount> m_irradianceVolumes;

	Array<Clipmap, kIndirectDiffuseClipmapCount> m_clipmapInfo;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramResourcePtr m_missProg;
	ShaderProgramResourcePtr m_sbtProg;
	ShaderProgramPtr m_libraryGrProg;
	ShaderProgramPtr m_populateCachesGrProg;
	ShaderProgramPtr m_computeIrradianceGrProg;
	ShaderProgramPtr m_tmpVisGrProg;
	ShaderProgramPtr m_sbtBuildGrProg;
	ShaderProgramPtr m_visProbesGrProg;

	Array<RenderTargetDesc, kIndirectDiffuseClipmapCount> m_probeValidityRtDescs;

	RenderTargetDesc m_tmpRtDesc; // TODO rm

	ImageResourcePtr m_blueNoiseImg;

	U32 m_sbtRecordSize = 0;
	U32 m_rayGenShaderGroupIdx = kMaxU32;
	U32 m_missShaderGroupIdx = kMaxU32;

	Bool m_texturesImportedOnce = false;

	class
	{
	public:
		RenderTargetHandle m_tmpRt;
		Array<RenderTargetHandle, kIndirectDiffuseClipmapCount> m_probeValidityRts;
	} m_runCtx;
};
/// @}

} // end namespace anki
