// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/resource/TextureResource.h>
#include <anki/Gr.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Screen space ambient occlusion pass
class Ssao : public RendererObject
{
anki_internal:
	static const Format RT_PIXEL_FORMAT = Format::R8_UNORM;

	Ssao(Renderer* r)
		: RendererObject(r)
	{
	}

	~Ssao();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rts[0];
	}

private:
	U32 m_width, m_height;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		TextureResourcePtr m_noiseTex;
	} m_main; ///< Main noisy pass.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_hblur; ///< Horizontal blur.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_vblur; ///< Vertical blur.

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_rts;
		const RenderingContext* m_ctx = nullptr;
	} m_runCtx; ///< Runtime context.

	RenderTargetDescription m_rtDescr;
	FramebufferDescription m_fbDescr;

	ANKI_USE_RESULT Error initMain(const ConfigSet& set);
	ANKI_USE_RESULT Error initVBlur(const ConfigSet& set);
	ANKI_USE_RESULT Error initHBlur(const ConfigSet& set);

	void runMain(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
	void runHBlur(RenderPassWorkContext& rgraphCtx);
	void runVBlur(RenderPassWorkContext& rgraphCtx);

	/// A RenderPassWorkCallback for SSAO main pass.
	static void runMainCallback(RenderPassWorkContext& rgraphCtx)
	{
		Ssao* const self = scast<Ssao*>(rgraphCtx.m_userData);
		self->runMain(*self->m_runCtx.m_ctx, rgraphCtx);
	}

	/// A RenderPassWorkCallback for SSAO HBlur.
	static void runHBlurCallback(RenderPassWorkContext& rgraphCtx)
	{
		Ssao* const self = scast<Ssao*>(rgraphCtx.m_userData);
		self->runHBlur(rgraphCtx);
	}

	/// A RenderPassWorkCallback for SSAO VBlur.
	static void runVBlurCallback(RenderPassWorkContext& rgraphCtx)
	{
		Ssao* const self = scast<Ssao*>(rgraphCtx.m_userData);
		self->runVBlur(rgraphCtx);
	}
};
/// @}

} // end namespace anki
