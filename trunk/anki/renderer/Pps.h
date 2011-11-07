#ifndef ANKI_RENDERER_PPS_H
#define ANKI_RENDERER_PPS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/Texture.h"
#include "anki/resource/Resource.h"
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Ssao.h"
#include "anki/renderer/Bl.h"


namespace anki {


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

		ShaderProgramResourcePointer prePassSProg;
		ShaderProgramResourcePointer postPassSProg;

		Texture prePassFai; ///< FAI #1
		Texture postPassFai; ///< FAI #2
};


} // end namespace


#endif
