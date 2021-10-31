// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_mainRtHandles[WRITE];
	}

private:
	Array<TexturePtr, 2> m_rts;
	Bool m_rtsImportedOnce = false;

	static constexpr U32 READ = 0;
	static constexpr U32 WRITE = 1;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		F32 m_radius;
		U32 m_sampleCount = 8;
		F32 m_ssaoStrength = 2.5f;
		F32 m_ssaoBias = -0.1f;
	} m_main;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		Array<ShaderProgramPtr, 2> m_grProgs;
		F32 m_sampleCount = 1.0f;
	} m_denoise;

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_mainRtHandles;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
};
/// @}

} // namespace anki
