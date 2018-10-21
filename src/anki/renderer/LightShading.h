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
	class
	{
	public:
		RenderTargetDescription m_rtDescr;
		FramebufferDescription m_fbDescr;

		// Light shaders
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_lightShading;

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
		RenderingContext* m_ctx;
	} m_runCtx; ///< Run context.

	ANKI_USE_RESULT Error initLightShading(const ConfigSet& config);
	ANKI_USE_RESULT Error initApplyFog(const ConfigSet& config);

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
