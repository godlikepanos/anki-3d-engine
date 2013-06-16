#ifndef ANKI_RENDERER_LF_H
#define ANKI_RENDERER_LF_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Gl.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/TextureResource.h"

namespace anki {

/// Lens flare rendering pass
class Lf: public OptionalRenderingPass
{
public:
	Lf(Renderer* r_)
		: OptionalRenderingPass(r_)
	{}

	~Lf();

	void init(const RendererInitializer& initializer);
	void run();

private:
	ShaderProgramResourcePointer drawProg;
	Ubo flareDataUbo;

	U8 maxLensFlareCount;
	U8 maxLightsWidthFlaresCount;

	TextureResourcePointer tex;
};

} // end namespace anki

#endif
