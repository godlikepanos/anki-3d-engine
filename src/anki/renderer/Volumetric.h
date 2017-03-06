// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>

namespace anki
{

// Forward
class Volumetric;

/// @addtogroup renderer
/// @{

/// Volumetic main pass.
class VolumetricMain : public RenderingPass
{
	friend class Volumetric;
	friend class VolumetricHBlur;
	friend class VolumetricVBlur;

anki_internal:
	TexturePtr m_rt; ///< vRT

	VolumetricMain(Renderer* r, Volumetric* vol)
		: RenderingPass(r)
		, m_vol(vol)
	{
	}

	~VolumetricMain()
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void setPreRunBarriers(RenderingContext& ctx);
	void run(RenderingContext& ctx);
	void setPostRunBarriers(RenderingContext& ctx);

private:
	Volumetric* m_vol;

	Vec3 m_fogParticleColor = Vec3(1.0);
	Mat3x4 m_prevCameraRot = Mat3x4::getIdentity();

	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;
	FramebufferPtr m_fb;

	TextureResourcePtr m_noiseTex;
};

/// Volumetric blur pass.
class VolumetricHBlur : public RenderingPass
{
	friend class Volumetric;
	friend class VolumetricVBlur;

anki_internal:
	VolumetricHBlur(Renderer* r, Volumetric* vol)
		: RenderingPass(r)
		, m_vol(vol)
	{
	}

	~VolumetricHBlur()
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void setPreRunBarriers(RenderingContext& ctx);
	void run(RenderingContext& ctx);
	void setPostRunBarriers(RenderingContext& ctx);

private:
	Volumetric* m_vol;

	TexturePtr m_rt;
	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;
	FramebufferPtr m_fb;
};

/// Volumetric blur pass.
class VolumetricVBlur : public RenderingPass
{
	friend class Volumetric;

anki_internal:
	VolumetricVBlur(Renderer* r, Volumetric* vol)
		: RenderingPass(r)
		, m_vol(vol)
	{
	}

	~VolumetricVBlur()
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void setPreRunBarriers(RenderingContext& ctx);
	void run(RenderingContext& ctx);
	void setPostRunBarriers(RenderingContext& ctx);

private:
	Volumetric* m_vol;

	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;
	FramebufferPtr m_fb;
};

/// Volumetric effects.
class Volumetric : public RenderingPass
{
	friend class VolumetricMain;
	friend class VolumetricHBlur;
	friend class VolumetricVBlur;

public:
	void setFogParticleColor(const Vec3& col)
	{
		m_main.m_fogParticleColor = col;
	}

anki_internal:
	VolumetricMain m_main;
	VolumetricHBlur m_hblur;
	VolumetricVBlur m_vblur;

	Volumetric(Renderer* r)
		: RenderingPass(r)
		, m_main(r, this)
		, m_hblur(r, this)
		, m_vblur(r, this)
	{
	}

	~Volumetric()
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& config);

private:
	U32 m_width = 0, m_height = 0;
};
/// @}

} // end namespace anki
