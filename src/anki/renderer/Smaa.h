// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>

namespace anki
{

/// @addtogroup renderer
/// @{

class SmaaEdge : public RenderingPass
{
anki_internal:
	TexturePtr m_rt;

	SmaaEdge(Renderer* r)
		: RenderingPass(r)
	{
	}

	~SmaaEdge();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void setPreRunBarriers(RenderingContext& ctx);
	void run(RenderingContext& ctx);
	void setPostRunBarriers(RenderingContext& ctx);

private:
	FramebufferPtr m_fb;
	ShaderResourcePtr m_vert;
	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;
};

class SmaaWeights : public RenderingPass
{
anki_internal:
	TexturePtr m_rt;

	SmaaWeights(Renderer* r)
		: RenderingPass(r)
	{
	}

	~SmaaWeights();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void setPreRunBarriers(RenderingContext& ctx);
	void run(RenderingContext& ctx);
	void setPostRunBarriers(RenderingContext& ctx);

private:
	FramebufferPtr m_fb;
	ShaderResourcePtr m_vert;
	ShaderResourcePtr m_frag;
	ShaderProgramPtr m_prog;
	TexturePtr m_areaTex;
	TexturePtr m_searchTex;
};

class Smaa : public RenderingPass
{
anki_internal:
	SmaaEdge m_edge;
	SmaaWeights m_weights;
	CString m_qualityPerset;
	TexturePtr m_stencilTex;

	Smaa(Renderer* r)
		: RenderingPass(r)
		, m_edge(r)
		, m_weights(r)
	{
	}

	~Smaa()
	{
	}

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

private:
	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
};
/// @}

} // end namespace anki
