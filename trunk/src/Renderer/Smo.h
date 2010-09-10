#ifndef SMO_H
#define SMO_H

#include "Common.h"
#include "RenderingStage.h"
#include "Fbo.h"
#include "Vbo.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"


class PointLight;
class SpotLight;


/**
 * Stencil masking optimizations
 */
class Smo: public RenderingStage
{
	public:
		Smo(Renderer& r_): RenderingStage(r_) {}
		void init(const RendererInitializer& initializer);
		void run(const PointLight& light);
		void run(const SpotLight& light);

	private:
		static float sMOUvSCoords[]; ///< Illumination stage stencil masking optimizations UV sphere vertex positions
		static Vbo sMOUvSVbo; ///< Illumination stage stencil masking optimizations UV sphere VBO
		RsrcPtr<ShaderProg> sProg;
		const ShaderProg::UniVar* modelViewProjectionMatUniVar; ///< Opt
};


#endif
