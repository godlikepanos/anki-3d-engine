// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderResource.h>

namespace anki
{

/// @addtogroup renderer
/// @{

// Quick pass to downscale the depth buffer.
class HalfDepth : public RenderingPass
{
anki_internal:
	TexturePtr m_depthRt;

	HalfDepth(Renderer* r)
		: RenderingPass(r)
	{
	}

	~HalfDepth();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void run(RenderingContext& ctx);

private:
	ShaderResourcePtr m_frag;
	PipelinePtr m_ppline;
	ResourceGroupPtr m_rcgroup;
	FramebufferPtr m_fb;
};
/// @}

} // end namespace
