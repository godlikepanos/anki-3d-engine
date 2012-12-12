#ifndef ANKI_RENDERER_SSAO_H
#define ANKI_RENDERER_SSAO_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/gl/Fbo.h"
#include "anki/gl/Texture.h"
#include "anki/gl/Ubo.h"
#include "anki/core/Timestamp.h"

namespace anki {

/// Screen space ambient occlusion pass
///
/// Three passes:
/// @li Calc ssao factor
/// @li Blur vertically
/// @li Blur horizontally
/// @li Repeat from 2
class Ssao: public SwitchableRenderingPass
{
public:
	Ssao(Renderer* r_)
		: SwitchableRenderingPass(r_)
	{}

	void init(const RendererInitializer& initializer);
	void run();

	/// @name Accessors
	/// @{
	const Texture& getFai() const
	{
		return vblurFai;
	}
	/// @}

private:
	Texture mpFai; ///< Main pass FAI. Not used if on non blit
	Texture vblurFai;
	Texture hblurFai;
	U32 blurringIterationsCount;
	Fbo mpFbo; ///< Main pass FBO. Not used if on non blit
	Fbo vblurFbo;
	Fbo hblurFbo;
	TextureResourcePointer noiseMap;
	ShaderProgramResourcePointer ssaoSProg;
	ShaderProgramResourcePointer hblurSProg;
	ShaderProgramResourcePointer vblurSProg;
	U32 bWidth, bHeight; ///< Blur passes size
	U32 mpWidth, mpHeight; ///< Main pass size
	U32 commonUboUpdateTimestamp = Timestamp::getTimestamp();
	Ubo commonUbo;

	static void createFbo(Fbo& fbo, Texture& fai, F32 width, F32 height);
	void initInternal(const RendererInitializer& initializer);

	/// This means that a bliting rendering will happen
	Bool blit() const
	{
		return mpWidth != bWidth;
	}
};

} // end namespace anki

#endif
