#ifndef IS_H
#define IS_H

#include "Common.h"
#include "RenderingStage.h"
#include "Fbo.h"
#include "Sm.h"
#include "Smo.h"
#include "Texture.h"


class PointLight;
class SpotLight;


/**
 * Illumination stage
 */
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

		/**
		 * @name Ptrs to uniform variables
		 */
		/**@{*/
		const ShaderProg::UniVar* ambientColUniVar;
		const ShaderProg::UniVar* sceneColMapUniVar;
		/**@}*/

		Vec3 viewVectors[4];
		Vec2 planes;

		/**
		 * Calc the view vector that we will use inside the shader to calculate the frag pos in view space
		 */
		void calcViewVectors();

		/**
		 * Calc the planes that we will use inside the shader to calculate the frag pos in view space
		 */
		void calcPlanes();
		void ambientPass(const Vec3& color);
		void pointLightPass(const PointLight& light);
		void spotLightPass(const SpotLight& light);
		void initFbo();
};


#endif
