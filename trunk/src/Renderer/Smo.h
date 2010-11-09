#ifndef SMO_H
#define SMO_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Vbo.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"


class PointLight;
class SpotLight;


/// Stencil masking optimizations
class Smo: public RenderingPass
{
	public:
		Smo(Renderer& r_, Object* parent): RenderingPass(r_, parent) {}
		void init(const RendererInitializer& initializer);
		void run(const PointLight& light);
		void run(const SpotLight& light);

	private:
		static float sMOUvSCoords[]; ///< Illumination stage stencil masking optimizations UV sphere vertex positions
		static Vbo* sMOUvSVbo; ///< Illumination stage stencil masking optimizations UV sphere VBO
		RsrcPtr<ShaderProg> sProg;
};


#endif
