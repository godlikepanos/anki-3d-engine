// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Executes various compute jobs required by the render queue. It's guaranteed to run before light shading and nothing
/// more. It can access the previous frame's depth buffer.
class GenericCompute : public RendererObject
{
public:
	GenericCompute(Renderer* r)
		: RendererObject(r)
	{
	}

	~GenericCompute();

	Error init()
	{
		return Error::kNone;
	}

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

private:
	void run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
