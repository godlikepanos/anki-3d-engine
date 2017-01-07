// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Post-processing stage.
class Pps : public RenderingPass
{
public:
	/// Load the color grading texture.
	Error loadColorGradingTexture(CString filename);

anki_internal:
	static const PixelFormat RT_PIXEL_FORMAT;

	Pps(Renderer* r);
	~Pps();

	ANKI_USE_RESULT Error init(const ConfigSet& config);
	ANKI_USE_RESULT Error run(RenderingContext& ctx);

	const TexturePtr& getRt() const
	{
		return m_rt;
	}

	TexturePtr& getRt()
	{
		return m_rt;
	}

private:
	static const U LUT_SIZE = 16;

	FramebufferPtr m_fb;
	Array2d<ShaderResourcePtr, 2, 2> m_frag; ///< One with Dbg and one without
	ShaderResourcePtr m_vert;
	Array2d<ShaderProgramPtr, 2, 2> m_prog; ///< With Dbg, Default FB or not
	TexturePtr m_rt;

	TextureResourcePtr m_lut; ///< Color grading lookup texture.
	TextureResourcePtr m_blueNoise;

	Bool8 m_sharpenEnabled = false;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
};
/// @}

} // end namespace anki
