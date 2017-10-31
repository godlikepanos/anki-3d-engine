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

/// Temporal AA resolve.
class TemporalAA : public RendererObject
{
public:
	TemporalAA(Renderer* r);

	~TemporalAA();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_renderRt;
	}

private:
	Array<TexturePtr, 2> m_rtTextures;
	FramebufferDescription m_fbDescr;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	class
	{
	public:
		RenderingContext* m_ctx = nullptr;
		RenderTargetHandle m_renderRt;
		RenderTargetHandle m_historyRt;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(const RenderingContext& ctx, const RenderGraph& rgraph, CommandBufferPtr& cmdb);

	/// A RenderPassWorkCallback for the AA pass.
	static void runCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		TemporalAA* self = static_cast<TemporalAA*>(userData);
		self->run(*self->m_runCtx.m_ctx, rgraph, cmdb);
	}
};
/// @}

} // end namespace anki
