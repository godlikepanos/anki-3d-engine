// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

// Forward
class DepthDownscale;

/// @addtogroup renderer
/// @{

/// Quick pass to downscale the depth buffer.
class HalfDepth : public RendererObject
{
anki_internal:
	TexturePtr m_depthRt;
	TexturePtr m_colorRt;

	HalfDepth(Renderer* r, DepthDownscale* depthDownscale)
		: RendererObject(r)
		, m_parent(depthDownscale)
	{
	}

	~HalfDepth();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void setPreRunBarriers(RenderingContext& ctx);
	void run(RenderingContext& ctx);
	void setPostRunBarriers(RenderingContext& ctx);

private:
	DepthDownscale* m_parent;

	FramebufferPtr m_fb;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
};

/// Quick pass to downscale the depth buffer.
class QuarterDepth : public RendererObject
{
anki_internal:
	TexturePtr m_colorRt;

	QuarterDepth(Renderer* r, DepthDownscale* depthDownscale)
		: RendererObject(r)
		, m_parent(depthDownscale)
	{
	}

	~QuarterDepth();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void setPreRunBarriers(RenderingContext& ctx);
	void run(RenderingContext& ctx);
	void setPostRunBarriers(RenderingContext& ctx);

private:
	DepthDownscale* m_parent;

	FramebufferPtr m_fb;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
};

class DepthDownscale : public RendererObject
{
anki_internal:
	HalfDepth m_hd;
	QuarterDepth m_qd;

	DepthDownscale(Renderer* r)
		: RendererObject(r)
		, m_hd(r, this)
		, m_qd(r, this)
	{
	}

	~DepthDownscale();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

private:
	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
};
/// @}

} // end namespace
