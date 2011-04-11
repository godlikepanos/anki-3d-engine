#ifndef EZ_H
#define EZ_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Accessors.h"


/// Material stage EarlyZ pass
class Ez: public RenderingPass
{
	public:
		Ez(Renderer& r_): RenderingPass(r_) {}

		GETTER_R_BY_VAL(bool, enabled, isEnabled)

		void init(const RendererInitializer& initializer);
		void run();

	private:
		Fbo fbo; ///< Writes to MS depth FAI
		bool enabled;
};


#endif
