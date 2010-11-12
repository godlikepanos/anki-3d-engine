#ifndef MS_H
#define MS_H

#include "RenderingPass.h"
#include "Texture.h"
#include "Fbo.h"
#include "Properties.h"


class RendererInitializer;
class Ez;


/// Material stage
class Ms: public RenderingPass
{
	PROPERTY_R(Texture, normalFai, getNormalFai) ///< The FAI for normals
	PROPERTY_R(Texture, diffuseFai, getDiffuseFai) ///< The FAI for diffuse color
	PROPERTY_R(Texture, specularFai, getSpecularFai) ///< The FAI for specular color and shininess
	PROPERTY_R(Texture, depthFai, getDepthFai) ///< The FAI for depth

	public:
		Ms(Renderer& r_, Object* parent);
		void init(const RendererInitializer& initializer);
		void run();

	private:
		Ez* ez;
		Fbo fbo;
};


#endif
