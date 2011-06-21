#ifndef MS_H
#define MS_H

#include "RenderingPass.h"
#include "Texture.h"
#include "Fbo.h"
#include "Accessors.h"
#include "Ez.h"


/// Material stage
class Ms: public RenderingPass
{
	public:
		Ms(Renderer& r_);

		/// @name Accessors
		/// @{
		GETTER_R(Texture, normalFai, getNormalFai)
		GETTER_R(Texture, diffuseFai, getDiffuseFai)
		GETTER_R(Texture, specularFai, getSpecularFai)
		GETTER_R(Texture, depthFai, getDepthFai)
		/// @}

		void init(const RendererInitializer& initializer);
		void run();

	private:
		Ez ez; /// EarlyZ pass
		Fbo fbo;
		Texture normalFai; ///< The FAI for normals
		Texture diffuseFai; ///< The FAI for diffuse color
		Texture specularFai; ///< The FAI for specular color and shininess
		Texture depthFai; ///< The FAI for depth
};


#endif
