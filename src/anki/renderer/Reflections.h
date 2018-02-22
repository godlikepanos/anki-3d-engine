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

/// Reflections pass.
class Reflections : public RendererObject
{
anki_internal:
	Reflections(Renderer* r)
		: RendererObject(r)
	{
	}

	~Reflections();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getReflectionRt() const
	{
		return m_runCtx.m_reflRt;
	}

	RenderTargetHandle getIrradianceRt() const
	{
		return m_runCtx.m_irradianceRt;
	}

private:
	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_grProg;

	TexturePtr m_reflTex;
	TexturePtr m_irradianceTex;

	Array<U8, 2> m_workgroupSize = {{16, 16}};

	class
	{
	public:
		RenderTargetHandle m_reflRt;
		RenderTargetHandle m_irradianceRt;
		RenderingContext* m_ctx ANKI_DBG_NULLIFY;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	static void runCallback(RenderPassWorkContext& rgraphCtx)
	{
		static_cast<Reflections*>(rgraphCtx.m_userData)->run(rgraphCtx);
	}

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
