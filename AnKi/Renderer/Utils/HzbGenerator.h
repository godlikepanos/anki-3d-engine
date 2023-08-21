// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Hierarchical depth generator.
class HzbGenerator : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(RenderTargetHandle srcDepthRt, UVec2 srcDepthRtSize, RenderTargetHandle dstHzbRt, UVec2 dstHzbRtSize,
							 RenderGraphDescription& rgraph, CString customName = {}) const;

	void populateRenderGraphDirectionalLight(RenderTargetHandle srcDepthRt, UVec2 srcDepthRtSize, ConstWeakArray<RenderTargetHandle> dstHzbRts,
											 ConstWeakArray<Mat4> dstViewProjectionMats, ConstWeakArray<UVec2> dstHzbSizes,
											 const Mat4& invViewProjMat, RenderGraphDescription& rgraph) const;

private:
	static constexpr U32 kMaxSpdMips = 12;

	ShaderProgramResourcePtr m_genPyramidProg;
	ShaderProgramPtr m_genPyramidGrProg;

	ShaderProgramResourcePtr m_maxDepthProg;
	ShaderProgramPtr m_maxDepthGrProg;

	ShaderProgramResourcePtr m_maxBoxProg;
	ShaderProgramPtr m_maxBoxGrProg;

	SamplerPtr m_maxSampler;

	BufferPtr m_counterBuffer;

	BufferPtr m_boxIndexBuffer;

	FramebufferDescription m_fbDescr;

	void populateRenderGraphInternal(RenderTargetHandle srcDepthRt, UVec2 srcDepthRtSize, RenderTargetHandle dstHzbRt, UVec2 dstHzbRtSize,
									 RenderGraphDescription& rgraph, CString customName) const;
};
/// @}

} // end namespace anki
