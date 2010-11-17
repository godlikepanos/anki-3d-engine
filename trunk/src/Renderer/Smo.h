#ifndef SMO_H
#define SMO_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"


class PointLight;
class SpotLight;
class Vao;
class Vbo;


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
		Vbo* sphereVbo; ///< Illumination stage stencil masking optimizations UV sphere VBO
		Vao* sphereVao; ///< And a VAO
		Vbo* cameraVbo; ///< A camera shape
		Vao* cameraVao; ///< And another VAO
		RsrcPtr<ShaderProg> sProg;
};


#endif
