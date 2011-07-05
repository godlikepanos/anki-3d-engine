#ifndef BS_H
#define BS_H

#include "RenderingPass.h"
#include "GfxApi/BufferObjects/Fbo.h"
#include "Resources/RsrcPtr.h"
#include "Resources/Texture.h"


class ShaderProg;


/// Blending stage.
/// The objects that blend must be handled differently
class Bs: public RenderingPass
{
	public:
		Bs(Renderer& r_): RenderingPass(r_) {}
		~Bs();

		void init(const RendererInitializer& initializer);
		void run();

	private:
		Fbo fbo; ///< Writes to Pps::prePassFai
		Fbo refractFbo; ///< Writes to refractFai
		RsrcPtr<ShaderProg> refractSProg;
		Texture refractFai;

		void createFbo();
		void createRefractFbo();
};


#endif
