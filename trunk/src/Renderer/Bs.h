#ifndef BS_H
#define BS_H

#include "Common.h"
#include "RenderingStage.h"
#include "Fbo.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"
#include "Texture.h"


/**
 * Blending stage
 */
class Bs: public RenderingStage
{
	public:
		Bs(Renderer& r_): RenderingStage(r_) {}
		void init(const RendererInitializer& initializer);
		void run();

	private:
		Fbo fbo;
		Fbo refractFbo;
		RsrcPtr<ShaderProg> refractSProg;
		Texture refractFai;

		void createFbo();
		void createRefractFbo();
};


#endif
