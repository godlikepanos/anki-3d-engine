// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void run(RenderingContext& ctx);

private:
	class Subpass
	{
	public:
		ShaderResourcePtr m_vert;
		ShaderResourcePtr m_frag;
		PipelinePtr m_ppline;
		ResourceGroupPtr m_rcGroup;
		FramebufferPtr m_fb;
	};

	Array<Subpass, IS_MIPMAP_COUNT - 1> m_passes;

	Error initSubpass(U idx, const UVec2& inputTexSize);
};
/// @}

} // end namespace anki
