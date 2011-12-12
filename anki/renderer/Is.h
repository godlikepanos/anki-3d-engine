#ifndef ANKI_RENDERER_IS_H
#define ANKI_RENDERER_IS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/Texture.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ShaderProgram.h"
#include "anki/math/Math.h"
#include "anki/gl/Vbo.h"
#include "anki/gl/Vao.h"
#include "anki/renderer/Sm.h"
#include "anki/renderer/Smo.h"


namespace anki {


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
	const Texture& getFai() const
	{
		return fai;
	}
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
	ShaderProgramResourcePointer ambientPassSProg;
	/// Illumination stage point light shader program
	ShaderProgramResourcePointer pointLightSProg;
	/// Illumination stage spot light w/o shadow shader program
	ShaderProgramResourcePointer spotLightNoShadowSProg;
	/// Illumination stage spot light w/ shadow shader program
	ShaderProgramResourcePointer spotLightShadowSProg;

	/// The ambient pass
	void ambientPass(const Vec3& color);

	/// The point light pass
	void pointLightPass(PointLight& plight);

	/// The spot light pass
	void spotLightPass(SpotLight& slight);

	/// Used in @ref init
	void initFbo();

	/// Init the copy stuff
	void initCopy();

	/// Copy the MS depth FAI to one of our own
	void copyDepth();
};


} // end namespace


#endif
