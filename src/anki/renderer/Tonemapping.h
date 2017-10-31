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

/// Tonemapping.
class Tonemapping : public RendererObject
{
anki_internal:
	Tonemapping(Renderer* r)
		: RendererObject(r)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderPassBufferHandle getAverageLuminanceBuffer() const
	{
		return m_runCtx.m_buffHandle;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	U8 m_rtIdx;

	BufferPtr m_luminanceBuff;

	class
	{
	public:
		RenderPassBufferHandle m_buffHandle;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(const RenderGraph& rgraph, CommandBufferPtr& cmdb);

	/// A RenderPassWorkCallback to run the compute pass.
	static void runCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		Tonemapping* self = static_cast<Tonemapping*>(userData);
		self->run(rgraph, cmdb);
	}
};
/// @}

} // end namespace anki
