// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
	/// Load the color grading texture.
	Error loadColorGradingTexture(CString filename);

anki_internal:
	static const Format RT_PIXEL_FORMAT = Format::R8G8B8_UNORM;

	FinalComposite(Renderer* r);
	~FinalComposite();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

private:
	static const U LUT_SIZE = 16;

	FramebufferDescription m_fbDescr;

	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_grProgs; ///< One with Dbg and one without

	TextureResourcePtr m_lut; ///< Color grading lookup texture.
	TextureResourcePtr m_blueNoise;

	class
	{
	public:
		RenderingContext* m_ctx = nullptr;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);

	void run(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);

	/// A RenderPassWorkCallback for the composite pass.
	static void runCallback(RenderPassWorkContext& rgraphCtx)
	{
		FinalComposite* self = scast<FinalComposite*>(rgraphCtx.m_userData);
		self->run(*self->m_runCtx.m_ctx, rgraphCtx);
	}
};
/// @}

} // end namespace anki
