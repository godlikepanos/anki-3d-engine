#ifndef MS_H
#define MS_H

#include "RenderingPass.h"
#include "Texture.h"
#include "Fbo.h"


class RendererInitializer;
class Ez;


/// Material stage
class Ms: public RenderingPass
{
	public:
		Texture normalFai;
		Texture diffuseFai;
		Texture specularFai;
		Texture depthFai;

		Ms(Renderer& r_, Object* parent);
		void init(const RendererInitializer& initializer);
		void run();

	private:
		Ez* ez;
		Fbo fbo;
};

#endif
