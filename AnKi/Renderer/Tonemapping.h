// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Tonemapping.
class Tonemapping : public RendererObject
{
public:
	Tonemapping(Renderer* r)
		: RendererObject(r)
	{
	}

	ANKI_USE_RESULT Error init();

	void importRenderTargets(RenderingContext& ctx);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	BufferHandle getAverageLuminanceBuffer() const
	{
		return m_runCtx.m_buffHandle;
	}

	RenderTargetHandle getExposureRT() const
	{
		return m_runCtx.m_exposureHandle;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	U32 m_inputTexMip;

	BufferPtr m_luminanceBuff;
	TexturePtr m_exposure1x1;

	class
	{
	public:
		BufferHandle m_buffHandle;
		RenderTargetHandle m_exposureHandle;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal();
};
/// @}

} // end namespace anki
