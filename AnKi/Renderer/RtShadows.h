// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr.h>
#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/TextureResource.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Shaders/Include/RtShadows.h>

namespace anki
{

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

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget(CString rtName, RenderTargetHandle& handle,
							  ShaderProgramPtr& optionalShaderProgram) const override;

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_historyAndFinalRt;
	}

public:
	class ShadowLayer
	{
	public:
		U64 m_lightUuid = MAX_U64;
		U64 m_frameLastUsed = MAX_U64;
	};

	/// @name Render targets
	/// @{
	TexturePtr m_historyAndFinalRt;
	RenderTargetDescription m_renderRt;

	Array<TexturePtr, 2> m_momentsRts;
	Array<TexturePtr, 2> m_historyLengthRts;

	RenderTargetDescription m_varianceRt;
	/// @}

	/// @name Programs
	/// @{
	ShaderProgramResourcePtr m_rayGenProg;
	ShaderProgramPtr m_rtLibraryGrProg;
	U32 m_rayGenShaderGroupIdx = MAX_U32;

	ShaderProgramResourcePtr m_missProg;
	U32 m_missShaderGroupIdx = MAX_U32;

	ShaderProgramResourcePtr m_denoiseProg;
	ShaderProgramPtr m_grDenoiseProg;

	ShaderProgramResourcePtr m_svgfVarianceProg;
	ShaderProgramPtr m_svgfVarianceGrProg;

	ShaderProgramResourcePtr m_svgfAtrousProg;
	ShaderProgramPtr m_svgfAtrousGrProg;

	ShaderProgramResourcePtr m_visualizeRenderTargetsProg;
	/// @}

	Array<ShadowLayer, MAX_RT_SHADOW_LAYERS> m_shadowLayers;

	U32 m_sbtRecordSize = 256;

	Bool m_historyAndFinalRtImportedOnce = false;
	Bool m_useSvgf = false;

	class
	{
	public:
		RenderingContext* m_ctx = nullptr;

		RenderTargetHandle m_renderRt;
		RenderTargetHandle m_historyAndFinalRt;

		BufferPtr m_sbtBuffer;
		PtrSize m_sbtOffset;
		U32 m_hitGroupCount = 0;

		BitSet<MAX_RT_SHADOW_LAYERS, U8> m_layersWithRejectedHistory = {false};
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(RenderPassWorkContext& rgraphCtx);
	void runDenoise(RenderPassWorkContext& rgraphCtx);

	void buildSbt();

	Bool findShadowLayer(U64 lightUuid, U32& layerIdx, Bool& rejectHistoryBuffer);
};
/// @}

} // namespace anki
