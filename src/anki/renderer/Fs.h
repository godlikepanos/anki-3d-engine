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

/// Forward rendering stage. The objects that blend must be handled differently
class Fs : public RenderingPass
{
anki_internal:
	Fs(Renderer* r)
		: RenderingPass(r)
	{
	}

	~Fs();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount) const;

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

	void drawVolumetric(RenderingContext& ctx, CommandBufferPtr cmdb);

	TexturePtr getRt() const
	{
		return m_rt;
	}

	U getWidth() const
	{
		return m_width;
	}

	U getHeight() const
	{
		return m_height;
	}

	FramebufferPtr getFramebuffer() const
	{
		return m_fb;
	}

private:
	U m_width;
	U m_height;
	FramebufferPtr m_fb;
	TexturePtr m_rt;

	class Vol
	{
	public:
		ShaderResourcePtr m_frag;
		ShaderProgramPtr m_prog;
		SamplerPtr m_nearestSampler;
		TextureResourcePtr m_noiseTex;
	} m_vol;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
	ANKI_USE_RESULT Error initVol();
};
/// @}

} // end namespace anki
