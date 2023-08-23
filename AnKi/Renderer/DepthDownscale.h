// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Downscales the depth buffer a few times.
class DepthDownscale : public RendererObject
{
public:
	static constexpr TextureSubresourceInfo kQuarterInternalResolution = TextureSurfaceInfo(0, 0, 0, 0);
	static constexpr TextureSubresourceInfo kEighthInternalResolution = TextureSurfaceInfo(1, 0, 0, 0);

	DepthDownscale() = default;

	~DepthDownscale();

	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	/// Return a FP color render target with hierarchical Z (min Z) in it's mips.
	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

	U32 getMipmapCount() const
	{
		return m_mipCount;
	}

private:
	RenderTargetDescription m_rtDescr;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	BufferPtr m_counterBuffer;

	RendererDynamicArray<FramebufferDescription> m_fbDescrs;

	U32 m_mipCount = 0;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx; ///< Run context.

	Error initInternal();
};
/// @}

} // end namespace anki
