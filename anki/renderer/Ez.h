#ifndef EZ_H
#define EZ_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"


/// Material stage EarlyZ pass
class Ez: public SwitchableRenderingPass
{
	public:
		Ez(Renderer& r_): SwitchableRenderingPass(r_) {}

		void init(const RendererInitializer& initializer);
		void run();

	private:
		Fbo fbo; ///< Writes to MS depth FAI
};


#endif
