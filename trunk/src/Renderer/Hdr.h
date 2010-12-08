#ifndef HDR_H
#define HDR_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Texture.h"
#include "RsrcPtr.h"
#include "ShaderProg.h"
#include "Properties.h"


/// High dynamic range lighting pass
class Hdr: private RenderingPass
{
	PROPERTY_R(Texture, toneFai, getToneFai) ///< Vertical blur pass FAI
	PROPERTY_R(Texture, hblurFai, getHblurFai) ///< pass0Fai with the horizontal blur FAI
	PROPERTY_R(Texture, fai, getFai) ///< The final FAI
	PROPERTY_R(float, renderingQuality, getRenderingQuality)
	/// The blurring iterations of the tone map
	PROPERTY_RW(uint, blurringIterationsNum, getBlurringIterationsNum, setBlurringIterationsNum)
	PROPERTY_RW(float, exposure, getExposure, setExposure)///< How bright is the HDR
	PROPERTY_RW(float, blurringDist, getBlurringDist, setBlurringDist)

	public:
		Hdr(Renderer& r_, Object* parent): RenderingPass(r_, parent) {}
		void init(const RendererInitializer& initializer);
		void run();

		/// Setters & getters
		/// @{
		bool isEnabled() const {return enabled;}
		/// @}

	private:
		Fbo toneFbo;
		Fbo hblurFbo;
		Fbo vblurFbo;
		RsrcPtr<ShaderProg> toneSProg;
		RsrcPtr<ShaderProg> hblurSProg;
		RsrcPtr<ShaderProg> vblurSProg;
		bool enabled;

		void initFbo(Fbo& fbo, Texture& fai);
};


#endif
