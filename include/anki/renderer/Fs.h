// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Forward rendering stage. The objects that blend must be handled differently
class Fs : public RenderingPass
{
anki_internal:
	Fs(Renderer* r)
		: RenderingPass(r)
	{
	}

	~Fs();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void prepareBuildCommandBuffers(RenderingContext& ctx);

	ANKI_USE_RESULT Error buildCommandBuffers(
		RenderingContext& ctx, U threadId, U threadCount) const;

	void run(RenderingContext& ctx);

	TexturePtr getRt() const
	{
		return m_rt;
	}

private:
	U m_width;
	U m_height;
	FramebufferPtr m_fb;
	TexturePtr m_rt;
	ResourceGroupPtr m_globalResources;
};
/// @}

} // end namespace anki
