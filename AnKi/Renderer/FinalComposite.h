// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline NumericCVar<U32> g_motionBlurSamplesCVar("R", "MotionBlurSamples", 32, 0, 2048, "Max motion blur samples");
inline NumericCVar<F32> g_filmGrainStrengthCVar("R", "FilmGrainStrength", 16.0f, 0.0f, 250.0f, "Film grain strength");

/// Post-processing stage.
class FinalComposite : public RendererObject
{
public:
	FinalComposite() = default;

	~FinalComposite() = default;

	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	/// Load the color grading texture.
	Error loadColorGradingTextureImage(CString filename);

	RenderTargetHandle getRenderTarget() const
	{
		return m_runCtx.m_rt;
	}

private:
	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_grProgs; ///< [Debug on or off]

	ShaderProgramResourcePtr m_defaultVisualizeRenderTargetProg;
	ShaderProgramPtr m_defaultVisualizeRenderTargetGrProg;

	ImageResourcePtr m_lut; ///< Color grading lookup texture.

	RenderTargetDesc m_rtDesc;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;

	Error initInternal();

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
