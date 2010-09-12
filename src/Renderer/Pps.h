#ifndef PPS_H
#define PPS_H

#include "Common.h"
#include "RenderingStage.h"
#include "Fbo.h"
#include "Hdr.h"
#include "Ssao.h"


/**
 * Post-processing stage
 *
 * This stage is divided into 2 two parts. The first happens before blending stage and the second after.
 */
class Pps: private RenderingStage
{
	public:
		struct UniVars
		{
			const ShaderProg::UniVar* isFai;
			const ShaderProg::UniVar* ppsPrePassFai;
			const ShaderProg::UniVar* ppsSsaoFai;
			const ShaderProg::UniVar* ppsHdrFai;
		};

	public:
		Hdr hdr;
		Ssao ssao;
		Texture prePassFai;
		Texture postPassFai;

		Pps(Renderer& r_): RenderingStage(r_), hdr(r_), ssao(r_) {}
		void init(const RendererInitializer& initializer);
		void runPrePass();
		void runPostPass();

	private:
		Fbo prePassFbo;
		Fbo postPassFbo;
		RsrcPtr<ShaderProg> prePassSProg;
		RsrcPtr<ShaderProg> postPassSProg;
		UniVars prePassSProgUniVars;
		UniVars postPassSProgUniVars;

		void initPassFbo(Fbo& fbo, Texture& fai, const char* msg);

		/**
		 * before BS pass
		 */
		void initPrePassSProg();

		/**
		 * after BS pass
		 */
		void initPostPassSProg();
};


#endif
