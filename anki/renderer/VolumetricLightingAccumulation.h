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
	U32 getFinalClusterInZ() const
	{
		return m_finalClusterZ;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	Array<TexturePtr, 2> m_rtTextures;
	TextureResourcePtr m_noiseTex;

	U32 m_finalClusterZ = 0;

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
