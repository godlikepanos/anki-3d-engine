// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Applies SSAO and decals to the GBuffer. It's a seperate pass because it requres the depth buffer.
class GBufferPost : public RendererObject
{
anki_internal:
	GBufferPost(Renderer* r)
		: RendererObject(r)
	{
	}

	~GBufferPost();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	FramebufferDescription m_fbDescr;

	class
	{
	public:
		RenderingContext* m_ctx ANKI_DBG_NULLIFY;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	static void runCallback(RenderPassWorkContext& rgraphCtx)
	{
		static_cast<GBufferPost*>(rgraphCtx.m_userData)->run(rgraphCtx);
	}

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
