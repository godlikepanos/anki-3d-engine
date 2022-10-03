// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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

/// Similar to ShadowmapsResolve but it's using ray tracing.
class RtShadows : public RendererObject
{
public:
	RtShadows(Renderer* r)
		: RendererObject(r)
	{
		registerDebugRenderTarget("RtShadows");
		registerDebugRenderTarget("RtShadows1");
		registerDebugRenderTarget("RtShadows2");
	}

	~RtShadows();

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  ShaderProgramPtr& optionalShaderProgram) const override;

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_upscaledRt;
	}

public:
	class ShadowLayer
	{
	public:
		U64 m_lightUuid = kMaxU64;
		U64 m_frameLastUsed = kMaxU64;
	};

	/// @name Render targets
	/// @{
	TexturePtr m_historyRt;
	RenderTargetDescription m_intermediateShadowsRtDescr;
	RenderTargetDescription m_upscaledRtDescr;

	Array<TexturePtr, 2> m_momentsRts;

	RenderTargetDescription m_varianceRtDescr;
	/// @}

	/// @name Programs
	/// @{
	ShaderProgramResourcePtr m_rayGenProg;
	ShaderProgramPtr m_rtLibraryGrProg;
	U32 m_rayGenShaderGroupIdx = kMaxU32;

	ShaderProgramResourcePtr m_missProg;
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

	ShaderProgramResourcePtr m_visualizeRenderTargetsProg;
	/// @}

	ImageResourcePtr m_blueNoiseImage;

	Array<ShadowLayer, kMaxRtShadowLayers> m_shadowLayers;

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

		BufferPtr m_sbtBuffer;
		PtrSize m_sbtOffset;
		U32 m_hitGroupCount = 0;

		BitSet<kMaxRtShadowLayers, U8> m_layersWithRejectedHistory = {false};

		U8 m_atrousPassIdx = 0;
		U8 m_denoiseOrientation = 0;
	} m_runCtx;

	Error initInternal();

	void run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
	void runDenoise(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
	void runSvgfVariance(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
	void runSvgfAtrous(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
	void runUpscale(RenderPassWorkContext& rgraphCtx);

	void buildSbt(RenderingContext& ctx);

	Bool findShadowLayer(U64 lightUuid, U32& layerIdx, Bool& rejectHistoryBuffer);

	U32 getPassCountWithoutUpscaling() const
	{
		return (m_useSvgf) ? (m_atrousPassCount + 2) : 3;
	}
};
/// @}

} // namespace anki
