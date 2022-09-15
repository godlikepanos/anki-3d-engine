// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
	LightShading(Renderer* r);

	~LightShading();

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
		RenderTargetDescription m_rtDescr;
		FramebufferDescription m_fbDescr;

		// Light shaders
		ShaderProgramResourcePtr m_prog;
		Array<ShaderProgramPtr, 2> m_grProg;
	} m_lightShading;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		Array<ShaderProgramPtr, 2> m_grProgs;
	} m_skybox;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_applyIndirect;

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

	Error initLightShading();
	Error initSkybox();
	Error initApplyFog();
	Error initApplyIndirect();

	void run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  ShaderProgramPtr& optionalShaderProgram) const override;
};
/// @}

} // end namespace anki
