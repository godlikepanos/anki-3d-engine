#ifndef EZ_H
#define EZ_H

#include "RenderingPass.h"
#include "Fbo.h"


/// Material stage EarlyZ pass
class Ez: public RenderingPass
{
	public:
		Ez(Renderer& r_): RenderingPass(r_) {}
		bool isEnabled() const {return enabled;}
		void init(const RendererInitializer& initializer);
		void run();

	private:
		Fbo fbo; ///< Writes to MS depth FAI
		bool enabled;
};


#endif
