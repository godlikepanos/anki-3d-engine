#ifndef HDR_H
#define HDR_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Texture.h"
#include "RsrcPtr.h"
#include "Accessors.h"


class ShaderProg;


/// High dynamic range lighting pass
class Hdr: private RenderingPass
{
	public:
		Hdr(Renderer& r_): RenderingPass(r_) {}
		void init(const RendererInitializer& initializer);
		void run();

		/// @name Accessors
		/// @{
		GETTER_SETTER_BY_VAL(float, exposure, getExposure, setExposure)
		GETTER_SETTER_BY_VAL(uint, blurringIterationsNum, getBlurringIterationsNum, setBlurringIterationsNum)
		GETTER_SETTER_BY_VAL(float, blurringDist, getBlurringDist, setBlurringDist)

		GETTER_R_BY_VAL(bool, enabled, isEnabled);
		GETTER_R_BY_VAL(float, renderingQuality, getRenderingQuality)
		GETTER_R(Texture, toneFai, getToneFai)
		GETTER_R(Texture, hblurFai, getHblurFai)
		GETTER_R(Texture, fai, getFai)
		/// @}

	private:
		bool enabled;
		float exposure; ///< How bright is the HDR
		uint blurringIterationsNum; ///< The blurring iterations of the tone map
		float blurringDist; ///< Distance in blurring
		float renderingQuality;
		Fbo toneFbo;
		Fbo hblurFbo;
		Fbo vblurFbo;
		RsrcPtr<ShaderProg> toneSProg;
		RsrcPtr<ShaderProg> hblurSProg;
		RsrcPtr<ShaderProg> vblurSProg;
		Texture toneFai; ///< Vertical blur pass FAI
		Texture hblurFai; ///< pass0Fai with the horizontal blur FAI
		Texture fai; ///< The final FAI

		void initFbo(Fbo& fbo, Texture& fai);
};


#endif
