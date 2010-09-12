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
 * 1) Calc ssao factor
 * 2) Blur vertically
 * 3) Blur horizontally repeat 2, 3
 */
class Ssao: private RenderingStage
{
	public:
		Texture ssaoFai; ///< It contains the unblurred SSAO factor
		Texture hblurFai;
		Texture fai;  ///< AKA vblurFai The final FAI

		Ssao(Renderer& r_): RenderingStage(r_) {}
		void init(const RendererInitializer& initializer);
		void run();

		/**
		 * @name Setters & getters
		 */
		/**@{*/
		bool isEnabled() const {return enabled;}
		float getRenderingQuality() const {return renderingQuality;}
		/**@}*/

	private:
		bool enabled;
		float renderingQuality;
		float blurringIterations;
		Fbo ssaoFbo;
		Fbo hblurFbo;
		Fbo vblurFbo;
		RsrcPtr<Texture> noiseMap;
		RsrcPtr<ShaderProg> ssaoSProg;
		RsrcPtr<ShaderProg> hblurSProg;
		RsrcPtr<ShaderProg> vblurSProg;

		/**
		 * Pointers to some uniforms
		 */
		/**@{*/
		const ShaderProg::UniVar* camerarangeUniVar;
		const ShaderProg::UniVar* msDepthFaiUniVar;
		const ShaderProg::UniVar* noiseMapUniVar;
		const ShaderProg::UniVar* msNormalFaiUniVar;
		const ShaderProg::UniVar* imgHblurSProgUniVar;
		const ShaderProg::UniVar* dimensionHblurSProgUniVar;
		const ShaderProg::UniVar* imgVblurSProgUniVar;
		const ShaderProg::UniVar* dimensionVblurSProgUniVar;
		/**@}*/

		void createFbo(Fbo& fbo, Texture& fai);
};


#endif
