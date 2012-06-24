#ifndef ANKI_RENDERER_BL_H
#define ANKI_RENDERER_BL_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/gl/Fbo.h"

namespace anki {

class ShaderProgramResource;

/// Blurring rendering pass
class Bl: public SwitchableRenderingPass
{
public:
	Bl(Renderer* r)
		: SwitchableRenderingPass(r)
	{}

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

	uint32_t getBlurringIterationsNum() const
	{
		return blurringIterationsNum;
	}
	uint32_t& getBlurringIterationsNum()
	{
		return blurringIterationsNum;
	}
	void setBlurringIterationsNum(const uint32_t x)
	{
		blurringIterationsNum = x;
	}
	/// @}

private:
	Fbo hBlurFbo; ///< Fbo that writes to blurFai
	Fbo vBlurFbo; ///< Fbo that writes to postPassSProg
	Fbo sideBlurFbo;

	ShaderProgramResourcePointer hBlurSProg;
	ShaderProgramResourcePointer vBlurSProg;
	ShaderProgramResourcePointer sideBlurSProg;

	Texture blurFai; ///< Temp FAI for blurring
	TextureResourcePointer sideBlurMap;

	uint32_t blurringIterationsNum; ///< How many times the pass will run
	float sideBlurFactor;

	void runBlur();
	void runSideBlur();
};

} // end namespace

#endif
