// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

// VolumetricFog effects.
class VolumetricFog : public RendererObject
{
public:
	Error init();

	void populateRenderGraph();

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	RenderTargetDesc m_rtDescr;

	UVec3 m_volumeSize;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx; // Runtime context.
};

} // end namespace anki
