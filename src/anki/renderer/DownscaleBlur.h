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

	void run(RenderingContext& ctx);

private:
	class Subpass
	{
	public:
		ShaderResourcePtr m_frag;
		ShaderProgramPtr m_prog;
		FramebufferPtr m_fb;
	};

	DynamicArray<Subpass> m_passes;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initSubpass(U idx, const UVec2& inputTexSize);
};
/// @}

} // end namespace anki
