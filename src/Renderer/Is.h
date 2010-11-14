#ifndef IS_H
#define IS_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Texture.h"
#include "RsrcPtr.h"
#include "ShaderProg.h"
#include "Math.h"
#include "Properties.h"


class PointLight;
class SpotLight;
class Vao;
class Sm;
class Smo;
class Vbo;


/// Illumination stage
class Is: private RenderingPass
{
	PROPERTY_R(Texture, fai, getFai) ///< The one and only FAI

	public:
		Is(Renderer& r_, Object* parent);
		void init(const RendererInitializer& initializer);
		void run();

	private:
		Sm* sm; ///< Shadowmapping pass
		Smo* smo; /// Stencil masking optimizations pass
		Fbo fbo; ///< This FBO writes to the Is::fai
		uint stencilRb; ///< Illumination stage stencil buffer
		RsrcPtr<ShaderProg> ambientPassSProg; ///< Illumination stage ambient pass shader program
		RsrcPtr<ShaderProg> pointLightSProg; ///< Illumination stage point light shader program
		RsrcPtr<ShaderProg> spotLightNoShadowSProg; ///< Illumination stage spot light w/o shadow shader program
		RsrcPtr<ShaderProg> spotLightShadowSProg; ///< Illumination stage spot light w/ shadow shader program

		/// @name For the quad drawing in light passes
		/// @{
		Vbo* quadPositionsVbo; ///< The VBO for quad positions
		Vbo* viewVectorsVbo; ///< The VBO to pass the @ref viewVectors.
		Vbo* quadVertIndecesVbo; ///< The VBO for quad array buffer elements
		Vao* vao; ///< This VAO is used in light passes only
		/// @}

		Vec2 planes; ///< Used to to calculate the frag pos in view space inside the shader program

		/// Draws the vao that has attached the viewVectorsVbo as well. Used in light passes
		void drawLightPassQuad() const;

		/// Calc the view vector that we will use inside the shader to calculate the frag pos in view space. This calculates
		/// the view vectors and updates the @ref viewVectorsVbo
		void calcViewVectors();

		/// Calc the planes that we will use inside the shader to calculate the frag pos in view space
		void calcPlanes();

		/// The ambient pass
		void ambientPass(const Vec3& color);

		/// The point light pass
		void pointLightPass(const PointLight& light);

		/// The spot light pass
		void spotLightPass(const SpotLight& light);

		/// Used in @ref init
		void initFbo();
};


#endif
