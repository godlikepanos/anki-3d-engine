// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/RenderingKey.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Performs GPU visibility for some pass.
class GpuVisibility : public RendererObject
{
public:
	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingTechnique technique, const Mat4& viewProjectionMat, Vec3 cameraPosition, RenderTargetHandle hzbRt,
							 RenderGraphDescription& rgraph);

	BufferHandle getMdiDrawCountsBufferHandle() const
	{
		return m_runCtx.m_mdiDrawCounts;
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
		BufferHandle m_mdiDrawCounts;
	} m_runCtx;
};
/// @}

} // end namespace anki
