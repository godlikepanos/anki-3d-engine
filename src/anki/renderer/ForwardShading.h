// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

	void drawUpscale(RenderingContext& ctx, CommandBufferPtr& cmdb, const RenderGraph& rgraph);

	U32 getWidth() const
	{
		return m_width;
	}

	U32 getHeight() const
	{
		return m_height;
	}

private:
	U32 m_width;
	U32 m_height;

	GraphicsRenderPassFramebufferDescription m_fbDescr;
	RenderTargetDescription m_rtDescr;

	class Vol
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		TextureResourcePtr m_noiseTex;
	} m_vol;

	class Upscale
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		TextureResourcePtr m_noiseTex;
	} m_upscale;

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderingContext* m_ctx = nullptr;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
	ANKI_USE_RESULT Error initVol();
	ANKI_USE_RESULT Error initUpscale();

	/// A RenderPassWorkCallback.
	static void runCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		ForwardShading* self = static_cast<ForwardShading*>(userData);
		self->run(*self->m_runCtx.m_ctx, cmdb, secondLevelCmdbIdx, secondLevelCmdbCount, rgraph);
	}

	void run(RenderingContext& ctx, CommandBufferPtr& cmdb, U threadId, U threadCount, const RenderGraph& rgraph);

	void drawVolumetric(RenderingContext& ctx, CommandBufferPtr& cmdb, const RenderGraph& rgraph);
};
/// @}

} // end namespace anki
