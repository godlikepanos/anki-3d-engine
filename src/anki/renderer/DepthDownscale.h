// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

// Forward
class DepthDownscale;

/// @addtogroup renderer
/// @{

/// Downscales the depth buffer a few times.
class DepthDownscale : public RendererObject
{
anki_internal:
	DepthDownscale(Renderer* r)
		: RendererObject(r)
	{
	}

	~DepthDownscale();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	/// Return a depth buffer that is a quarter of the resolution of the renderer.
	RenderTargetHandle getHalfDepthRt() const
	{
		return m_runCtx.m_halfDepthRt;
	}

	/// Return a FP color render target with hierarchical Z (min Z) in it's mips.
	RenderTargetHandle getHiZRt() const
	{
		return m_runCtx.m_hizRt;
	}

private:
	RenderTargetDescription m_depthRtDescr;
	RenderTargetDescription m_hizRtDescr;
	ShaderProgramResourcePtr m_prog;

	class Pass
	{
	public:
		FramebufferDescription m_fbDescr;
		ShaderProgramPtr m_grProg;
	};

	Array<Pass, HIERARCHICAL_Z_MIPMAP_COUNT> m_passes;

	class
	{
	public:
		RenderTargetHandle m_halfDepthRt;
		RenderTargetHandle m_hizRt;
		U m_pass;
	} m_runCtx; ///< Run context.

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(RenderPassWorkContext& rgraphCtx);

	/// A RenderPassWorkCallback for half depth main pass.
	static void runCallback(RenderPassWorkContext& rgraphCtx)
	{
		static_cast<DepthDownscale*>(rgraphCtx.m_userData)->run(rgraphCtx);
	}
};
/// @}

} // end namespace anki
