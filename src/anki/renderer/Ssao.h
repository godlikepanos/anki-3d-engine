// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/resource/ShaderResource.h>
#include <anki/resource/TextureResource.h>
#include <anki/Gr.h>
#include <anki/core/Timestamp.h>

namespace anki
{

// Forward
class Ssao;

/// @addtogroup renderer
/// @{

/// Screen space ambient occlusion pass
class SsaoMain : public RenderingPass
{
	friend class SsaoVBlur;
	friend class SsaoHBlur;

anki_internal:
	static constexpr F32 HEMISPHERE_RADIUS = 3.0; // In game units

	SsaoMain(Renderer* r, Ssao* ssao)
		: RenderingPass(r)
		, m_ssao(ssao)
	{
	}

	TexturePtr getRt() const;

	TexturePtr getPreviousRt() const;

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

private:
	Ssao* m_ssao;

	Array<TexturePtr, 2> m_rt;
	Array<FramebufferPtr, 2> m_fb;
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	TextureResourcePtr m_noiseTex;
};

/// Screen space ambient occlusion blur pass.
class SsaoHBlur : public RenderingPass
{
	friend class SsaoVBlur;

anki_internal:
	SsaoHBlur(Renderer* r, Ssao* ssao)
		: RenderingPass(r)
		, m_ssao(ssao)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

private:
	Ssao* m_ssao;

	TexturePtr m_rt;
	FramebufferPtr m_fb;
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
};

/// Screen space ambient occlusion blur pass.
class SsaoVBlur : public RenderingPass
{
anki_internal:
	SsaoVBlur(Renderer* r, Ssao* ssao)
		: RenderingPass(r)
		, m_ssao(ssao)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

private:
	Ssao* m_ssao;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
};

/// Screen space ambient occlusion pass
class Ssao : public RenderingPass
{
	friend class SsaoMain;
	friend class SsaoHBlur;
	friend class SsaoVBlur;

anki_internal:
	static const PixelFormat RT_PIXEL_FORMAT;

	SsaoMain m_main;
	SsaoHBlur m_hblur;
	SsaoVBlur m_vblur;

	Ssao(Renderer* r)
		: RenderingPass(r)
		, m_main(r, this)
		, m_hblur(r, this)
		, m_vblur(r, this)
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	TexturePtr getRt() const
	{
		return m_main.getRt();
	}

private:
	U32 m_width, m_height;
};
/// @}

} // end namespace anki
