#ifndef ANKI_RENDERER_SSLR_H
#define ANKI_RENDERER_SSLR_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/Resource.h"
#include "anki/Gl.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Screen space local reflections pass
class Sslr: public OptionalRenderingPass, public BlurringRenderingPass
{
	friend class Pps;

public:
	GlTextureHandle& _getRt()
	{
		return m_dirs[(U)DirectionEnum::VERTICAL].m_rt;
	}

private:
	U32 m_width;
	U32 m_height;

	// 1st pass
	ProgramResourcePointer m_reflectionFrag;
	GlProgramPipelineHandle m_reflectionPpline;
	GlSamplerHandle m_depthMapSampler;

	// 2nd pass: blit
	ProgramResourcePointer m_blitFrag;
	GlProgramPipelineHandle m_blitPpline;

	Sslr(Renderer* r)
		: OptionalRenderingPass(r)
	{}

	void init(const RendererInitializer& initializer);
	void run(GlJobChainHandle& jobs);
};

/// @}

} // end namespace anki

#endif
