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
class FinalComposite : public RenderingPass
{
public:
	/// Load the color grading texture.
	Error loadColorGradingTexture(CString filename);

anki_internal:
	static const PixelFormat RT_PIXEL_FORMAT;

	FinalComposite(Renderer* r);
	~FinalComposite();

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
	TexturePtr m_rt;

	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_grProgs; ///< One with Dbg and one without

	TextureResourcePtr m_lut; ///< Color grading lookup texture.
	TextureResourcePtr m_blueNoise;

	Bool8 m_sharpenEnabled = false;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
};
/// @}

} // end namespace anki
