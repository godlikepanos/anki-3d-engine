#ifndef PPS_H
#define PPS_H

#include "RenderingPass.h"
#include "gl/Fbo.h"
#include "rsrc/Texture.h"
#include "rsrc/RsrcPtr.h"
#include "Hdr.h"
#include "Ssao.h"
#include "Bl.h"


class ShaderProgram;


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
		const Hdr& getHdr() const
		{
			return hdr;
		}
		Hdr& getHdr()
		{
			return hdr;
		}

		const Ssao& getSsao() const
		{
			return ssao;
		}

		const Bl& getBl() const
		{
			return bl;
		}
		Bl& getBl()
		{
			return bl;
		}

		const Texture& getPrePassFai() const
		{
			return prePassFai;
		}

		const Texture& getPostPassFai() const
		{
			return postPassFai;
		}
		/// @}

	private:
		/// @name Passes
		/// @{
		Hdr hdr;
		Ssao ssao;
		Bl bl;
		/// @}

		Fbo prePassFbo;
		Fbo postPassFbo;

		RsrcPtr<ShaderProgram> prePassSProg;
		RsrcPtr<ShaderProgram> postPassSProg;

		Texture prePassFai; ///< FAI #1
		Texture postPassFai; ///< FAI #2
};


#endif
