// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/Gr.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Material stage also known as G buffer stage. It populates the G buffer
class Ms : public RenderingPass
{
anki_internal:
	TexturePtr m_rt0;
	TexturePtr m_rt1;
	TexturePtr m_rt2;
	TexturePtr m_depthRt;

	Ms(Renderer* r)
		: RenderingPass(r)
	{
	}

	~Ms();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount) const;

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

private:
	FramebufferPtr m_fb;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	/// Create a G buffer FBO
	ANKI_USE_RESULT Error createRt();
};
/// @}

} // end namespace anki
