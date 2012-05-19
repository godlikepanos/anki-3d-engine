#ifndef ANKI_RENDERER_IS_H
#define ANKI_RENDERER_IS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ShaderProgramResource.h"
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
	Texture fai;
};


} // end namespace


#endif
