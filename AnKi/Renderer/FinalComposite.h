// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Post-processing stage.
class FinalComposite : public RendererObject
{
public:
	FinalComposite(Renderer* r);
	~FinalComposite();

	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	/// Load the color grading texture.
	Error loadColorGradingTextureImage(CString filename);

private:
	static constexpr U kLutSize = 16;

	FramebufferDescription m_fbDescr;

	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_grProgs; ///< [Debug on or off]

	ShaderProgramResourcePtr m_defaultVisualizeRenderTargetProg;
	ShaderProgramPtr m_defaultVisualizeRenderTargetGrProg;

	ImageResourcePtr m_lut; ///< Color grading lookup texture.

	Error initInternal();

	void run(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
