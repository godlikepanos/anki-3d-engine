// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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
	static const U ATTACHMENT_COUNT = 3;
	static const Array<PixelFormat, ATTACHMENT_COUNT> RT_PIXEL_FORMATS;
	static const PixelFormat DEPTH_RT_PIXEL_FORMAT;

	Ms(Renderer* r)
		: RenderingPass(r)
	{
	}

	~Ms();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void prepareBuildCommandBuffers(RenderingContext& ctx)
	{
	}

	ANKI_USE_RESULT Error buildCommandBuffers(
		RenderingContext& ctx, U threadId, U threadCount) const;

	void run(RenderingContext& ctx);

	TexturePtr& getRt0()
	{
		return m_rt0;
	}

	TexturePtr& getRt1()
	{
		return m_rt1;
	}

	TexturePtr& getRt2()
	{
		return m_rt2;
	}

	TexturePtr getDepthRt() const
	{
		return m_depthRt;
	}

	FramebufferPtr& getFramebuffer()
	{
		return m_fb;
	}

private:
	FramebufferPtr m_fb;

	/// Contains diffuse color and emission
	TexturePtr m_rt0;

	/// Contains specular color, roughness
	TexturePtr m_rt1;

	/// Contains normals
	TexturePtr m_rt2;

	/// Depth stencil
	TexturePtr m_depthRt;

	DArray<CommandBufferPtr> m_secondLevelCmdbs;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	/// Create a G buffer FBO
	ANKI_USE_RESULT Error createRt(U32 samples);
};
/// @}

} // end namespace anki
