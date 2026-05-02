// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

// Downscales the depth buffer a few times.
class DepthDownscale : public RendererObject
{
public:
	static constexpr TextureSubresourceDesc kQuarterInternalResolution = TextureSubresourceDesc::surface(0, 0, 0);
	static constexpr TextureSubresourceDesc kEighthInternalResolution = TextureSubresourceDesc::surface(1, 0, 0);

	DepthDownscale() = default;

	~DepthDownscale();

	Error init();

	void populateRenderGraph();

	// Return a FP color render target with hierarchical Z (min Z) in it's mips.
	RenderTargetHandle getDepthRt() const
	{
		return m_runCtx.m_depthRt;
	}

	RenderTargetHandle getNormalsRt() const
	{
		return m_runCtx.m_normalsRt;
	}

	U8 getMipmapCount() const
	{
		return m_mipCount;
	}

private:
	RenderTargetDesc m_depthRtDesc;
	RenderTargetDesc m_normalRtDesc;

	Array<RendererShaderProgram, 2> m_prog;

	SegregatedListsGpuMemoryPoolAllocation m_counterBuffer;

	U8 m_mipCount = 0;

	class
	{
	public:
		RenderTargetHandle m_depthRt;
		RenderTargetHandle m_normalsRt;
	} m_runCtx;

	Error initInternal();
};

} // end namespace anki
