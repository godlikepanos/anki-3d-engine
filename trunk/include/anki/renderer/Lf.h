#ifndef ANKI_RENDERER_LF_H
#define ANKI_RENDERER_LF_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gl.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ProgramResource.h"
#include "anki/resource/TextureResource.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Lens flare rendering pass
class Lf: public OptionalRenderingPass
{
	friend class MainRenderer;

public:
	Lf(Renderer* r)
		: OptionalRenderingPass(r)
	{}

	~Lf();

	void init(const RendererInitializer& initializer);
	void run(GlJobChainHandle& jobs);

	const GlTextureHandle& getRt() const
	{
		return m_rt;
	}
	GlTextureHandle& getRt()
	{
		return m_rt;
	}

private:
	// Pseudo flares
	ProgramResourcePointer m_pseudoFrag;
	GlProgramPipelineHandle m_pseudoPpline;

	// Sprite billboards
	ProgramResourcePointer m_realVert; 
	ProgramResourcePointer m_realFrag;
	GlProgramPipelineHandle m_realPpline;

	GlTextureHandle m_rt;
	GlFramebufferHandle m_fb;

	TextureResourcePointer m_lensDirtTex;
	U8 m_maxFlaresPerLight;
	U8 m_maxLightsWithFlares;
	GlBufferHandle m_flareDataBuff;
	
	void initInternal(const RendererInitializer& initializer);
};

/// @}

} // end namespace anki

#endif
