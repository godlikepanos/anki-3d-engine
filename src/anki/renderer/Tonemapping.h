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

/// Tonemapping.
class Tonemapping : public RendererObject
{
anki_internal:
	Tonemapping(Renderer* r)
		: RendererObject(r)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void importRenderTargets(RenderingContext& ctx);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderPassBufferHandle getAverageLuminanceBuffer() const
	{
		return m_runCtx.m_buffHandle;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	U8 m_inputTexMip;

	BufferPtr m_luminanceBuff;

	class
	{
	public:
		RenderPassBufferHandle m_buffHandle;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(RenderPassWorkContext& rgraphCtx);

	/// A RenderPassWorkCallback to run the compute pass.
	static void runCallback(RenderPassWorkContext& rgraphCtx)
	{
		Tonemapping* const self = scast<Tonemapping*>(rgraphCtx.m_userData);
		self->run(rgraphCtx);
	}
};
/// @}

} // end namespace anki
