// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_EZ_H
#define ANKI_RENDERER_EZ_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gl.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Material stage EarlyZ pass
class Ez: public OptionalRenderingPass
{
	friend class Ms;

private:
	U32 m_maxObjectsToDraw;

	Ez(Renderer* r)
		: OptionalRenderingPass(r)
	{}

	void init(const RendererInitializer& initializer);
	void run(GlJobChainHandle& jobs);
};

/// @}

} // end namespace anki


#endif
