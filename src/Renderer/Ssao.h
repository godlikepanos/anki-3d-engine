#ifndef SSAO_H
#define SSAO_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Texture.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"


/// Screen space ambient occlusion pass
///
/// Three passes:
/// 1) Calc ssao factor
/// 2) Blur vertically
/// 3) Blur horizontally repeat 2, 3
class Ssao: private RenderingPass
{
	public:
		Texture ssaoFai; ///< It contains the unblurred SSAO factor
		Texture hblurFai;
		Texture fai;  ///< AKA vblurFai The final FAI

		Ssao(Renderer& r_, Object* parent): RenderingPass(r_, parent) {}
		void init(const RendererInitializer& initializer);
		void run();

		/// @name Setters & getters
		/// @{
		bool isEnabled() const {return enabled;}
		float getRenderingQuality() const {return renderingQuality;}
		/// @}

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

		void createFbo(Fbo& fbo, Texture& fai);
};


#endif
