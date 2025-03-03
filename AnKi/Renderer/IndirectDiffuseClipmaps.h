// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/TraditionalDeferredShading.h>
#include <AnKi/Collision/Forward.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline NumericCVar<F32> g_indirectDiffuseClipmap0MetersCVar("R", "IndirectDiffuseClipmap0Meters", 20.0, 10.0, 50.0, "The size of the 1st clipmap");
inline NumericCVar<U32> g_indirectDiffuseClipmap0CellsPerCubicMeterCVar("R", "IndirectDiffuseClipmap0CellsPerCubicMeter", 1, 1, 10 * 3,
																		"Cell count per cubic meter");

/// Ambient global illumination passes.
class IndirectDiffuseProbes2 : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(RenderingContext& ctx);

private:
	static constexpr U32 kClipmapLevelCount = 3;

	Array<TexturePtr, kClipmapLevelCount> m_clipmapLevelTextures;
};
/// @}

} // end namespace anki
