#ifndef IS_H
#define IS_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Texture.h"
#include "RsrcPtr.h"
#include "ShaderProg.h"
#include "Math.h"
#include "Vbo.h"
#include "Vao.h"
#include "Sm.h"
#include "Smo.h"


class PointLight;
class SpotLight;


/// Illumination stage
class Is: private RenderingPass
{
	public:
		Is(Renderer& r_);
		void init(const RendererInitializer& initializer);
		void run();

		/// @name Accessors
		/// @{
		const Texture& getFai() const {return fai;}
		/// @}

	private:
		Sm sm; ///< Shadowmapping pass
		Smo smo; /// Stencil masking optimizations pass
		Fbo fbo; ///< This FBO writes to the Is::fai
		Texture fai; ///< The one and only FAI
		uint stencilRb; ///< Illumination stage stencil buffer
		RsrcPtr<ShaderProg> ambientPassSProg; ///< Illumination stage ambient pass shader program
		RsrcPtr<ShaderProg> pointLightSProg; ///< Illumination stage point light shader program
		RsrcPtr<ShaderProg> spotLightNoShadowSProg; ///< Illumination stage spot light w/o shadow shader program
		RsrcPtr<ShaderProg> spotLightShadowSProg; ///< Illumination stage spot light w/ shadow shader program

		/// The ambient pass
		void ambientPass(const Vec3& color);

		/// The point light pass
		void pointLightPass(const PointLight& plight);

		/// The spot light pass
		void spotLightPass(const SpotLight& slight);

		/// Used in @ref init
		void initFbo();
};


#endif
