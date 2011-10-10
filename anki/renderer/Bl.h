#ifndef BL_H
#define BL_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/Texture.h"
#include "anki/resource/RsrcPtr.h"
#include "anki/gl/Fbo.h"


class ShaderProgram;


/// Blurring rendering pass
class Bl: public SwitchableRenderingPass
{
	public:
		Bl(Renderer& r_);
		void init(const RendererInitializer& initializer);
		void run();

		/// @name Accessors
		/// @{
		float getSideBlurFactor() const
		{
			return sideBlurFactor;
		}
		float& getSideBlurFactor()
		{
			return sideBlurFactor;
		}
		void setSideBlurFactor(const float x)
		{
			sideBlurFactor = x;
		}

		uint getBlurringIterationsNum() const
		{
			return blurringIterationsNum;
		}
		uint& getBlurringIterationsNum()
		{
			return blurringIterationsNum;
		}
		void setBlurringIterationsNum(const uint x)
		{
			blurringIterationsNum = x;
		}
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

		uint blurringIterationsNum;
		float sideBlurFactor;

		void runBlur();
		void runSideBlur();
};


#endif
