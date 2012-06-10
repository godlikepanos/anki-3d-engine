#ifndef ANKI_RENDERER_DBG_H
#define ANKI_RENDERER_DBG_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"

namespace anki {

/// Debugging stage
class Dbg: public SwitchableRenderingPass
{
public:
	Dbg(Renderer* r_)
		: SwitchableRenderingPass(r_)
	{}
	
	void init(const RendererInitializer& initializer);
	void run();

private:
	Fbo fbo;
};

} // end namespace

#endif
