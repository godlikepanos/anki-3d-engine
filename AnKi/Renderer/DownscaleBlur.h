// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Downsample the IS and blur it at the same time.
class DownscaleBlur : public RendererObject
{
public:
	DownscaleBlur(Renderer* r)
		: RendererObject(r)
	{
	}

	~DownscaleBlur();

	Error init();

	/// Import render targets
	void importRenderTargets(RenderingContext& ctx);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	U32 getPassWidth(U32 pass) const
	{
		return m_rtTex->getWidth() >> min(pass, m_passCount - 1);
	}

	U32 getPassHeight(U32 pass) const
	{
		return m_rtTex->getHeight() >> min(pass, m_passCount - 1);
	}

	U32 getMipmapCount() const
	{
		return m_passCount;
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	const Array<U32, 2> m_workgroupSize = {16, 16};

	U32 m_passCount = 0; ///< It's also the mip count of the m_rtTex.

	TexturePtr m_rtTex;

	DynamicArray<FramebufferDescription> m_fbDescrs;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;

	Error initInternal();

	void run(U32 passIdx, RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
