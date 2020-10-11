// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Temporal AA resolve.
class TemporalAA : public RendererObject
{
public:
	TemporalAA(Renderer* r);

	~TemporalAA();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_renderRt;
	}

private:
	Array<TexturePtr, 2> m_rtTextures;

	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_grProgs;

	Array<U32, 2> m_workgroupSize = {};

	class
	{
	public:
		RenderingContext* m_ctx = nullptr;
		RenderTargetHandle m_renderRt;
		RenderTargetHandle m_historyRt;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);

	void run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
