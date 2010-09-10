#ifndef RENDERING_STAGE_H
#define RENDERING_STAGE_H

#include "Common.h"


class Renderer;
class RendererInitializer;


/**
 * Rendering stage
 */
class RenderingStage
{
	public:
		RenderingStage(Renderer& r_): r(r_) {}
		virtual void init(const RendererInitializer& initializer) = 0;

	protected:
		Renderer& r; ///< Know your father
};


#endif
