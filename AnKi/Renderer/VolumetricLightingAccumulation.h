// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline NumericCVar<F32> g_volumetricLightingAccumulationQualityXYCVar("R", "VolumetricLightingAccumulationQualityXY", 4.0f, 1.0f, 16.0f,
																	  "Quality of XY dimensions of volumetric lights");
inline NumericCVar<F32> g_volumetricLightingAccumulationQualityZCVar("R", "VolumetricLightingAccumulationQualityZ", 4.0f, 1.0f, 16.0f,
																	 "Quality of Z dimension of volumetric lights");
inline NumericCVar<U32> g_volumetricLightingAccumulationFinalZSplitCVar("R", "VolumetricLightingAccumulationFinalZSplit", 26, 1, 256,
																		"Final cluster split that will recieve volumetric lights");

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
