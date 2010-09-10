#ifndef SSAO_H
#define SSAO_H

#include "Common.h"
#include "RenderingStage.h"
#include "Fbo.h"
#include "Texture.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"


/**
 * Screen space ambient occlusion pass
 *
 * Three passes:
 * - Calc ssao factor
 * - Blur vertically
 * - Blur horizontally
 */
class Ssao: private RenderingStage
{
	public:
		Texture pass0Fai;
		Texture pass1Fai;
		Texture fai;  ///< The final FAI

		Ssao(Renderer& r_): RenderingStage(r_) {}
		void init(const RendererInitializer& initializer);
		void run();

		/**
		 * @name Setters & getters
		 */
		/**@{*/
		bool isEnabled() const {return enabled;}
		float getRenderingQuality() const {return renderingQuality;}
		float getBluringQuality() const {return bluringQuality;}
		/**@}*/

	private:
		bool enabled;
		float renderingQuality;
		float bluringQuality;
		Fbo pass0Fbo;
		Fbo pass1Fbo;
		Fbo pass2Fbo;
		uint width, height, bwidth, bheight;
		RsrcPtr<Texture> noiseMap;
		RsrcPtr<ShaderProg> ssaoSProg;
		RsrcPtr<ShaderProg> blurSProg;
		RsrcPtr<ShaderProg> blurSProg2;
		const ShaderProg::UniVar* camerarangeUniVar;
		const ShaderProg::UniVar* msDepthFaiUniVar;
		const ShaderProg::UniVar* noiseMapUniVar;
		const ShaderProg::UniVar* msNormalFaiUniVar;
		const ShaderProg::UniVar* blurSProgFaiUniVar;
		const ShaderProg::UniVar* blurSProg2FaiUniVar;

		void initBlurFbo(Fbo& fbo, Texture& fai);
};


#endif
