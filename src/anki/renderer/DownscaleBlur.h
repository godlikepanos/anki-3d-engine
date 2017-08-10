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

/// Downsample the IS and blur it at the same time.
class DownscaleBlur : public RenderingPass
{
anki_internal:
	DownscaleBlur(Renderer* r)
		: RenderingPass(r)
	{
	}

	~DownscaleBlur();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void setPreRunBarriers(RenderingContext& ctx);
	void run(RenderingContext& ctx);
	void setPostRunBarriers(RenderingContext& ctx);

	U getPassWidth(U pass) const
	{
		return m_passes[min(pass, m_passes.getSize() - 1)].m_width;
	}

	U getPassHeight(U pass) const
	{
		return m_passes[min(pass, m_passes.getSize() - 1)].m_height;
	}

	TexturePtr getPassTexture(U pass) const
	{
		return m_passes[min(pass, m_passes.getSize() - 1)].m_rt;
	}

private:
	class Subpass
	{
	public:
		TexturePtr m_rt;
		FramebufferPtr m_fb;
		U32 m_width, m_height;
	};

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	DynamicArray<Subpass> m_passes;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initSubpass(U idx, const UVec2& inputTexSize);
};
/// @}

} // end namespace anki
