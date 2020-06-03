// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/resource/TextureResource.h>
#include <anki/Gr.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Screen space global illumination.
class Ssgi : public RendererObject
{
public:
	Ssgi(Renderer* r)
		: RendererObject(r)
	{
		registerDebugRenderTarget("SSGI");
	}

	~Ssgi();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rts[m_main.m_writeRtIdx];
	}

	void getDebugRenderTarget(CString rtName, RenderTargetHandle& handle) const override
	{
		ANKI_ASSERT(rtName == "SSGI");
		handle = m_runCtx.m_rts[m_main.m_writeRtIdx];
	}

private:
	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		Array<ShaderProgramPtr, 2> m_grProg;
		Array<TexturePtr, 2> m_rts;
		TextureResourcePtr m_noiseTex;
		U32 m_maxSteps = 32;
		U32 m_firstStepPixels = 16;
		U32 m_depthLod = 0;
		Bool m_rtImportedOnce = false;
		U8 m_writeRtIdx = 1;
	} m_main;

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_rts;
		RenderingContext* m_ctx ANKI_DEBUG_CODE(= nullptr);
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace
