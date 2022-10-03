// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Gr.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Global illumination.
class IndirectDiffuse : public RendererObject
{
public:
	IndirectDiffuse(Renderer* r)
		: RendererObject(r)
	{
		registerDebugRenderTarget("IndirectDiffuse");
		registerDebugRenderTarget("IndirectDiffuseVrsSri");
	}

	~IndirectDiffuse();

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  ShaderProgramPtr& optionalShaderProgram) const override;

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_mainRtHandles[kWrite];
	}

private:
	Array<TexturePtr, 2> m_rts;
	Bool m_rtsImportedOnce = false;

	static constexpr U32 kRead = 0;
	static constexpr U32 kWrite = 1;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		RenderTargetDescription m_rtHandle;

		ShaderProgramResourcePtr m_visualizeProg;
		ShaderProgramPtr m_visualizeGrProg;

		U32 m_sriTexelDimension = 16;
	} m_vrs;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		FramebufferDescription m_fbDescr;
	} m_main;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		Array<ShaderProgramPtr, 2> m_grProgs;
		FramebufferDescription m_fbDescr;
	} m_denoise;

	class
	{
	public:
		RenderTargetHandle m_sriRt;
		Array<RenderTargetHandle, 2> m_mainRtHandles;
	} m_runCtx;

	Error initInternal();
};
/// @}

} // namespace anki
