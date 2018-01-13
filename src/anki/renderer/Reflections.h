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

/// Reflections pass.
class Reflections : public RendererObject
{
anki_internal:
	Reflections(Renderer* r)
		: RendererObject(r)
	{
	}

	~Reflections();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	RenderTargetDescription m_rtDescr;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	static void runCallback(RenderPassWorkContext& rgraphCtx)
	{
		static_cast<Reflections*>(rgraphCtx.m_userData)->run(rgraphCtx);
	}

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
