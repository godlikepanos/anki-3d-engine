// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Lens flare rendering pass. Part of forward shading.
class LensFlare : public RenderingPass
{
anki_internal:
	LensFlare(Renderer* r)
		: RenderingPass(r)
	{
	}

	~LensFlare();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void resetOcclusionQueries(RenderingContext& ctx, CommandBufferPtr cmdb);

	void runOcclusionTests(RenderingContext& ctx, CommandBufferPtr cmdb);

	void updateIndirectInfo(RenderingContext& ctx, CommandBufferPtr cmdb);

	void run(RenderingContext& ctx, CommandBufferPtr cmdb);

private:
	// Occlusion query
	DynamicArray<OcclusionQueryPtr> m_queries;
	BufferPtr m_queryResultBuff;
	BufferPtr m_indirectBuff;
	ShaderProgramResourcePtr m_updateIndirectBuffProg;
	ShaderProgramPtr m_updateIndirectBuffGrProg;

	ShaderProgramResourcePtr m_occlusionProg;
	ShaderProgramPtr m_occlusionGrProg;

	// Sprite billboards
	ShaderProgramResourcePtr m_realProg;
	ShaderProgramPtr m_realGrProg;
	U8 m_maxSpritesPerFlare;
	U8 m_maxFlares;
	U16 m_maxSprites;

	ANKI_USE_RESULT Error initSprite(const ConfigSet& config);
	ANKI_USE_RESULT Error initOcclusion(const ConfigSet& config);

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};
/// @}

} // end namespace anki
