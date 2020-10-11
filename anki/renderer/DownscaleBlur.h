// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>

namespace anki
{

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

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

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
	static const Bool m_useCompute = true;
	Array<U32, 2> m_workgroupSize = {0, 0};

	U32 m_passCount = 0; ///< It's also the mip count of the m_rtTex.

	TexturePtr m_rtTex;

	DynamicArray<FramebufferDescription> m_fbDescrs;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	class
	{
	public:
		RenderTargetHandle m_rt;
		U32 m_crntPassIdx = MAX_U32;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initSubpass(U idx, const UVec2& inputTexSize);

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
