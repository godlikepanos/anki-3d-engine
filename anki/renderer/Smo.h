#ifndef ANKI_RENDERER_SMO_H
#define ANKI_RENDERER_SMO_H

#include "anki/renderer/RenderingPass.h"


namespace anki {


class Light;


/// Stencil masking optimizations
class Smo: public RenderingPass
{
public:
	Smo(Renderer* r_)
		: RenderingPass(r_)
	{}

	~Smo();

	void init(const RendererInitializer& initializer);
	void run(const Light& light);

private:
};


} // end namespace


#endif
