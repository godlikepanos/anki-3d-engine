#ifndef ANKI_RENDERER_EZ_H
#define ANKI_RENDERER_EZ_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"

namespace anki {

/// Material stage EarlyZ pass
class Ez: public OptionalRenderingPass
{
public:
	Ez(Renderer* r)
		: OptionalRenderingPass(r)
	{}

	void init(const RendererInitializer& initializer);
	void run();

private:
	U maxObjectsToDraw;
};

} // end namespace anki


#endif
