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
	friend class MainRenderer;

public:
	Lf(Renderer* r_)
		: OptionalRenderingPass(r_)
	{}

	~Lf();

	void init(const RendererInitializer& initializer);
	void run();

	const Texture& getFai() const
	{
		return fai;
	}

private:
	/// Pseudo lens flare program
	ShaderProgramResourcePointer pseudoProg;
	/// Real lens flare program
	ShaderProgramResourcePointer realProg;
	Texture fai;
	Fbo fbo;
	TextureResourcePointer lensDirtTex;
	U8 maxFlaresPerLight;
	U8 maxLightsWithFlares;
	GlBuffer flareDataUbo;
	const ShaderProgramUniformBlock* ublock;
	

	void initInternal(const RendererInitializer& initializer);
};

} // end namespace anki

#endif
