// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_PPS_H
#define ANKI_RENDERER_PPS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/TextureResource.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Post-processing stage.
class Pps: public RenderingPass
{
public:
	Pps(Renderer* r);
	~Pps();

	/// @privatesection
	/// @{
	ANKI_USE_RESULT Error init(const ConfigSet& config);
	void run(CommandBufferPtr& jobs);

	const TexturePtr& getRt() const
	{
		return m_rt;
	}

	TexturePtr& getRt()
	{
		return m_rt;
	}
	/// @}

	const Bloom& getBloom() const
	{
		return *m_bloom;
	}

	Bloom& getBloom()
	{
		return *m_bloom;
	}

	const Ssao& getSsao() const
	{
		return *m_ssao;
	}

	Ssao& getSsao()
	{
		return *m_ssao;
	}

	const Sslr& getSslr() const
	{
		return *m_sslr;
	}

	Sslr& getSslr()
	{
		return *m_sslr;
	}

	const Tm& getTm() const
	{
		return *m_tm;
	}

	Tm& getTm()
	{
		return *m_tm;
	}

	/// Load the color grading texture.
	Error loadColorGradingTexture(CString filename);

private:
	UniquePtr<Ssao> m_ssao;
	UniquePtr<Sslr> m_sslr;
	UniquePtr<Tm> m_tm;
	UniquePtr<Bloom> m_bloom;
	UniquePtr<Sslf> m_sslf;

	FramebufferPtr m_fb;
	ShaderResourcePointer m_frag;
	PipelinePtr m_ppline;
	TexturePtr m_rt;

	TextureResourcePointer m_lut; ///< Color grading lookup texture.

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
};
/// @}

} // end namespace anki

#endif
