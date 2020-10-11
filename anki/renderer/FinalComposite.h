// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/resource/TextureResource.h>

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
	Error loadColorGradingTexture(CString filename);

private:
	static const U LUT_SIZE = 16;

	FramebufferDescription m_fbDescr;

	ShaderProgramResourcePtr m_prog;
	Array2d<ShaderProgramPtr, 2, 2> m_grProgs; ///< [Debug on or off][Dbg render target on or off]

	TextureResourcePtr m_lut; ///< Color grading lookup texture.
	TextureResourcePtr m_blueNoise;

	class
	{
	public:
		RenderingContext* m_ctx = nullptr;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);

	void run(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
