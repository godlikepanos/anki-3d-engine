#ifndef SMO_H
#define SMO_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"
#include "Vbo.h"
#include "Vao.h"


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
		/// @name UV sphere stuff
		/// @{
		static float sMOUvSCoords[]; ///< Illumination stage stencil masking optimizations UV sphere vertex positions
		Vbo spherePositionsVbo; ///< Illumination stage stencil masking optimizations UV sphere VBO
		Vao sphereVao; ///< And a VAO
		/// @}

		/// @name Camera shape stuff
		/// @{
		Vbo cameraPositionsVbo; ///< A camera shape
		Vbo cameraVertIndecesVbo; ///< The vertex indeces
		Vao cameraVao; ///< And another VAO
		/// @}

		RsrcPtr<ShaderProg> sProg;
};


#endif
