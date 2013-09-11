#ifndef ANKI_RENDERER_PPS_H
#define ANKI_RENDERER_PPS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Ssao.h"
#include "anki/renderer/Bl.h"
#include "anki/renderer/Lf.h"

namespace anki {

class ShaderProgram;

/// Post-processing stage.This stage is divided into 2 two parts. The first
/// happens before blending stage and the second after
class Pps: public OptionalRenderingPass
{
public:
	Pps(Renderer* r);
	~Pps();

	void init(const RendererInitializer& initializer);
	void run();

	/// @name Accessors
	/// @{
	const Hdr& getHdr() const
	{
		return hdr;
	}
	Hdr& getHdr()
	{
		return hdr;
	}

	const Ssao& getSsao() const
	{
		return ssao;
	}

	const Bl& getBl() const
	{
		return bl;
	}
	Bl& getBl()
	{
		return bl;
	}

	const Lf& getLf() const
	{
		return lf;
	}
	Lf& getLf()
	{
		return lf;
	}

	const Texture& getFai() const
	{
		return fai;
	}
	Texture& getFai()
	{
		return fai;
	}
	/// @}

private:
	/// @name Passes
	/// @{
	Hdr hdr;
	Ssao ssao;
	Bl bl;
	Lf lf;
	/// @}

	Fbo fbo;
	ShaderProgramResourcePointer prog;
	Texture fai;

	void initInternal(const RendererInitializer& initializer);
};

} // end namespace anki

#endif
