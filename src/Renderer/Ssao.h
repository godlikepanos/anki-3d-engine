#ifndef SSAO_H
#define SSAO_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Texture.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"
#include "Vbo.h"
#include "Vao.h"


/// Screen space ambient occlusion pass
///
/// Three passes:
/// 1) Calc ssao factor
/// 2) Blur vertically
/// 3) Blur horizontally repeat 2, 3
class Ssao: private RenderingPass
{
	public:
		Ssao(Renderer& r_): RenderingPass(r_) {}
		void init(const RendererInitializer& initializer);
		void run();

		/// @name Setters & getters
		/// @{
		bool isEnabled() const {return enabled;}
		float getRenderingQuality() const {return renderingQuality;}
		const Texture& getFai() const {return fai;}
		/// @}

	private:
		Texture ssaoFai; ///< It contains the unblurred SSAO factor
		Texture hblurFai;
		Texture fai;  ///< AKA vblurFai The final FAI
		bool enabled;
		float renderingQuality;
		float blurringIterationsNum;
		Fbo ssaoFbo;
		Fbo hblurFbo;
		Fbo vblurFbo;
		RsrcPtr<Texture> noiseMap;
		RsrcPtr<ShaderProg> ssaoSProg;
		RsrcPtr<ShaderProg> hblurSProg;
		RsrcPtr<ShaderProg> vblurSProg;
		
		/// @name For the quad drawing in light passes
		/// @{
		Vbo quadPositionsVbo; ///< The VBO for quad positions
		Vbo viewVectorsVbo; ///< The VBO to pass the @ref viewVectors.
		Vbo quadVertIndecesVbo; ///< The VBO for quad array buffer elements
		Vao vao; ///< This VAO is used in light passes only
		/// @}

		void createFbo(Fbo& fbo, Texture& fai);
};


#endif
