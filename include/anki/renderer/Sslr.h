#ifndef ANKI_RENDERER_SSLR_H
#define ANKI_RENDERER_SSLR_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/Resource.h"
#include "anki/Gl.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Screen space local reflections pass
class Sslr: public OptionalRenderingPass
{
public:
	Sslr(Renderer* r)
		: OptionalRenderingPass(r)
	{}

	void init(const RendererInitializer& initializer);
	void run(GlJobChainHandle& jobs);

private:
	U32 m_width;
	U32 m_height;

	GlFramebufferHandle m_fb;
	GlTextureHandle m_rt;

	ProgramResourcePointer m_reflectionFrag;
	GlProgramPipelineHandle m_reflectionPpline;
};

/// @}

} // end namespace anki

#endif
