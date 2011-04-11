#ifndef SM_H
#define SM_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Texture.h"
#include "Accessors.h"
#include "VisibilityTester.h"


class Camera;


/// Shadowmapping pass
class Sm: private RenderingPass
{
	public:
		Sm(Renderer& r_): RenderingPass(r_) {}

		/// @name Accessors
		/// @{
		GETTER_R(Texture, shadowMap, getShadowMap)
		GETTER_R_BY_VAL(bool, enabled, isEnabled)
		GETTER_R_BY_VAL(bool, pcfEnabled, isPcfEnabled)
		GETTER_R_BY_VAL(bool, bilinearEnabled, isBilinearEnabled)
		GETTER_R_BY_VAL(int, resolution, getResolution)
		/// @}

		void init(const RendererInitializer& initializer);

		/// Render the scene only with depth and store the result in the shadowMap
		/// @param[in] cam The light camera
		void run(const Camera& cam);

	private:
		Fbo fbo; ///< Illumination stage shadowmapping FBO
		Texture shadowMap;
		bool enabled; ///< If false then disable
		bool pcfEnabled; ///< Enable Percentage Closer Filtering
		bool bilinearEnabled; ///< Shadowmap bilinear filtering. Better quality
		int resolution; ///< Shadowmap resolution. The higher the better but slower
};


#endif
