// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>
#include <anki/util/Array.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Shadowmapping pass
class ShadowMapping : public RenderingPass
{
anki_internal:
	TexturePtr m_spotTex; ///< ESM map.
	TexturePtr m_omniTexArray; ///< ESM map.

	ShadowMapping(Renderer* r)
		: RenderingPass(r)
	{
	}

	~ShadowMapping();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	void prepareBuildCommandBuffers(RenderingContext& ctx);

	void prepareBuildCommandBuffers2(RenderingContext& ctx);

	void buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	void setPostRunBarriers(RenderingContext& ctx);

private:
	class CacheEntry
	{
	public:
		U32 m_timestamp = 0; ///< Timestamp of last render or light change
		U64 m_lightUuid = 0;
	};

	class SpotCacheEntry : public CacheEntry
	{
	public:
		Array<U32, 4> m_renderArea;
		Array<F32, 4> m_uv;
	};

	class OmniCacheEntry : public CacheEntry
	{
	public:
		U32 m_layerId;
		Array<FramebufferPtr, 6> m_fbs;
	};

	U32 m_tileResolution; ///< Shadowmap resolution
	U32 m_atlasResolution;

	DynamicArray<SpotCacheEntry> m_spots;
	DynamicArray<OmniCacheEntry> m_omnis;

	FramebufferPtr m_spotsFb;

	struct
	{
		TexturePtr m_rt; ///< Shadowmap that will be resolved to ESM maps.
		FramebufferPtr m_fb;
		U32 m_batchSize = 0;
	} m_scratchSm;

	ShaderProgramResourcePtr m_esmResolveProg;
	ShaderProgramPtr m_esmResolveGrProg;

#if 1
	class Tile
	{
	public:
		U64 m_timestamp = 0;
		U64 m_lightUuid = 0;
		U8 m_face = 0;

		Array<F32, 4> m_uv;
		Array<U32, 2> m_viewportXY;
	};

	DynamicArray<Tile> m_tiles;
	U32 m_tileCountPerRowOrColumn = 0;

	class TileKey
	{
	public:
		U64 m_lightUuid;
		U8 m_face;

		U64 computeHash() const
		{
			return anki::computeHash(this, sizeof(*this), 693);
		}
	};

	HashMap<TileKey, U32> m_lightUuidToTileIdx;

	Bool allocateTile(U64 lightTimestamp, U64 lightUuid, U32 face, U32& tileAllocated);
	static Bool shouldRenderTile(U64 lightTimestamp, U64 lightUuid, U32 face, const Tile& tileIdx);
#endif

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	/// Find the best shadowmap for that light
	template<typename TLightElement, typename TShadowmap, typename TContainer>
	void bestCandidate(const TLightElement& light, TContainer& arr, TShadowmap*& out);

	/// Check if a shadow pass can be skipped.
	Bool skip(PointLightQueueElement& light, OmniCacheEntry& sm);
	Bool skip(SpotLightQueueElement& light, SpotCacheEntry& sm);

	void doSpotLight(
		const SpotLightQueueElement& light, CommandBufferPtr& cmdBuff, U threadId, U threadCount, U tileIdx) const;

	void doOmniLight(
		const PointLightQueueElement& light, CommandBufferPtr cmdbs[], U threadId, U threadCount, U firstTileIdx) const;
};
/// @}

} // end namespace anki
