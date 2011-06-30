#ifndef BL_H
#define BL_H

#include "RenderingPass.h"
#include "Util/Accessors.h"
#include "Resources/Texture.h"
#include "Resources/RsrcPtr.h"
#include "GfxApi/BufferObjects/Fbo.h"


class ShaderProg;


class Bl: private RenderingPass
{
	public:
		Bl(Renderer& r_);
		void init(const RendererInitializer& initializer);
		void run();

		/// @name Accessors
		/// @{
		GETTER_SETTER_BY_VAL(bool, enabled, isEnabled, setEnabled)
		GETTER_SETTER_BY_VAL(uint, blurringIterationsNum, getBlurringIterationsNum, setBlurringIterationsNum)
		GETTER_SETTER_BY_VAL(float, sideBlurFactor, getSideBlurFactor, setSideBlurFactor)
		/// @}

	private:
		Fbo hBlurFbo; ///< Fbo that writes to blurFai
		Fbo vBlurFbo; ///< Fbo that writes to postPassSProg
		Fbo sideBlurFbo;

		RsrcPtr<ShaderProg> hBlurSProg;
		RsrcPtr<ShaderProg> vBlurSProg;
		RsrcPtr<ShaderProg> sideBlurSProg;

		Texture blurFai; ///< Temp FAI for blurring
		RsrcPtr<Texture> sideBlurMap;

		bool enabled;
		uint blurringIterationsNum;
		float sideBlurFactor;

		void runBlur();
		void runSideBlur();
};


#endif
