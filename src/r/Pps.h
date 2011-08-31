#ifndef R_PPS_H
#define R_PPS_H

#include "RenderingPass.h"
#include "gl/Fbo.h"
#include "rsrc/Texture.h"
#include "rsrc/RsrcPtr.h"
#include "Hdr.h"
#include "Ssao.h"
#include "Bl.h"


class ShaderProgram;


namespace r {


/// Post-processing stage.
///
/// This stage is divided into 2 two parts. The first happens before blending
/// stage and the second after.
class Pps: private RenderingPass
{
	public:
		Pps(Renderer& r_);
		~Pps();
		void init(const RendererInitializer& initializer);
		void runPrePass();
		void runPostPass();

		/// @name Accessors
		/// @{
		GETTER_RW(Hdr, hdr, getHdr)
		GETTER_RW(Ssao, ssao, getSsao)
		GETTER_RW(Bl, bl, getBl)
		GETTER_R(Texture, prePassFai, getPrePassFai)
		GETTER_R(Texture, postPassFai, getPostPassFai)
		/// @}

	private:
		/// @name Passes
		/// @{
		Hdr hdr;
		Ssao ssao;
		Bl bl;
		/// @}

		gl::Fbo prePassFbo;
		gl::Fbo postPassFbo;

		RsrcPtr<ShaderProgram> prePassSProg;
		RsrcPtr<ShaderProgram> postPassSProg;

		Texture prePassFai; ///< FAI #1
		Texture postPassFai; ///< FAI #2
};


} // end namespace


#endif
