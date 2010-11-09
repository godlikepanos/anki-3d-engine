#ifndef BS_H
#define BS_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "RsrcPtr.h"
#include "Texture.h"


class ShaderProg;


/// Blending stage.
/// The objects that blend must be handled differently
class Bs: public RenderingPass
{
	public:
		Bs(Renderer& r_, Object* parent): RenderingPass(r_, parent) {}
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
