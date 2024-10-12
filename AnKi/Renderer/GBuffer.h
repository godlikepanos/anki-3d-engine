// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline NumericCVar<U32> g_hzbWidthCVar("R", "HzbWidth", 512, 16, 4 * 1024, "HZB map width");
inline NumericCVar<U32> g_hzbHeightCVar("R", "HzbHeight", 256, 16, 4 * 1024, "HZB map height");
inline BoolCVar g_gbufferVrsCVar("R", "GBufferVrs", false, "Enable VRS in GBuffer");
inline BoolCVar g_visualizeGiProbesCVar("R", "VisualizeGiProbes", false, "Visualize GI probes");
inline BoolCVar g_visualizeReflectionProbesCVar("R", "VisualizeReflProbes", false, "Visualize reflection probes");

/// G buffer stage. It populates the G buffer
class GBuffer : public RendererObject
{
public:
	GBuffer()
	{
		registerDebugRenderTarget("GBufferNormals");
		registerDebugRenderTarget("GBufferAlbedo");
		registerDebugRenderTarget("GBufferVelocity");
	}

	~GBuffer();

	Error init();

	void importRenderTargets(RenderingContext& ctx);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getColorRt(U idx) const
	{
		return m_runCtx.m_colorRts[idx];
	}

	RenderTargetHandle getDepthRt() const
	{
		return m_runCtx.m_crntFrameDepthRt;
	}

	RenderTargetHandle getPreviousFrameDepthRt() const
	{
		return m_runCtx.m_prevFrameDepthRt;
	}

	const RenderTargetHandle& getHzbRt() const
	{
		return m_runCtx.m_hzbRt;
	}

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override;

	/// Returns a buffer with indices of the visible AABBs. Used in debug drawing.
	void getVisibleAabbsBuffer(BufferView& visibleAaabbIndicesBuffer, BufferHandle& dep) const
	{
		visibleAaabbIndicesBuffer = m_runCtx.m_visibleAaabbIndicesBuffer;
		dep = m_runCtx.m_visibleAaabbIndicesBufferDepedency;
		ANKI_ASSERT(visibleAaabbIndicesBuffer.isValid() && dep.isValid());
	}

private:
	Array<RenderTargetDesc, kGBufferColorRenderTargetCount> m_colorRtDescrs;
	Array<TexturePtr, 2> m_depthRts;
	TexturePtr m_hzbRt;

	ShaderProgramResourcePtr m_visNormalProg;
	ShaderProgramPtr m_visNormalGrProg;

	ShaderProgramResourcePtr m_visualizeProbeProg;
	ShaderProgramPtr m_visualizeGiProbeGrProg;
	ShaderProgramPtr m_visualizeReflProbeGrProg;

	class
	{
	public:
		Array<RenderTargetHandle, kGBufferColorRenderTargetCount> m_colorRts;
		RenderTargetHandle m_crntFrameDepthRt;
		RenderTargetHandle m_prevFrameDepthRt;
		RenderTargetHandle m_hzbRt;

		BufferView m_visibleAaabbIndicesBuffer; ///< Optional
		BufferHandle m_visibleAaabbIndicesBufferDepedency;
	} m_runCtx;
};
/// @}

} // end namespace anki
