#ifndef ANKI_RENDERER_HDR_H
#define ANKI_RENDERER_HDR_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/Texture.h"
#include "anki/resource/RsrcPtr.h"


namespace anki {


class ShaderProgram;


/// High dynamic range lighting pass
class Hdr: public SwitchableRenderingPass
{
	public:
		Hdr(Renderer& r_);
		~Hdr();
		void init(const RendererInitializer& initializer);
		void run();

		/// @name Accessors
		/// @{
		float getExposure() const
		{
			return exposure;
		}
		float& getExposure()
		{
			return exposure;
		}
		void setExposure(const float x)
		{
			exposure = x;
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

		float getBlurringDistance() const
		{
			return blurringDist;
		}
		float& getBlurringDistance()
		{
			return blurringDist;
		}
		void setBlurringDistance(const float x)
		{
			blurringDist = x;
		}

		float getRenderingQuality() const
		{
			return renderingQuality;
		}

		const Texture& getToneFai() const
		{
			return toneFai;
		}

		const Texture& getHblurFai() const
		{
			return hblurFai;
		}

		const Texture& getFai() const
		{
			return fai;
		}
		/// @}

	private:
		float exposure; ///< How bright is the HDR
		uint blurringIterationsNum; ///< The blurring iterations of the tone map
		float blurringDist; ///< Distance in blurring
		float renderingQuality;
		Fbo toneFbo;
		Fbo hblurFbo;
		Fbo vblurFbo;
		RsrcPtr<ShaderProgram> toneSProg;
		RsrcPtr<ShaderProgram> hblurSProg;
		RsrcPtr<ShaderProgram> vblurSProg;
		Texture toneFai; ///< Vertical blur pass FAI
		Texture hblurFai; ///< pass0Fai with the horizontal blur FAI
		Texture fai; ///< The final FAI

		void initFbo(Fbo& fbo, Texture& fai);
};


} // end namespace


#endif
