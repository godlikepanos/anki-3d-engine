// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

// Clustered deferred light pass.
class LightShading : public RendererObject
{
public:
	LightShading()
	{
		registerDebugRenderTarget("LightShading");
	}

	Error init();

	void populateRenderGraph();

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	class
	{
	public:
		RenderTargetDesc m_rtDescr;

		RendererShaderProgram m_prog;
	} m_lightShading;

	class
	{
	public:
		Array<RendererShaderProgram, 3> m_progs;
	} m_skybox;

	class
	{
	public:
		RendererShaderProgram m_prog;
	} m_applyFog;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;

	void run(RenderPassWorkContext& rgraphCtx);

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  [[maybe_unused]] DebugRenderTargetDrawStyle& drawStyle) const override
	{
		handles[0] = m_runCtx.m_rt;
		drawStyle = DebugRenderTargetDrawStyle::kTonemap;
	}
};

} // end namespace anki
