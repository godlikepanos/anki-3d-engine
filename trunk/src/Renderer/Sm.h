#ifndef SM_H
#define SM_H

#include "RenderingStage.h"
#include "Fbo.h"
#include "Texture.h"


class Camera;


/**
 * Shadowmapping pass
 */
class Sm: private RenderingStage
{
	public:
		Texture shadowMap;

		Sm(Renderer& r_): RenderingStage(r_) {}

		void init(const RendererInitializer& initializer);

		/**
		 * Render the scene only with depth and store the result in the shadowMap
		 * @param cam The light camera
		 */
		void run(const Camera& cam);

		bool isEnabled() const {return enabled;}
		bool isPcfEnabled() const {return pcfEnabled;}
		bool isBilinearEnabled() const {return bilinearEnabled;}
		int getResolution() const {return resolution;}

	private:
		Fbo fbo; ///< Illumination stage shadowmapping FBO
		bool enabled; ///< If false then disable
		bool pcfEnabled; ///< Enable Percentage Closer Filtering
		bool bilinearEnabled; ///< Shadowmap bilinear filtering. Better quality
		int resolution; ///< Shadowmap resolution. The higher the better but slower
};


#endif
