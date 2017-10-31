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

/// Downsample the IS and blur it at the same time.
class DownscaleBlur : public RendererObject
{
anki_internal:
	DownscaleBlur(Renderer* r)
		: RendererObject(r)
	{
	}

	~DownscaleBlur();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	U getPassWidth(U pass) const
	{
		return m_passes[min<U>(pass, m_passes.getSize() - 1)].m_width;
	}

	U getPassHeight(U pass) const
	{
		return m_passes[min<U>(pass, m_passes.getSize() - 1)].m_height;
	}

	RenderTargetHandle getPassRt(U pass) const
	{
		return m_runCtx.m_rts[min<U>(pass, m_runCtx.m_rts.getSize() - 1)];
	}

private:
	class Subpass
	{
	public:
		RenderTargetDescription m_rtDescr;
		U32 m_width, m_height;
	};

	DynamicArray<Subpass> m_passes;

	FramebufferDescription m_fbDescr;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	class
	{
	public:
		DynamicArray<RenderTargetHandle> m_rts;
		U32 m_crntPassIdx = MAX_U32;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initSubpass(U idx, const UVec2& inputTexSize);

	void run(const RenderGraph& rgraph, CommandBufferPtr& cmdb);

	/// A RenderPassWorkCallback for the downscall passes.
	static void runCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		DownscaleBlur* self = static_cast<DownscaleBlur*>(userData);
		self->run(rgraph, cmdb);
	}
};
/// @}

} // end namespace anki
