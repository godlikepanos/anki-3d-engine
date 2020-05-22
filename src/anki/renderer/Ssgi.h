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
	}

	~Ssgi();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		Array<ShaderProgramPtr, 2> m_grProg;
		TexturePtr m_rt;
		TextureResourcePtr m_noiseTex;
		U32 m_maxSteps = 32;
		U32 m_depthLod = 0;
	} m_main;

	class
	{
	public:
	} m_denoise;

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderingContext* m_ctx ANKI_DEBUG_CODE(= nullptr);
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace
