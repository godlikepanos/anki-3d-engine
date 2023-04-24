// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// XXX
class GpuVisibility : public RendererObject
{
public:
	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	class
	{
	public:
		BufferHandle m_instanceRateRenderables;
		BufferHandle m_drawIndexedIndirectArgs;
		BufferHandle m_mdiDrawCounts;
	} m_runCtx;
};
/// @}

} // end namespace anki
