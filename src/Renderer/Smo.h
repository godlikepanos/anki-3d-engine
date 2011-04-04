#ifndef SMO_H
#define SMO_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"
#include "Vbo.h"
#include "Vao.h"
#include "Camera.h"


class PointLight;
class SpotLight;


/// Stencil masking optimizations
class Smo: public RenderingPass
{
	public:
		Smo(Renderer& r_): RenderingPass(r_) {}
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

		struct CameraGeom
		{
			Vbo positionsVbo; ///< A camera shape
			Vbo vertIndecesVbo; ///< The vertex indeces
			Vao vao; ///< And another VAO
		};

		boost::array<CameraGeom, Camera::CT_NUM> camGeom;
		/// @}

		RsrcPtr<ShaderProg> sProg;

		void initCamGeom();
};


#endif
