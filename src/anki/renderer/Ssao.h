// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	static const PixelFormat RT_PIXEL_FORMAT;

	Ssao(Renderer* r)
		: RendererObject(r)
	{
	}

	~Ssao();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

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

	Array<TexturePtr, 2> m_rtTextures;
	GraphicsRenderPassFramebufferDescription m_fbDescr;

	ANKI_USE_RESULT Error initMain(const ConfigSet& set);
	ANKI_USE_RESULT Error initVBlur(const ConfigSet& set);
	ANKI_USE_RESULT Error initHBlur(const ConfigSet& set);

	void runMain(CommandBufferPtr& cmdb, const RenderingContext& ctx, const RenderGraph& rgraph);
	void runHBlur(CommandBufferPtr& cmdb, const RenderGraph& rgraph);
	void runVBlur(CommandBufferPtr& cmdb, const RenderGraph& rgraph);

	/// A RenderPassWorkCallback for SSAO main pass.
	static void runMainCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		Ssao* self = static_cast<Ssao*>(userData);
		self->runMain(cmdb, *self->m_runCtx.m_ctx, rgraph);
	}

	/// A RenderPassWorkCallback for SSAO HBlur.
	static void runHBlurCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		Ssao* self = static_cast<Ssao*>(userData);
		self->runHBlur(cmdb, rgraph);
	}

	/// A RenderPassWorkCallback for SSAO VBlur.
	static void runVBlurCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		Ssao* self = static_cast<Ssao*>(userData);
		self->runVBlur(cmdb, rgraph);
	}
};
/// @}

} // end namespace anki
