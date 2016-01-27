// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>

namespace anki
{

// Forward
class Ir;

/// @addtogroup renderer
/// @{

/// Reflections.
class Refl : public RenderingPass
{
anki_internal:
	Refl(Renderer* r);

	~Refl();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	/// Run after IS.
	void run(CommandBufferPtr cmdb);

	TexturePtr getRt() const
	{
		return m_rt;
	}

private:
	U32 m_width = 0;
	U32 m_height = 0;

	Bool8 m_sslrEnabled = false;

	// 1st pass: Do the indirect lighting computation
	ShaderResourcePtr m_frag;
	ShaderResourcePtr m_vert;
	PipelinePtr m_ppline;
	TexturePtr m_rt;
	FramebufferPtr m_fb;
	ResourceGroupPtr m_rcGroup;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
};
/// @}

} // end namespace anki
