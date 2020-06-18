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

/// Screen space reflections.
class Ssr : public RendererObject
{
public:
	Ssr(Renderer* r)
		: RendererObject(r)
	{
		registerDebugRenderTarget("SSR");
	}

	~Ssr();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_grProg;

	TexturePtr m_rt;
	TextureResourcePtr m_noiseTex;

	Array<U32, 2> m_workgroupSize = {};
	U32 m_maxSteps = 32;
	U32 m_depthLod = 0;
	U32 m_firstStepPixels = 16;

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderingContext* m_ctx ANKI_DEBUG_CODE(= nullptr);
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(RenderPassWorkContext& rgraphCtx);

	void getDebugRenderTarget(CString rtName, RenderTargetHandle& handle) const override
	{
		ANKI_ASSERT(rtName == "SSR");
		handle = m_runCtx.m_rt;
	}
};
/// @}

} // end namespace anki
