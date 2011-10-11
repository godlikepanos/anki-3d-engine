#ifndef ANKI_RENDERER_BS_H
#define ANKI_RENDERER_BS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/RsrcPtr.h"
#include "anki/resource/Texture.h"


class ShaderProgram;


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
		RsrcPtr<ShaderProgram> refractSProg;
		Texture refractFai;

		void createFbo();
		void createRefractFbo();
};


#endif
