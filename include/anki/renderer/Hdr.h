#ifndef ANKI_RENDERER_HDR_H
#define ANKI_RENDERER_HDR_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Fbo.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/Resource.h"
#include "anki/core/Timestamp.h"
#include "anki/gl/Ubo.h"

namespace anki {

class ShaderProgram;

/// High dynamic range lighting pass
class Hdr: public SwitchableRenderingPass
{
public:
	Hdr(Renderer* r_)
		: SwitchableRenderingPass(r_)
	{}

	~Hdr();

	void init(const RendererInitializer& initializer);
	void run();

	/// @name Accessors
	/// @{
	F32 getExposure() const
	{
		return exposure;
	}
	void setExposure(const F32 x)
	{
		exposure = x;
		parameterUpdateTimestamp = Timestamp::getTimestamp();
	}

	U32 getBlurringIterationsCount() const
	{
		return blurringIterationsCount;
	}
	void setBlurringIterationsCount(const U32 x)
	{
		blurringIterationsCount = x;
	}

	const Texture& getFai() const
	{
		return vblurFai;
	}
	/// @}

private:
	U32 width;
	U32 height;
	F32 exposure = 4.0; ///< How bright is the HDR
	U32 blurringIterationsCount = 2; ///< The blurring iterations
	F32 blurringDist = 1.0; ///< Distance in blurring
	F32 renderingQuality = 0.5;
	Fbo hblurFbo;
	Fbo vblurFbo;
	ShaderProgramResourcePointer toneSProg;
	ShaderProgramResourcePointer hblurSProg;
	ShaderProgramResourcePointer vblurSProg;
	Texture hblurFai; ///< pass0Fai with the horizontal blur FAI
	Texture vblurFai; ///< The final FAI
	/// When a parameter changed by the setters
	U32 parameterUpdateTimestamp = Timestamp::getTimestamp();
	/// When the commonUbo got updated
	U32 commonUboUpdateTimestamp = Timestamp::getTimestamp();
	Ubo commonUbo;

	void initFbo(Fbo& fbo, Texture& fai);
	void initInternal(const RendererInitializer& initializer);
};

} // end namespace anki

#endif
