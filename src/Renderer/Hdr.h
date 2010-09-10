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
		Texture pass0Fai; ///< Vertical blur pass FAI
		Texture pass1Fai; ///< pass0Fai with the horizontal blur FAI
		Texture fai; ///< The final FAI

		Hdr(Renderer& r_): RenderingStage(r_) {}
		void init(const RendererInitializer& initializer);
		void run();

		/**
		 * Setters & getters
		 */
		/**@{*/
		float& getBlurringDist() {return blurringDist;}
		void setBlurringDist(float f) {blurringDist = f;}
		bool isEnabled() const {return enabled;}
		float getRenderingQuality() const {return renderingQuality;}
		/**@}*/

	private:
		Fbo toneFbo;
		Fbo pass1Fbo;
		Fbo pass2Fbo;
		RsrcPtr<ShaderProg> toneSProg;
		RsrcPtr<ShaderProg> pass1SProg;
		RsrcPtr<ShaderProg> pass2SProg;
		const ShaderProg::UniVar* toneProgFaiUniVar;
		const ShaderProg::UniVar* pass1SProgFaiUniVar;
		const ShaderProg::UniVar* pass2SProgFaiUniVar;
		float blurringDist;
		bool enabled;
		float renderingQuality;

		void initFbos(Fbo& fbo, Texture& fai, int internalFormat);
};


#endif
