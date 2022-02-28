// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// VolumetricFog effects.
class VolumetricFog : public RendererObject
{
public:
	VolumetricFog(Renderer* r)
		: RendererObject(r)
	{
	}

	~VolumetricFog()
	{
	}

	ANKI_USE_RESULT Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

	const Array<U32, 3>& getVolumeSize() const
	{
		return m_volumeSize;
	}

	/// Get the last cluster split in Z axis that will be affected by lighting.
	U32 getFinalClusterInZ() const
	{
		return m_finalZSplit;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	RenderTargetDescription m_rtDescr;

	U32 m_finalZSplit = 0;

	Array<U32, 2> m_workgroupSize = {};
	Array<U32, 3> m_volumeSize;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx; ///< Runtime context.
};
/// @}

} // end namespace anki
