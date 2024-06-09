// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

// Forward
extern NumericCVar<F32> g_volumetricLightingAccumulationQualityXYCVar;
extern NumericCVar<F32> g_volumetricLightingAccumulationQualityZCVar;
extern NumericCVar<U32> g_volumetricLightingAccumulationFinalZSplitCVar;

/// @addtogroup renderer
/// @{

/// Volumetric lighting. It accumulates lighting in a volume texture.
class VolumetricLightingAccumulation : public RendererObject
{
public:
	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rts[1];
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	Array<TexturePtr, 2> m_rtTextures;
	ImageResourcePtr m_noiseImage;

	Array<U32, 3> m_volumeSize;

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_rts;
	} m_runCtx; ///< Runtime context.
};
/// @}

} // end namespace anki
