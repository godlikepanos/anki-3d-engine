// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Volumetric effects.
class Volumetric : public RendererObject
{
public:
	void setFogParticleColor(const Vec3& col)
	{
		m_main.m_fogParticleColor = col;
	}

anki_internal:

	Volumetric(Renderer* r)
		: RendererObject(r)
	{
	}

	~Volumetric()
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const;

private:
	U32 m_width = 0, m_height = 0;

	class
	{
	public:
		Vec3 m_fogParticleColor = Vec3(1.0);
		Mat3x4 m_prevCameraRot = Mat3x4::getIdentity();

		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;

		TextureResourcePtr m_noiseTex;
	} m_main; ///< Main noisy pass.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_hblur; ///< Horizontal blur.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_vblur; ///< Vertical blur.

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_rts;
		const RenderingContext* m_ctx = nullptr;
	} m_runCtx; ///< Runtime context.

	Array<TexturePtr, 2> m_rtTextures;
	FramebufferDescription m_fbDescr;

	ANKI_USE_RESULT Error initMain(const ConfigSet& set);
	ANKI_USE_RESULT Error initVBlur(const ConfigSet& set);
	ANKI_USE_RESULT Error initHBlur(const ConfigSet& set);

	void runMain(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
	void runHBlur(RenderPassWorkContext& rgraphCtx);
	void runVBlur(RenderPassWorkContext& rgraphCtx);

	/// A RenderPassWorkCallback for SSAO main pass.
	static void runMainCallback(RenderPassWorkContext& rgraphCtx)
	{
		Volumetric* const self = scast<Volumetric*>(rgraphCtx.m_userData);
		self->runMain(*self->m_runCtx.m_ctx, rgraphCtx);
	}

	/// A RenderPassWorkCallback for SSAO HBlur.
	static void runHBlurCallback(RenderPassWorkContext& rgraphCtx)
	{
		Volumetric* const self = scast<Volumetric*>(rgraphCtx.m_userData);
		self->runHBlur(rgraphCtx);
	}

	/// A RenderPassWorkCallback for SSAO VBlur.
	static void runVBlurCallback(RenderPassWorkContext& rgraphCtx)
	{
		Volumetric* const self = scast<Volumetric*>(rgraphCtx.m_userData);
		self->runVBlur(rgraphCtx);
	}
};
/// @}

} // end namespace anki
