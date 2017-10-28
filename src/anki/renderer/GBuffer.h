// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/Gr.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// G buffer stage. It populates the G buffer
class GBuffer : public RendererObject
{
anki_internal:
	GBuffer(Renderer* r)
		: RendererObject(r)
	{
	}

	~GBuffer();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getColorRt(U idx) const
	{
		return m_colorRts[idx];
	}

	RenderTargetHandle getDepthRt() const
	{
		return m_depthRt;
	}

private:
	Array<RenderTargetDescription, GBUFFER_COLOR_ATTACHMENT_COUNT> m_colorRtDescrs;
	RenderTargetDescription m_depthRtDescr;
	GraphicsRenderPassFramebufferDescription m_fbDescr;

	RenderingContext* m_ctx = nullptr;
	Array<RenderTargetHandle, GBUFFER_COLOR_ATTACHMENT_COUNT> m_colorRts;
	RenderTargetHandle m_depthRt;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	// A RenderPassWorkCallback for G-buffer pass.
	static void runCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		GBuffer* self = static_cast<GBuffer*>(userData);
		self->runInThread(cmdb, secondLevelCmdbIdx, secondLevelCmdbCount, *self->m_ctx);
	}

	void runInThread(CommandBufferPtr& cmdb, U32 threadId, U32 threadCount, const RenderingContext& ctx) const;
};
/// @}

} // end namespace anki
