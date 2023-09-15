// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// @memberof HzbGenerator
class HzbDirectionalLightInput
{
public:
	class Cascade
	{
	public:
		RenderTargetHandle m_hzbRt;
		UVec2 m_hzbRtSize;
		Mat3x4 m_viewMatrix;
		Mat4 m_projectionMatrix;
	};

	RenderTargetHandle m_depthBufferRt;
	UVec2 m_depthBufferRtSize;

	Mat4 m_cameraInverseViewProjectionMatrix;

	Array<Cascade, kMaxShadowCascades> m_cascades;
	U32 m_cascadeCount = 0;
};

/// Hierarchical depth generator.
class HzbGenerator : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(RenderTargetHandle srcDepthRt, UVec2 srcDepthRtSize, RenderTargetHandle dstHzbRt, UVec2 dstHzbRtSize,
							 RenderGraphDescription& rgraph, CString customName = {}) const;

	void populateRenderGraphDirectionalLight(const HzbDirectionalLightInput& in, RenderGraphDescription& rgraph) const;

private:
	class DispatchInput
	{
	public:
		RenderTargetHandle m_srcDepthRt;
		UVec2 m_srcDepthRtSize;
		RenderTargetHandle m_dstHzbRt;
		UVec2 m_dstHzbRtSize;
	};

	ShaderProgramResourcePtr m_genPyramidProg;
	ShaderProgramPtr m_genPyramidGrProg;

	ShaderProgramResourcePtr m_maxDepthProg;
	ShaderProgramPtr m_maxDepthGrProg;

	ShaderProgramResourcePtr m_maxBoxProg;
	ShaderProgramPtr m_maxBoxGrProg;

	SamplerPtr m_maxSampler;

	// This class assumes that the populateRenderGraph and the populateRenderGraphDirectionalLight will be called once per frame
	static constexpr U32 kCounterBufferElementCount = 1 + kMaxShadowCascades; ///< One for the main pass and a few for shadow cascades
	U32 m_counterBufferElementSize = 0;
	BufferPtr m_counterBuffer;

	BufferPtr m_boxIndexBuffer;

	FramebufferDescription m_fbDescr;

#if ANKI_ASSERTIONS_ENABLED
	// Some helper things to make sure that we don't re-use the counters inside a frame
	mutable U64 m_crntFrame = 0;
	mutable U8 m_counterBufferElementUseMask = 0;
#endif

	void populateRenderGraphInternal(ConstWeakArray<DispatchInput> dispatchInputs, U32 firstCounterBufferElement, CString customName,
									 RenderGraphDescription& rgraph) const;
};
/// @}

} // end namespace anki
