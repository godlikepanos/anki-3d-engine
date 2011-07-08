#ifndef R_SMO_H
#define R_SMO_H

#include "RenderingPass.h"
#include "GfxApi/BufferObjects/Fbo.h"
#include "Resources/ShaderProg.h"
#include "Resources/RsrcPtr.h"
#include "GfxApi/BufferObjects/Vbo.h"
#include "GfxApi/BufferObjects/Vao.h"
#include "Scene/Camera.h"


class PointLight;
class SpotLight;


namespace R {


/// Stencil masking optimizations
class Smo: public RenderingPass
{
	public:
		Smo(Renderer& r_): RenderingPass(r_) {}
		~Smo();
		void init(const RendererInitializer& initializer);
		void run(const PointLight& light);
		void run(const SpotLight& light);

	private:
		/// @todo
		struct Geom
		{
			Geom() {}
			~Geom();

			RsrcPtr<Mesh> mesh;
			Vao vao;
		};

		Geom sphereGeom;

		/// An array of geometry stuff. For perspective cameras the shape is a
		/// pyramid, see the blend file with the vertex positions
		boost::array<Geom, Camera::CT_NUM> camGeom;

		RsrcPtr<ShaderProg> sProg;

		void initCamGeom();

		void setUpGl(bool inside);
		void restoreGl(bool inside);
};


} // end namespace


#endif
