// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

ANKI_CVAR(NumericCVar<F32>, Render, VrsThreshold, 0.1f, 0.0f, 1.0f, "Threshold under which a lower shading rate will be applied")

// Computes the shading rate image to be used by a number of passes.
class VrsSriGeneration : public RendererObject
{
public:
	VrsSriGeneration()
	{
		registerDebugRenderTarget("VrsSri");
		registerDebugRenderTarget("VrsSriDownscaled");
	}

	Error init();

	void importRenderTargets();

	void populateRenderGraph();

	RenderTargetHandle getSriRt() const
	{
		return m_runCtx.m_rt;
	}

	RenderTargetHandle getDownscaledSriRt() const
	{
		return m_runCtx.m_downscaledRt;
	}

	U8 getSriTexelDimension() const
	{
		return m_sriTexelDimension;
	}

public:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	ShaderProgramResourcePtr m_visualizeProg;
	ShaderProgramPtr m_visualizeGrProg;

	ShaderProgramResourcePtr m_downscaleProg;
	ShaderProgramPtr m_downscaleGrProg;

	RendererTexture m_sriTex;
	RendererTexture m_downscaledSriTex;
	Bool m_sriTexImportedOnce = false;

	U8 m_sriTexelDimension = 16;

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderTargetHandle m_downscaledRt;
	} m_runCtx;

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  [[maybe_unused]] DebugRenderTargetDrawStyle& drawStyle) const override
	{
		if(rtName == "VrsSri")
		{
			handles[0] = m_runCtx.m_rt;
		}
		else
		{
			ANKI_ASSERT(rtName == "VrsSriDownscaled");
			handles[0] = m_runCtx.m_downscaledRt;
		}
	}
};

} // end namespace anki
