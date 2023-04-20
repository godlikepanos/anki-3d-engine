// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/GpuVisibility.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>

namespace anki {

void GpuVisibility::populateRenderGraph(RenderingContext& ctx)
{
	// Allocate memory for the indirect commands

	// TODO
}

} // end namespace anki
