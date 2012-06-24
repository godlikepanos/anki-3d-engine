#ifndef ANKI_RENDERER_HDR_H
#define ANKI_RENDERER_HDR_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/Resource.h"

namespace anki {

class ShaderProgram;

/// High dynamic range lighting pass
class Hdr: public SwitchableRenderingPass
{
public:
	Hdr(Renderer* r_)
		: SwitchableRenderingPass(r_)
	{}

	~Hdr();

	void init(const RendererInitializer& initializer);
	void run();

	/// @name Accessors
	/// @{
	float getExposure() const
	{
		return exposure;
	}
	void setExposure(const float x)
	{
		exposure = x;
		toneSProg.bind();
		toneSProg.findUniformVariableByName("exposure")->set(exposure);
	}

	uint getBlurringIterationsNum() const
	{
		return blurringIterationsNum;
	}
	void setBlurringIterationsNum(const uint x)
	{
		blurringIterationsNum = x;
	}

	float getBlurringDistance() const
	{
		return blurringDist;
	}
	void setBlurringDistance(const float x)
	{
		blurringDist = x;

		hblurSProg.bind();
		hblurSProg.findUniformVariableByName("blurringDist")->set(
			float(blurringDist / width));
		vblurSProg.bind();
		vblurSProg.findUniformVariableByName("blurringDist")->set(
			float(blurringDist / height));
	}

	float getRenderingQuality() const
	{
		return renderingQuality;
	}

	const Texture& getToneFai() const
	{
		return toneFai;
	}

	const Texture& getHblurFai() const
	{
		return hblurFai;
	}

	const Texture& getFai() const
	{
		return fai;
	}
	/// @}

private:
	uint32_t width;
	uint32_t height;
	float exposure = 4.0; ///< How bright is the HDR
	uint32_t blurringIterationsNum = 2; ///< The blurring iterations of the tone map
	float blurringDist = 1.0; ///< Distance in blurring
	float renderingQuality = 0.5;
	Fbo toneFbo;
	Fbo hblurFbo;
	Fbo vblurFbo;
	ShaderProgramResource toneSProg;
	ShaderProgramResource hblurSProg;
	ShaderProgramResource vblurSProg;
	Texture toneFai; ///< Vertical blur pass FAI
	Texture hblurFai; ///< pass0Fai with the horizontal blur FAI
	Texture fai; ///< The final FAI

	void initFbo(Fbo& fbo, Texture& fai);
};

} // end namespace

#endif
