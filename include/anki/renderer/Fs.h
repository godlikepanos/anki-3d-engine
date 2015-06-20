// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/renderer/RenderingPass.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Forward rendering stage. The objects that blend must be handled differently
class Fs: public RenderingPass
{
public:
	Fs(Renderer* r)
		: RenderingPass(r)
	{}

	~Fs();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	ANKI_USE_RESULT Error run(CommandBufferPtr& cmdb);

private:
	FramebufferPtr m_fb;
};
/// @}

} // end namespace anki

