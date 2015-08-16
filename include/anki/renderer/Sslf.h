// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/renderer/RenderingPass.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Screen space lens flare pass.
class Sslf: public RenderingPass
{
anki_internal:
	Sslf(Renderer* r)
		: RenderingPass(r)
	{}

	ANKI_USE_RESULT Error init(const ConfigSet& config);
	void run(CommandBufferPtr& cmdb);

	TexturePtr& getRt()
	{
		return m_rt;
	}

private:
	TexturePtr m_rt;
	FramebufferPtr m_fb;
	ShaderResourcePtr m_frag;
	PipelinePtr m_ppline;
	TextureResourcePtr m_lensDirtTex;
	ResourceGroupPtr m_rcGroup;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
};
/// @}

} // end namespace anki

