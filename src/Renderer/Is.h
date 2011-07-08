#ifndef R_IS_H
#define R_IS_H

#include "RenderingPass.h"
#include "GfxApi/BufferObjects/Fbo.h"
#include "Resources/Texture.h"
#include "Resources/RsrcPtr.h"
#include "Resources/ShaderProg.h"
#include "Math/Math.h"
#include "GfxApi/BufferObjects/Vbo.h"
#include "GfxApi/BufferObjects/Vao.h"
#include "Sm.h"
#include "Smo.h"


class PointLight;
class SpotLight;


namespace R {


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
		Texture copyMsDepthFai;
		Fbo readFbo;
		Fbo writeFbo;
		/// Illumination stage ambient pass shader program
		RsrcPtr<ShaderProg> ambientPassSProg;
		/// Illumination stage point light shader program
		RsrcPtr<ShaderProg> pointLightSProg;
		/// Illumination stage spot light w/o shadow shader program
		RsrcPtr<ShaderProg> spotLightNoShadowSProg;
		/// Illumination stage spot light w/ shadow shader program
		RsrcPtr<ShaderProg> spotLightShadowSProg;

		/// The ambient pass
		void ambientPass(const Vec3& color);

		/// The point light pass
		void pointLightPass(const PointLight& plight);

		/// The spot light pass
		void spotLightPass(const SpotLight& slight);

		/// Used in @ref init
		void initFbo();

		/// Init the copy stuff
		void initCopy();

		/// Copy the MS depth FAI to one of our own
		void copyDepth();
};


} // end namespace


#endif
