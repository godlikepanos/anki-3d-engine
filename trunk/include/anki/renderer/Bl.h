// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_BL_H
#define ANKI_RENDERER_BL_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ProgramResource.h"
#include "anki/Gl.h"

namespace anki {

class ProgramResource;

/// Blurring rendering pass
class Bl: public SwitchableRenderingPass
{
public:
	Bl(Renderer* r)
		: SwitchableRenderingPass(r)
	{}

	void init(const RendererInitializer& initializer);
	void run();

#if 0
	/// @name Accessors
	/// @{
	F32 getSideBlurFactor() const
	{
		return sideBlurFactor;
	}
	F32& getSideBlurFactor()
	{
		return sideBlurFactor;
	}
	void setSideBlurFactor(const F32 x)
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
#endif

private:
#if 0
	Fbo hBlurFbo; ///< Fbo that writes to blurFai
	Fbo vBlurFbo; ///< Fbo that writes to postPassSProg
	Fbo sideBlurFbo;

	ProgramResourcePointer hBlurSProg;
	ProgramResourcePointer vBlurSProg;
	ProgramResourcePointer sideBlurSProg;

	Texture blurFai; ///< Temp FAI for blurring
	TextureResourcePointer sideBlurMap;

	uint32_t blurringIterationsNum; ///< How many times the pass will run
	float sideBlurFactor;

	void runBlur();
	void runSideBlur();
#endif
};

} // end namespace

#endif
