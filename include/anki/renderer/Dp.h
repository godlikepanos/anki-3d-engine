// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_DP_H
#define ANKI_RENDERER_DP_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gr.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Depth buffer processing.
class Dp: public RenderingPass
{
public:
	/// @privatesection
	/// @{
	Dp(Renderer* r)
	:	RenderingPass(r)
	{}

	GlTextureHandle& getSmallDepthRt()
	{
		return m_smallDepthRt;
	}

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	ANKI_USE_RESULT Error run(GlCommandBufferHandle& cmdBuff);
	/// @}

private:
	GlTextureHandle m_smallDepthRt; ///< A smaller depth buffer
	GlFramebufferHandle m_smallDepthFb; ///< Used to blit
	UVec2 m_smallDepthSize;
};
/// @}

} // end namespace anki

#endif

