// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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

	void setFog(const Vec3& color, F32 factor)
	{
		m_fogColor = color;
		m_fogFactor = factor;
	}

anki_internal:
	static const PixelFormat RT_PIXEL_FORMAT;

	Pps(Renderer* r);
	~Pps();

	ANKI_USE_RESULT Error init(const ConfigSet& config);
	void run(RenderingContext& ctx);

	const TexturePtr& getRt() const
	{
		return m_rt;
	}

	TexturePtr& getRt()
	{
		return m_rt;
	}

private:
	FramebufferPtr m_fb;
	ShaderResourcePtr m_frag;
	PipelinePtr m_ppline;
	TexturePtr m_rt;
	ResourceGroupPtr m_rcGroup;

	TextureResourcePtr m_lut; ///< Color grading lookup texture.

	Vec3 m_fogColor = Vec3(1.0);
	F32 m_fogFactor = 1.0;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
};
/// @}

} // end namespace anki
