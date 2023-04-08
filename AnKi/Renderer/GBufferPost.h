// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Applies SSAO and decals to the GBuffer. It's a seperate pass because it requres the depth buffer.
class GBufferPost : public RendererObject
{
public:
	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	FramebufferDescription m_fbDescr;

	Error initInternal();

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
