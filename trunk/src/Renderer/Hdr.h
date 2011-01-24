#ifndef HDR_H
#define HDR_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Texture.h"
#include "RsrcPtr.h"
#include "Properties.h"


class ShaderProg;


/// High dynamic range lighting pass
class Hdr: private RenderingPass
{
	PROPERTY_R(bool, enabled, isEnabled)
	PROPERTY_R(Texture, toneFai, getToneFai) ///< Vertical blur pass FAI
	PROPERTY_R(Texture, hblurFai, getHblurFai) ///< pass0Fai with the horizontal blur FAI
	PROPERTY_R(Texture, fai, getFai) ///< The final FAI
	PROPERTY_R(float, renderingQuality, getRenderingQuality)
	/// The blurring iterations of the tone map
	PROPERTY_RW(uint, blurringIterationsNum, getBlurringIterationsNum, setBlurringIterationsNum)
	//PROPERTY_RW(float, exposure, getExposure, setExposure)///< How bright is the HDR
	PROPERTY_RW(float, blurringDist, getBlurringDist, setBlurringDist)

	public:
		Hdr(Renderer& r_): RenderingPass(r_) {}
		void init(const RendererInitializer& initializer);
		void run();
		GETTER_SETTER_BY_VAL(float, exposure, getExposure, setExposure)

	private:
		float exposure; ///< How bright is the HDR
		Fbo toneFbo;
		Fbo hblurFbo;
		Fbo vblurFbo;
		RsrcPtr<ShaderProg> toneSProg;
		RsrcPtr<ShaderProg> hblurSProg;
		RsrcPtr<ShaderProg> vblurSProg;

		void initFbo(Fbo& fbo, Texture& fai);
};


#endif
