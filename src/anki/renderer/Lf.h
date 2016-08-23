// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/Gr.h>
#include <anki/resource/ShaderResource.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Lens flare rendering pass. Part of forward shading.
class Lf : public RenderingPass
{
anki_internal:
	Lf(Renderer* r)
		: RenderingPass(r)
	{
	}

	~Lf();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void resetOcclusionQueries(RenderingContext& ctx, CommandBufferPtr cmdb);

	void runOcclusionTests(RenderingContext& ctx, CommandBufferPtr cmdb);

	void run(RenderingContext& ctx, CommandBufferPtr cmdb);

private:
	// Occlusion query
	ShaderResourcePtr m_occlusionVert;
	ShaderResourcePtr m_occlusionFrag;
	PipelinePtr m_occlusionPpline;
	ResourceGroupPtr m_occlusionRcGroup;

	// Sprite billboards
	ShaderResourcePtr m_realVert;
	ShaderResourcePtr m_realFrag;
	PipelinePtr m_realPpline;
	U8 m_maxSpritesPerFlare;
	U8 m_maxFlares;
	U16 m_maxSprites;

	ANKI_USE_RESULT Error initSprite(const ConfigSet& config);
	ANKI_USE_RESULT Error initOcclusion(const ConfigSet& config);

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};
/// @}

} // end namespace anki
