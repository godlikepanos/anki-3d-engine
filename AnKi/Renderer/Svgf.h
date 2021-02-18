// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Spatio-termporal variance filtering.
class Svgf : public RendererObject
{
public:
	Svgf(Renderer* r);

	~Svgf();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

private:
	ShaderProgramResourcePtr m_prog;

	class
	{
	public:
		RenderingContext* m_ctx = nullptr;
	} m_runCtx;
};
/// @}

} // end namespace anki
