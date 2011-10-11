#ifndef ANKI_RENDERER_SSAO_H
#define ANKI_RENDERER_SSAO_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/Texture.h"
#include "anki/resource/ShaderProgram.h"
#include "anki/resource/RsrcPtr.h"
#include "anki/gl/Vbo.h"
#include "anki/gl/Vao.h"


/// Screen space ambient occlusion pass
///
/// Three passes:
/// 1) Calc ssao factor
/// 2) Blur vertically
/// 3) Blur horizontally repeat 2, 3
class Ssao: public SwitchableRenderingPass
{
	public:
		Ssao(Renderer& r_)
		:	SwitchableRenderingPass(r_)
		{}

		void init(const RendererInitializer& initializer);
		void run();

		/// @name Accessors
		/// @{
		float getRenderingQuality() const
		{
			return renderingQuality;
		}

		const Texture& getFai() const
		{
			return fai;
		}
		/// @}

	private:
		Texture ssaoFai; ///< It contains the unblurred SSAO factor
		Texture hblurFai;
		Texture fai;  ///< AKA vblurFai The final FAI
		float renderingQuality;
		float blurringIterationsNum;
		Fbo ssaoFbo;
		Fbo hblurFbo;
		Fbo vblurFbo;
		RsrcPtr<Texture> noiseMap;
		RsrcPtr<ShaderProgram> ssaoSProg;
		RsrcPtr<ShaderProgram> hblurSProg;
		RsrcPtr<ShaderProgram> vblurSProg;

		void createFbo(Fbo& fbo, Texture& fai);
};


#endif
