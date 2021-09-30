// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Gr.h>

namespace anki
{

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
	}

	~IndirectDiffuse();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget(CString rtName, RenderTargetHandle& handle,
							  ShaderProgramPtr& optionalShaderProgram) const override
	{
		ANKI_ASSERT(rtName == "IndirectDiffuse");
		handle = m_runCtx.m_mainRtHandles[WRITE];
	}

private:
	Array<TexturePtr, 2> m_rts;
	Array<TexturePtr, 2> m_momentsAndHistoryLengthRts;
	Bool m_rtsImportedOnce = false;

	static constexpr U32 READ = 0;
	static constexpr U32 WRITE = 1;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		U32 m_maxSteps = 32;
		U32 m_stepIncrement = 16;
		U32 m_depthLod = 0;
	} m_main;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		Array<ShaderProgramPtr, 2> m_grProgs;
		F32 m_minSampleCount = 1.0f;
		F32 m_maxSampleCount = 1.0f;
	} m_denoise;

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_mainRtHandles;
		Array<RenderTargetHandle, 2> m_momentsAndHistoryLengthHandles;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
};
/// @}

} // namespace anki
