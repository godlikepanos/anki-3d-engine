#ifndef R_BL_H
#define R_BL_H

#include "RenderingPass.h"
#include "util/Accessors.h"
#include "rsrc/Texture.h"
#include "rsrc/RsrcPtr.h"
#include "gl/Fbo.h"


class ShaderProgram;


namespace r {


class Bl: private RenderingPass
{
	public:
		Bl(Renderer& r_);
		void init(const RendererInitializer& initializer);
		void run();

		/// @name Accessors
		/// @{
		GETTER_SETTER_BY_VAL(bool, enabled, isEnabled, setEnabled)
		GETTER_SETTER_BY_VAL(uint, blurringIterationsNum,
			getBlurringIterationsNum, setBlurringIterationsNum)
		GETTER_SETTER_BY_VAL(float, sideBlurFactor, getSideBlurFactor,
			setSideBlurFactor)
		/// @}

	private:
		Fbo hBlurFbo; ///< Fbo that writes to blurFai
		Fbo vBlurFbo; ///< Fbo that writes to postPassSProg
		Fbo sideBlurFbo;

		RsrcPtr<ShaderProgram> hBlurSProg;
		RsrcPtr<ShaderProgram> vBlurSProg;
		RsrcPtr<ShaderProgram> sideBlurSProg;

		Texture blurFai; ///< Temp FAI for blurring
		RsrcPtr<Texture> sideBlurMap;

		bool enabled;
		uint blurringIterationsNum;
		float sideBlurFactor;

		void runBlur();
		void runSideBlur();
};


} // end namespace


#endif
