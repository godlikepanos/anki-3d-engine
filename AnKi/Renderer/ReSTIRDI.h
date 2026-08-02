// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

ANKI_CVAR(BoolCVar, Render, ReSTIRDI, false, "Enable ReSTIR direct lighting")

// ReSTIR direct lighting
class ReSTIRDI : public RendererObject
{
public:
	ReSTIRDI()
	{
		registerDebugRenderTarget("ReSTIRDI");
	}

	Error init();

	void populateRenderGraph();

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  DebugRenderTargetDrawStyle& drawStyle) const override
	{
		ANKI_ASSERT(rtName == "ReSTIRDI");
		handles[0] = m_runCtx.m_rt;
		drawStyle = DebugRenderTargetDrawStyle::kTonemap;
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	RendererShaderProgram m_groundTruthGrProg;

	RenderTargetDesc m_rtDesc;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;
};

} // namespace anki
