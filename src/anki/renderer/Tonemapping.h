// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
public:
	Tonemapping(Renderer* r)
		: RendererObject(r)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void importRenderTargets(RenderingContext& ctx);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	BufferHandle getAverageLuminanceBuffer() const
	{
		return m_runCtx.m_buffHandle;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	U32 m_inputTexMip;

	BufferPtr m_luminanceBuff;

	class
	{
	public:
		BufferHandle m_buffHandle;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
