// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Post-processing stage.
class FinalComposite : public RendererObject
{
public:
	FinalComposite(Renderer* r);
	~FinalComposite();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	/// Load the color grading texture.
	Error loadColorGradingTextureImage(CString filename);

private:
	static const U LUT_SIZE = 16;

	FramebufferDescription m_fbDescr;

	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_grProgs; ///< [Debug on or off]

	ShaderProgramResourcePtr m_defaultVisualizeRenderTargetProg;
	ShaderProgramPtr m_defaultVisualizeRenderTargetGrProg;

	ImageResourcePtr m_lut; ///< Color grading lookup texture.
	ImageResourcePtr m_blueNoise;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);

	void run(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
