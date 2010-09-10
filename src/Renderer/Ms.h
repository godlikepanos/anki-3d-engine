#ifndef MS_H
#define MS_H

#include "Common.h"
#include "RenderingStage.h"
#include "Ez.h"
#include "Texture.h"


class RendererInitializer;


/**
 * Material stage
 */
class Ms: public RenderingStage
{
	public:
		Ez ez;
		Texture normalFai;
		Texture diffuseFai;
		Texture specularFai;
		Texture depthFai;

		Ms(Renderer& r_): RenderingStage(r_), ez(r_) {}
		void init(const RendererInitializer& initializer);
		void run();

	private:
		Fbo fbo;
};

#endif
