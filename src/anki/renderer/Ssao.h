// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/resource/TextureResource.h>
#include <anki/Gr.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Screen space ambient occlusion pass
class Ssao : public RendererObject
{
public:
	static const Format RT_PIXEL_FORMAT = Format::R8_UNORM;

	Ssao(Renderer* r)
		: RendererObject(r)
	{
	}

	~Ssao();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rts[1];
	}

private:
	static const Bool m_useNormal = false;
	static const Bool m_useCompute = true;
	static const Bool m_useSoftBlur = true;
	static const Bool m_blurUseCompute = true;
	U32 m_width, m_height;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		TextureResourcePtr m_noiseTex;
		Array<U32, 2> m_workgroupSize = {};
	} m_main; ///< Main noisy pass.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		Array<U32, 2> m_workgroupSize = {};
	} m_blur; ///< Box blur.

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_rts;
		const RenderingContext* m_ctx = nullptr;
	} m_runCtx; ///< Runtime context.

	Array<RenderTargetDescription, 2> m_rtDescrs;
	FramebufferDescription m_fbDescr;

	ANKI_USE_RESULT Error initMain(const ConfigSet& set);
	ANKI_USE_RESULT Error initBlur(const ConfigSet& set);

	void runMain(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
	void runBlur(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
