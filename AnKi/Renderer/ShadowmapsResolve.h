// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Gr.h>

namespace anki {

// Resolves shadowmaps into a dir light texture (temporally accumulated) and a packed punctual lights texture.
class ShadowmapsResolve : public RendererObject
{
public:
	ShadowmapsResolve()
	{
		registerDebugRenderTarget("ResolvedDirLightShadows");
		registerDebugRenderTarget("ResolvedPunctualLightShadows");
	}

	Error init();

	void populateRenderGraph();

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  [[maybe_unused]] DebugRenderTargetDrawStyle& drawStyle) const override
	{
		handles[0] = (rtName == "ResolvedDirLightShadows") ? m_runCtx.m_dirLightRt : m_runCtx.m_punctualLightsRt;
	}

	RenderTargetHandle getDirLightRt() const
	{
		return m_runCtx.m_dirLightRt;
	}

	RenderTargetHandle getPunctualLightsRt() const
	{
		return m_runCtx.m_punctualLightsRt;
	}

public:
	Array<RendererShaderProgram, 3> m_progs;
	RenderTargetDesc m_punctualLightsRtDescr;
	Array<RendererTexture, 2> m_dirLightShadowTextures;
	Bool m_firstTexImport = true;
	ImageResourcePtr m_noiseImage;

	class
	{
	public:
		RenderTargetHandle m_dirLightRt;
		RenderTargetHandle m_punctualLightsRt;
	} m_runCtx;
};

} // namespace anki
