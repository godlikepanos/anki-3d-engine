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

	U32 getMipmapCount() const
	{
		return m_passes.getSize();
	}

	void getClientDepthMapInfo(F32*& depthValues, U32& width, U32& height) const
	{
		width = m_copyToBuff.m_lastMipWidth;
		height = m_copyToBuff.m_lastMipHeight;
		ANKI_ASSERT(m_copyToBuff.m_buffAddr);
		depthValues = static_cast<F32*>(m_copyToBuff.m_buffAddr);
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

	DynamicArray<Pass> m_passes;

	class
	{
	public:
		RenderTargetHandle m_halfDepthRt;
		RenderTargetHandle m_hizRt;
		U m_pass;
	} m_runCtx; ///< Run context.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		BufferPtr m_buff;
		void* m_buffAddr = nullptr;
		U32 m_lastMipWidth = MAX_U32, m_lastMipHeight = MAX_U32;
	} m_copyToBuff; ///< Copy to buffer members.

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(RenderPassWorkContext& rgraphCtx);

	void runCopyToBuffer(RenderPassWorkContext& rgraphCtx);

	/// A RenderPassWorkCallback for half depth main pass.
	static void runCallback(RenderPassWorkContext& rgraphCtx)
	{
		static_cast<DepthDownscale*>(rgraphCtx.m_userData)->run(rgraphCtx);
	}

	static void runCopyToBufferCallback(RenderPassWorkContext& rgraphCtx)
	{
		static_cast<DepthDownscale*>(rgraphCtx.m_userData)->runCopyToBuffer(rgraphCtx);
	}
};
/// @}

} // end namespace anki
