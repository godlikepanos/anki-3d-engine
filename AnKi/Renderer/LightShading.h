// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Clustered deferred light pass.
class LightShading : public RendererObject
{
public:
	LightShading()
	{
		registerDebugRenderTarget("LightShading");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	class
	{
	public:
		RenderTargetDesc m_rtDescr;

		// Light shaders
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_lightShading;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		Array<ShaderProgramPtr, 3> m_grProgs;
	} m_skybox;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_applyFog;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx; ///< Run context.

	ShaderProgramResourcePtr m_visualizeRtProg;
	ShaderProgramPtr m_visualizeRtGrProg;

	void run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  ShaderProgramPtr& optionalShaderProgram) const override;
};
/// @}

} // end namespace anki
