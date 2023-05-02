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
class GPUVisibility : public RendererObject
{
public:
	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	BufferHandle getMDIDrawCountsBufferHandle() const
	{
		return m_runCtx.m_MDIDrawCounts;
	}

	BufferHandle getDrawIndexedIndirectArgsBufferHandle() const
	{
		return m_runCtx.m_drawIndexedIndirectArgs;
	}

	BufferHandle getInstanceRateRenderablesBufferHandle() const
	{
		return m_runCtx.m_instanceRateRenderables;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	class
	{
	public:
		BufferHandle m_instanceRateRenderables;
		BufferHandle m_drawIndexedIndirectArgs;
		BufferHandle m_MDIDrawCounts;
	} m_runCtx;
};
/// @}

} // end namespace anki
