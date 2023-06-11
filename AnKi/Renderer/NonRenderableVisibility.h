// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr.h>
#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// XXX
class NonRenderableVisibility : public RendererObject
{
public:
	void populateRenderGraph(RenderingContext& rgraph);

private:
};
/// @}

} // end namespace anki
