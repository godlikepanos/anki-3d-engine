// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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

/// Motion blur pass.
class MotionBlur : public RendererObject
{
anki_internal:
	MotionBlur(Renderer* r)
		: RendererObject(r)
	{
	}

	~MotionBlur();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	Array<U16, 2> m_workgroupSize = {{16, 16}};

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	RenderTargetDescription m_rtDescr;

	class
	{
	public:
		RenderTargetHandle m_rt;
		const RenderingContext* m_ctx = nullptr;
	} m_runCtx; ///< Runtime context.

	Error initInternal(const ConfigSet& config);

	void run(RenderPassWorkContext& rgraphCtx);

	/// A RenderPassWorkCallback.
	static void runCallback(RenderPassWorkContext& rgraphCtx)
	{
		MotionBlur* const self = scast<MotionBlur*>(rgraphCtx.m_userData);
		self->run(rgraphCtx);
	}
};
/// @}

} // end namespace anki
