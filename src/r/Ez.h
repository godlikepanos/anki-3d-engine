#ifndef R_EZ_H
#define R_EZ_H

#include "RenderingPass.h"
#include "gl/Fbo.h"
#include "util/Accessors.h"


namespace r {


/// Material stage EarlyZ pass
class Ez: public RenderingPass
{
	public:
		Ez(Renderer& r_): RenderingPass(r_) {}

		GETTER_R_BY_VAL(bool, enabled, isEnabled)

		void init(const RendererInitializer& initializer);
		void run();

	private:
		gl::Fbo fbo; ///< Writes to MS depth FAI
		bool enabled;
};


} // end namespace


#endif
