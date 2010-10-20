#ifndef EZ_H
#define EZ_H

#include "RenderingStage.h"
#include "Fbo.h"


/**
 * Material stage EarlyZ pass
 */
class Ez: public RenderingStage
{
	public:
		Ez(Renderer& r_): RenderingStage(r_) {}
		bool isEnabled() const {return enabled;}
		void init(const RendererInitializer& initializer);
		void run();

	private:
		Fbo fbo; ///< Writes to MS depth FAI
		bool enabled;
};


#endif
