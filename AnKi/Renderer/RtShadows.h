// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr.h>
#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Util/BitSet.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline BoolCVar g_rtShadowsSvgfCVar("R", "RtShadowsSvgf", false, "Enable or not RT shadows SVGF");
inline NumericCVar<U8> g_rtShadowsSvgfAtrousPassCountCVar("R", "RtShadowsSvgfAtrousPassCount", 3, 1, 20, "Number of atrous passes of SVGF");
inline NumericCVar<U32> g_rtShadowsRaysPerPixelCVar("R", "RtShadowsRaysPerPixel", 1, 1, 8, "Number of shadow rays per pixel");
inline BoolCVar g_rayTracedShadowsCVar("R", "RayTracedShadows", false, "Enable or not ray traced shadows. Ignored if RT is not supported");

/// Similar to ShadowmapsResolve but it's using ray tracing.
class RtShadows : public RendererObject
{
public:
	RtShadows()
	{
		registerDebugRenderTarget("RtShadows");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  ShaderProgramPtr& optionalShaderProgram) const override;

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_upscaledRt;
	}

public:
	/// @name Render targets
	/// @{
	TexturePtr m_historyRt;
	RenderTargetDesc m_intermediateShadowsRtDescr;
	RenderTargetDesc m_upscaledRtDescr;

	Array<TexturePtr, 2> m_momentsRts;

	RenderTargetDesc m_varianceRtDescr;

	TexturePtr m_dummyHistoryLenTex;
	/// @}

	/// @name Programs
	/// @{
	ShaderProgramResourcePtr m_setupBuildSbtProg;
	ShaderProgramPtr m_setupBuildSbtGrProg;

	ShaderProgramResourcePtr m_buildSbtProg;
	ShaderProgramPtr m_buildSbtGrProg;

	ShaderProgramResourcePtr m_rayGenAndMissProg;
	ShaderProgramPtr m_rtLibraryGrProg;
	U32 m_rayGenShaderGroupIdx = kMaxU32;
	U32 m_missShaderGroupIdx = kMaxU32;

	ShaderProgramResourcePtr m_denoiseProg;
	ShaderProgramPtr m_grDenoiseHorizontalProg;
	ShaderProgramPtr m_grDenoiseVerticalProg;

	ShaderProgramResourcePtr m_svgfVarianceProg;
	ShaderProgramPtr m_svgfVarianceGrProg;

	ShaderProgramResourcePtr m_svgfAtrousProg;
	ShaderProgramPtr m_svgfAtrousGrProg;
	ShaderProgramPtr m_svgfAtrousLastPassGrProg;

	ShaderProgramResourcePtr m_upscaleProg;
	ShaderProgramPtr m_upscaleGrProg;
	/// @}

	ImageResourcePtr m_blueNoiseImage;

	U32 m_sbtRecordSize = 256;

	Bool m_rtsImportedOnce = false;
	Bool m_useSvgf = false;
	U8 m_atrousPassCount = 5;

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_intermediateShadowsRts;
		RenderTargetHandle m_historyRt;
		RenderTargetHandle m_upscaledRt;

		RenderTargetHandle m_prevMomentsRt;
		RenderTargetHandle m_currentMomentsRt;

		Array<RenderTargetHandle, 2> m_varianceRts;
	} m_runCtx;

	Error initInternal();

	void runDenoise(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx, Bool horizontal);

	U32 getPassCountWithoutUpscaling() const
	{
		return (m_useSvgf) ? (m_atrousPassCount + 2) : 3;
	}
};
/// @}

} // namespace anki
