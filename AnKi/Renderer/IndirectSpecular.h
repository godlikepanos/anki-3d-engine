// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Screen space reflections and probe sampling.
class IndirectSpecular : public RendererObject
{
public:
	IndirectSpecular(Renderer* r)
		: RendererObject(r)
	{
		registerDebugRenderTarget("SSR");
	}

	~IndirectSpecular();

	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rts[kWrite];
	}

private:
	static constexpr U32 kRead = 0;
	static constexpr U32 kWrite = 1;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	Array<TexturePtr, 2> m_rts;
	Bool m_rtsImportedOnce = false;

	FramebufferDescription m_fbDescr;

	ImageResourcePtr m_noiseImage;

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_rts;
	} m_runCtx;

	Error initInternal();

	void run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  ShaderProgramPtr& optionalShaderProgram) const override;
};
/// @}

} // end namespace anki
