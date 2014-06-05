// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_PPS_H
#define ANKI_RENDERER_PPS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gl.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Ssao.h"
#include "anki/renderer/Bl.h"
#include "anki/renderer/Lf.h"
#include "anki/renderer/Sslr.h"

namespace anki {

class ShaderProgram;

/// @addtogroup renderer
/// @{

/// Post-processing stage.This stage is divided into 2 two parts. The first
/// happens before blending stage and the second after
class Pps: public OptionalRenderingPass
{
	friend class Renderer;

public:
	/// @name Accessors
	/// @{
	const Hdr& getHdr() const
	{
		return m_hdr;
	}
	Hdr& getHdr()
	{
		return m_hdr;
	}

	const Ssao& getSsao() const
	{
		return m_ssao;
	}
	Ssao& getSsao()
	{
		return m_ssao;
	}

	const Bl& getBl() const
	{
		return m_bl;
	}
	Bl& getBl()
	{
		return m_bl;
	}

	const Lf& getLf() const
	{
		return m_lf;
	}
	Lf& getLf()
	{
		return m_lf;
	}

	const Sslr& getSslr() const
	{
		return m_sslr;
	}
	Sslr& getSslr()
	{
		return m_sslr;
	}
	/// @}

	/// @privatesection
	/// @{
	const GlTextureHandle& _getRt() const
	{
		return m_rt;
	}
	GlTextureHandle& _getRt()
	{
		return m_rt;
	}
	/// @}

private:
	/// @name Passes
	/// @{
	Hdr m_hdr;
	Ssao m_ssao;
	Bl m_bl;
	Lf m_lf;
	Sslr m_sslr;
	/// @}

	GlFramebufferHandle m_fb;
	ProgramResourcePointer m_frag;
	GlProgramPipelineHandle m_ppline;
	GlTextureHandle m_rt;

	Pps(Renderer* r);
	~Pps();

	void init(const RendererInitializer& initializer);
	void run(GlJobChainHandle& jobs);

	void initInternal(const RendererInitializer& initializer);
};

/// @}

} // end namespace anki

#endif
