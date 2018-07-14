// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Forward rendering stage. The objects that blend must be handled differently
class ForwardShading : public RendererObject
{
anki_internal:
	ForwardShading(Renderer* r)
		: RendererObject(r)
	{
	}

	~ForwardShading();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	void drawUpscale(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);

	U32 getWidth() const
	{
		return m_width;
	}

	U32 getHeight() const
	{
		return m_height;
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	U32 m_width;
	U32 m_height;

	FramebufferDescription m_fbDescr;
	RenderTargetDescription m_rtDescr;

	class Vol
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		TextureResourcePtr m_noiseTex;
	} m_vol;

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderingContext* m_ctx = nullptr;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
	ANKI_USE_RESULT Error initVol();

	/// A RenderPassWorkCallback.
	static void runCallback(RenderPassWorkContext& rgraphCtx)
	{
		ForwardShading* self = scast<ForwardShading*>(rgraphCtx.m_userData);
		self->run(*self->m_runCtx.m_ctx, rgraphCtx);
	}

	void run(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);

	void drawVolumetric(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
