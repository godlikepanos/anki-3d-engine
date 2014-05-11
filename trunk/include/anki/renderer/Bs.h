#ifndef ANKI_RENDERER_BS_H
#define ANKI_RENDERER_BS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gl.h"
#include "anki/resource/Resource.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Blending stage. The objects that blend must be handled differently
class Bs: public RenderingPass
{
public:
	Bs(Renderer* r)
		: RenderingPass(r) 
	{}

	~Bs();

	void init(const RendererInitializer& initializer);
	void run(GlJobChainHandle& jobs);
};

/// @}

} // end namespace anki

#endif
