#ifndef IS_H
#define IS_H

#include "RenderingStage.h"
#include "Fbo.h"
#include "Sm.h"
#include "Smo.h"
#include "Texture.h"


class PointLight;
class SpotLight;


/// Illumination stage
class Is: private RenderingStage
{
	public:
		Sm sm;
		Smo smo;
		Texture fai;

		Is(Renderer& r_): RenderingStage(r_), sm(r_), smo(r_) {}
		void init(const RendererInitializer& initializer);
		void run();

	private:
		Fbo fbo; ///< This FBO writes to the Is::fai
		uint stencilRb; ///< Illumination stage stencil buffer
		RsrcPtr<ShaderProg> ambientPassSProg; ///< Illumination stage ambient pass shader program
		RsrcPtr<ShaderProg> pointLightSProg; ///< Illumination stage point light shader program
		RsrcPtr<ShaderProg> spotLightNoShadowSProg; ///< Illumination stage spot light w/o shadow shader program
		RsrcPtr<ShaderProg> spotLightShadowSProg; ///< Illumination stage spot light w/ shadow shader program

		/// @name Ptrs to uniform variables
		/// @{
		const ShaderProg::UniVar* ambientColUniVar;
		const ShaderProg::UniVar* sceneColMapUniVar;
		/// @}

		Vec3 viewVectors[4];
		Vec2 planes;

		/// Calc the view vector that we will use inside the shader to calculate the frag pos in view space
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
