#ifndef HDR_H
#define HDR_H

#include "Common.h"
#include "RenderingStage.h"
#include "Fbo.h"
#include "Texture.h"
#include "RsrcPtr.h"
#include "ShaderProg.h"


/**
 * High dynamic range lighting pass
 */
class Hdr: private RenderingStage
{
	public:
		Texture toneFai; ///< Vertical blur pass FAI
		Texture hblurFai; ///< pass0Fai with the horizontal blur FAI
		Texture fai; ///< The final FAI

		Hdr(Renderer& r_): RenderingStage(r_) {}
		void init(const RendererInitializer& initializer);
		void run();

		/**
		 * Setters & getters
		 */
		/**@{*/
		float getBlurringDist() {return blurringDist;}
		void setBlurringDist(float f) {blurringDist = f;}
		uint getBlurringIterations() {return blurringIterations;}
		void setBlurringIterations(uint i) {blurringIterations = i;}
		bool isEnabled() const {return enabled;}
		float getRenderingQuality() const {return renderingQuality;}
		/**@}*/

	private:
		Fbo toneFbo;
		Fbo hblurFbo;
		Fbo vblurFbo;
		RsrcPtr<ShaderProg> toneSProg;
		RsrcPtr<ShaderProg> hblurSProg;
		RsrcPtr<ShaderProg> vblurSProg;
		const ShaderProg::UniVar* toneProgFaiUniVar;
		const ShaderProg::UniVar* hblurSProgFaiUniVar;
		const ShaderProg::UniVar* vblurSProgFaiUniVar;
		float blurringDist;
		uint blurringIterations;
		float overExposure; ///< @todo
		bool enabled;
		float renderingQuality;

		void initFbo(Fbo& fbo, Texture& fai);
};


#endif
