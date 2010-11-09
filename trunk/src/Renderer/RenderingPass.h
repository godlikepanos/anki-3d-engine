#ifndef RENDERING_PASS_H
#define RENDERING_PASS_H

#include "Object.h"


class Renderer;
class RendererInitializer;


/// Rendering pass
class RenderingPass: public Object
{
	public:
		RenderingPass(Renderer& r_, Object* parent = NULL);

		/// All passes should have an init
		virtual void init(const RendererInitializer& initializer) = 0;

	protected:
		Renderer& r; ///< Know your father
};


inline RenderingPass::RenderingPass(Renderer& r_, Object* parent):
	Object(parent),
	r(r_)
{}


#endif
