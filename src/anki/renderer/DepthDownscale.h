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

	/// Return a FP color render target with hierarchical Z (min Z) in it's mips.
	RenderTargetHandle getHiZRt() const
	{
		return m_runCtx.m_hizRt;
	}

	U32 getMipmapCount() const
	{
		return m_mipCount;
	}

	void getClientDepthMapInfo(F32*& depthValues, U32& width, U32& height) const
	{
		width = m_copyToBuff.m_lastMipWidth;
		height = m_copyToBuff.m_lastMipHeight;
		ANKI_ASSERT(m_copyToBuff.m_buffAddr);
		depthValues = static_cast<F32*>(m_copyToBuff.m_buffAddr);
	}

private:
	static const U32 MIPS_WRITTEN_PER_PASS = 2;

	RenderTargetDescription m_hizRtDescr;
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	U32 m_mipCount = 0;

	class
	{
	public:
		RenderTargetHandle m_hizRt;
		U8 m_mip;
	} m_runCtx; ///< Run context.

	class
	{
	public:
		BufferPtr m_buff;
		void* m_buffAddr = nullptr;
		U32 m_lastMipWidth = MAX_U32, m_lastMipHeight = MAX_U32;
	} m_copyToBuff; ///< Copy to buffer members.

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
