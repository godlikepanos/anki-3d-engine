// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Volumetric lighting. It accumulates lighting in a volume texture.
class VolumetricLightingAccumulation : public RendererObject
{
public:
	VolumetricLightingAccumulation(Renderer* r);
	~VolumetricLightingAccumulation();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rts[1];
	}

	/// Get the last cluster split in Z axis that will be affected by lighting.
	U32 getFinalZSplit() const
	{
		return m_finalZSplit;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	Array<TexturePtr, 2> m_rtTextures;
	ImageResourcePtr m_noiseImage;

	U32 m_finalZSplit = 0;

	Array<U32, 3> m_workgroupSize = {};
	Array<U32, 3> m_volumeSize;

	class
	{
	public:
		RenderingContext* m_ctx = nullptr;
		Array<RenderTargetHandle, 2> m_rts;
	} m_runCtx; ///< Runtime context.

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
