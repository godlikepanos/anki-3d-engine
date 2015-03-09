// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_BS_H
#define ANKI_RENDERER_BS_H

#include "anki/renderer/RenderingPass.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Forward rendering stage. The objects that blend must be handled differently
class Fs: public RenderingPass
{
public:
	Fs(Renderer* r)
	:	RenderingPass(r) 
	{}

	~Fs();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	ANKI_USE_RESULT Error run(GlCommandBufferHandle& cmdb);

private:
	GlFramebufferHandle m_fb;
};
/// @}

} // end namespace anki

#endif
