// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderResource.h>

namespace anki
{

// Forward
class DepthDownscale;

/// @addtogroup renderer
/// @{

/// Quick pass to downscale the depth buffer.
class HalfDepth : public RenderingPass
{
anki_internal:
	TexturePtr m_depthRt;

	HalfDepth(Renderer* r, DepthDownscale* depthDownscale)
		: RenderingPass(r)
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
	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;
};

/// Quick pass to downscale the depth buffer.
class QuarterDepth : public RenderingPass
{
anki_internal:
	TexturePtr m_depthRt;

	QuarterDepth(Renderer* r, DepthDownscale* depthDownscale)
		: RenderingPass(r)
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
	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;
};

class DepthDownscale : public RenderingPass
{
anki_internal:
	HalfDepth m_hd;
	QuarterDepth m_qd;

	DepthDownscale(Renderer* r)
		: RenderingPass(r)
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
