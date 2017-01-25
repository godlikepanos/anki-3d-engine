// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Volumetric effects.
class Volumetric : public RenderingPass
{
public:
	void setFogParticleColor(const Vec3& color)
	{
		m_fogParticleColor = color;
	}

anki_internal:
	TexturePtr m_rt; ///< vRT

	Volumetric(Renderer* r)
		: RenderingPass(r)
	{
	}

	~Volumetric();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void setPreRunBarriers(RenderingContext& ctx);
	void run(RenderingContext& ctx);
	void setPostRunBarriers(RenderingContext& ctx);

private:
	U32 m_width = 0, m_height = 0;
	Vec3 m_fogParticleColor = Vec3(1.0);
	Mat3x4 m_prevCameraRot = Mat3x4::getIdentity();

	TexturePtr m_hRt;

	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;
	FramebufferPtr m_fb;

	ShaderResourcePtr m_vFrag;
	ShaderProgramPtr m_vProg;
	FramebufferPtr m_vFb;

	ShaderResourcePtr m_hFrag;
	ShaderProgramPtr m_hProg;
	FramebufferPtr m_hFb;

	TextureResourcePtr m_noiseTex;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
};
/// @}

} // end namespace anki
