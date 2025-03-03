// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuseProbes2.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/PrimaryNonRenderableVisibility.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>

namespace anki {

void IndirectDiffuseProbes2::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuse);

	m_runCtx = {};
}

} // end namespace anki
