#ifndef BS_H
#define BS_H

#include "RenderingStage.h"
#include "Fbo.h"
#include "RsrcPtr.h"
#include "Texture.h"


class ShaderProg;


/// Blending stage.
/// The objects that blend must be handled differently
class Bs: public RenderingStage
{
	public:
		Bs(Renderer& r_): RenderingStage(r_) {}
		void init(const RendererInitializer& initializer);
		void run();

	private:
		Fbo fbo; ///< Writes to pps.prePassFai
		Fbo refractFbo; ///< Writes to refractFai
		RsrcPtr<ShaderProg> refractSProg;
		Texture refractFai;

		void createFbo();
		void createRefractFbo();
};


#endif
