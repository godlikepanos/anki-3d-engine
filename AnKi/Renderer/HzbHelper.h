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
class HzbHelper : public RendererObject
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
	ShaderProgramPtr m_genPyramidMainCameraGrProg;
	ShaderProgramPtr m_genPyramidShadowGrProg;

	ShaderProgramResourcePtr m_minMaxDepthProg;
	ShaderProgramPtr m_minMaxDepthGrProg;

	ShaderProgramResourcePtr m_minMaxBoxProg;
	ShaderProgramPtr m_minMaxBoxGrProg;

	SamplerPtr m_maxSampler;

	BufferPtr m_counterBuffer;

	BufferPtr m_boxIndexBuffer;

	FramebufferDescription m_fbDescr;

	void populateRenderGraphInternal(RenderTargetHandle srcDepthRt, UVec2 srcDepthRtSize, RenderTargetHandle dstHzbRt, UVec2 dstHzbRtSize,
									 RenderGraphDescription& rgraph, CString customName, ShaderProgram* prog, Sampler* sampler) const;
};
/// @}

} // end namespace anki
