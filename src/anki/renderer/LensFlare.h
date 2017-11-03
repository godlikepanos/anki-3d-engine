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

/// @addtogroup renderer
/// @{

/// Lens flare rendering pass. Part of forward shading.
class LensFlare : public RendererObject
{
anki_internal:
	LensFlare(Renderer* r)
		: RendererObject(r)
	{
	}

	~LensFlare();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void runOcclusionTests(const RenderingContext& ctx, CommandBufferPtr& cmdb);
	void runDrawFlares(const RenderingContext& ctx, CommandBufferPtr& cmdb);

	void populateRenderGraph(RenderingContext& ctx);

	/// Get it to set a dependency.
	RenderPassBufferHandle getIndirectDrawBuffer() const
	{
		return m_runCtx.m_indirectBuffHandle;
	}

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

	class
	{
	public:
		RenderingContext* m_ctx = nullptr;
		RenderPassBufferHandle m_queryResultBuffHandle;
		RenderPassBufferHandle m_indirectBuffHandle;
	} m_runCtx;

	ANKI_USE_RESULT Error initSprite(const ConfigSet& config);
	ANKI_USE_RESULT Error initOcclusion(const ConfigSet& config);

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	void resetOcclusionQueries(const RenderingContext& ctx, CommandBufferPtr& cmdb);
	void copyQueryResult(const RenderingContext& ctx, const RenderGraph& rgraph, CommandBufferPtr& cmdb);
	void updateIndirectInfo(const RenderingContext& ctx, const RenderGraph& rgraph, CommandBufferPtr& cmdb);

	/// A RenderPassWorkCallback for clearing the ocl queries.
	static void runResetOclQueriesCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		LensFlare* self = static_cast<LensFlare*>(userData);
		self->resetOcclusionQueries(*self->m_runCtx.m_ctx, cmdb);
	}

	/// A RenderPassWorkCallback for copying the query results to a buffer.
	static void runCopyQueryResultCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		LensFlare* self = static_cast<LensFlare*>(userData);
		self->copyQueryResult(*self->m_runCtx.m_ctx, rgraph, cmdb);
	}

	/// A RenderPassWorkCallback for updating the indirect info.
	static void runUpdateIndirectCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		LensFlare* self = static_cast<LensFlare*>(userData);
		self->updateIndirectInfo(*self->m_runCtx.m_ctx, rgraph, cmdb);
	}
};
/// @}

} // end namespace anki
