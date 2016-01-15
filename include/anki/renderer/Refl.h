// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>

namespace anki
{

// Forward
class Ir;

/// @addtogroup renderer
/// @{

/// Reflections.
class Refl : public RenderingPass
{
anki_internal:
	Refl(Renderer* r);

	~Refl();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	/// Run before MS.
	ANKI_USE_RESULT Error run1(CommandBufferPtr cmdb);

	/// Run after IS.
	void run2(CommandBufferPtr cmdb);

private:
	U32 m_width = 0;
	U32 m_height = 0;

	Bool8 m_irEnabled = false;
	Bool8 m_sslrEnabled = false;

	// Sub-stages
	UniquePtr<Ir> m_ir;

	// 1st pass
	ShaderResourcePtr m_frag;
	PipelinePtr m_ppline;
	TexturePtr m_rt;
	FramebufferPtr m_fb;
	ResourceGroupPtr m_rcGroup;
	BufferPtr m_uniforms;
	Timestamp m_uniformsUpdateTimestamp = 0;

	// 2nd pass: Blit
	ResourceGroupPtr m_blitRcGroup;
	FramebufferPtr m_isFb;
	ShaderResourcePtr m_blitFrag;
	ShaderResourcePtr m_blitVert;
	PipelinePtr m_blitPpline;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
	ANKI_USE_RESULT Error init1stPass(const ConfigSet& config);
	ANKI_USE_RESULT Error init2ndPass();

	void writeUniforms(CommandBufferPtr cmdb);
};
/// @}

} // end namespace anki
