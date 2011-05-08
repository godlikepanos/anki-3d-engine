#ifndef PPS_H
#define PPS_H

#include "RenderingPass.h"
#include "Fbo.h"
#include "Texture.h"
#include "RsrcPtr.h"
#include "Hdr.h"
#include "Ssao.h"


class ShaderProg;


/// Post-processing stage.
///
/// This stage is divided into 2 two parts. The first happens before blending stage and the second after.
class Pps: private RenderingPass
{
	public:
		Pps(Renderer& r_);
		void init(const RendererInitializer& initializer);
		void runPrePass();
		void runPostPass();

		/// @name Accessors
		/// @{
		GETTER_R(Hdr, hdr, getHdr)
		GETTER_R(Ssao, ssao, getSsao)
		GETTER_R(Texture, prePassFai, getPrePassFai)
		GETTER_R(Texture, postPassFai, getPostPassFai)
		/// @}

	private:
		/// @name Passes
		/// @{
		Hdr hdr;
		Ssao ssao;
		/// @}

		Fbo prePassFbo;
		Fbo postPassFbo;
		Fbo hBlurFbo; ///< Fbo that writes to blurFai
		Fbo vBlurFbo; ///< Fbo that writes to postPassSProg
		Fbo sideBlurFbo;
		RsrcPtr<ShaderProg> prePassSProg;
		RsrcPtr<ShaderProg> postPassSProg;
		RsrcPtr<ShaderProg> hBlurSProg;
		RsrcPtr<ShaderProg> vBlurSProg;
		RsrcPtr<ShaderProg> sideBlurSProg;
		Texture prePassFai; ///< FAI #1
		Texture postPassFai; ///< FAI #2
		Texture blurFai; ///< Temp FAI for blurring
		RsrcPtr<Texture> sideBlur;

		void initPassFbo(Fbo& fbo, Texture& fai);

		/// Before BS pass
		void initPrePassSProg();

		/// After BS pass
		void initPostPassSProg();

		void runBlur();
};


#endif
