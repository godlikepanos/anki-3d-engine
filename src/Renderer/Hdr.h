#ifndef HDR_H
#define HDR_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Texture.h"
#include "RsrcPtr.h"
#include "ShaderProg.h"


/// High dynamic range lighting pass
class Hdr: private RenderingPass
{
	public:
		Texture toneFai; ///< Vertical blur pass FAI
		Texture hblurFai; ///< pass0Fai with the horizontal blur FAI
		Texture fai; ///< The final FAI

		Hdr(Renderer& r_, Object* parent): RenderingPass(r_, parent) {}
		void init(const RendererInitializer& initializer);
		void run();

		/// Setters & getters
		/// @{
		float getBlurringDist() {return blurringDist;}
		void setBlurringDist(float f) {blurringDist = f;}
		uint getBlurringIterations() {return blurringIterations;}
		void setBlurringIterations(uint i) {blurringIterations = i;}
		float getExposure() const {return exposure;}
		void setExposure(float f) {exposure = f;}
		bool isEnabled() const {return enabled;}
		float getRenderingQuality() const {return renderingQuality;}
		/// @}

	private:
		Fbo toneFbo;
		Fbo hblurFbo;
		Fbo vblurFbo;
		RsrcPtr<ShaderProg> toneSProg;
		RsrcPtr<ShaderProg> hblurSProg;
		RsrcPtr<ShaderProg> vblurSProg;
		float blurringDist;
		uint blurringIterations;
		float exposure; ///< @todo
		bool enabled;
		float renderingQuality;

		void initFbo(Fbo& fbo, Texture& fai);
};


#endif
