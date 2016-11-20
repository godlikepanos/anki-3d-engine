// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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
	void setFog(const Vec3& color, F32 factor)
	{
		m_fogColor = color;
		m_fogFactor = factor;
	}

anki_internal:
	TexturePtr m_rt;

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
	ResourceGroupPtr m_rc;
	ShaderResourcePtr m_frag;
	PipelinePtr m_ppline;
	FramebufferPtr m_fb;

	Vec3 m_fogColor = Vec3(1.0);
	F32 m_fogFactor = 1.0;
};
/// @}

} // end namespace anki
