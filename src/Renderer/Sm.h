#ifndef SM_H
#define SM_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Texture.h"
#include "Properties.h"
#include "VisibilityTester.h"


class Camera;


/// Shadowmapping pass
class Sm: private RenderingPass
{
	PROPERTY_R(bool, enabled, isEnabled) ///< If false then disable
	PROPERTY_R(bool, pcfEnabled, isPcfEnabled) ///< Enable Percentage Closer Filtering
	PROPERTY_R(bool, bilinearEnabled, isBilinearEnabled) ///< Shadowmap bilinear filtering. Better quality
	PROPERTY_R(int, resolution, getResolution) ///< Shadowmap resolution. The higher the better but slower

	public:
		Texture shadowMap;

		Sm(Renderer& r_): RenderingPass(r_) {}

		void init(const RendererInitializer& initializer);

		/// Render the scene only with depth and store the result in the shadowMap
		/// @param[in] cam The light camera
		void run(const Camera& cam);

	private:
		Fbo fbo; ///< Illumination stage shadowmapping FBO
};


#endif
