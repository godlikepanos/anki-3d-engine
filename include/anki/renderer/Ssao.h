#ifndef ANKI_RENDERER_SSAO_H
#define ANKI_RENDERER_SSAO_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/gl/Fbo.h"
#include "anki/gl/Texture.h"

namespace anki {

/// Screen space ambient occlusion pass
///
/// Three passes:
/// @li Calc ssao factor
/// @li Blur vertically
/// @li Blur horizontally repeat 2, 3
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
	float getRenderingQuality() const
	{
		return renderingQuality;
	}

	const Texture& getFai() const
	{
		return fai;
	}
	/// @}

private:
	Texture ssaoFai; ///< It contains the unblurred SSAO factor
	Texture hblurFai;
	Texture fai;  ///< AKA vblurFai The final FAI
	float renderingQuality;
	uint32_t blurringIterationsNum;
	Fbo ssaoFbo;
	Fbo hblurFbo;
	Fbo vblurFbo;
	TextureResourcePointer noiseMap;
	ShaderProgramResource ssaoSProg;
	ShaderProgramResource hblurSProg;
	ShaderProgramResource vblurSProg;
	uint32_t width, height;

	void createFbo(Fbo& fbo, Texture& fai);
};

} // end namespace anki

#endif
