// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderProgramResource.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Clustered deferred light pass.
class LightShading : public RendererObject
{
anki_internal:
	LightShading(Renderer* r);

	~LightShading();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	RenderTargetDescription m_rtDescr;
	FramebufferDescription m_fbDescr;

	// Light shaders
	ShaderProgramResourcePtr m_prog;
	const ShaderProgramResourceVariant* m_progVariant = nullptr;

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderingContext* m_ctx;
	} m_runCtx; ///< Run context.

	/// Called by init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
