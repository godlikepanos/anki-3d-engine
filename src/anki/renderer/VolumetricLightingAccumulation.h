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

/// Volumetric lighting. It accumulates lighting in a volume texture.
class VolumetricLightingAccumulation : public RendererObject
{
anki_internal:
	VolumetricLightingAccumulation(Renderer* r)
		: RendererObject(r)
	{
	}

	~VolumetricLightingAccumulation()
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	TexturePtr m_rtTex;
	TextureResourcePtr m_noiseTex;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx; ///< Runtime context.

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
