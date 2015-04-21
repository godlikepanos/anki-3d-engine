// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_PPS_H
#define ANKI_RENDERER_PPS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gr.h"
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
class Pps: public RenderingPass
{
	friend class Renderer;

public:
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

	/// Load the color grading texture.
	Error loadColorGradingTexture(CString filename);

	/// @privatesection
	/// @{
	const TextureHandle& _getRt() const
	{
		return m_rt;
	}
	TextureHandle& _getRt()
	{
		return m_rt;
	}
	/// @}

private:
	Hdr m_hdr;
	Ssao m_ssao;
	Bl m_bl;
	Lf m_lf;
	Sslr m_sslr;

	FramebufferHandle m_fb;
	ShaderResourcePointer m_frag;
	PipelineHandle m_ppline;
	TextureHandle m_rt;

	TextureResourcePointer m_lut; ///< Color grading lookup texture.

	Pps(Renderer* r);
	~Pps();

	ANKI_USE_RESULT Error init(const ConfigSet& config);
	ANKI_USE_RESULT Error run(CommandBufferHandle& jobs);

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
};

/// @}

} // end namespace anki

#endif
