// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/core/App.h>
#include <anki/core/Trace.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/ThreadHive.h>

namespace anki
{

struct ShadowMapping::LightToRenderToScratchInfo
{
	Array<U32, 4> m_viewport;
	RenderQueue* m_renderQueue;
	U32 m_drawcallCount;
};

ShadowMapping::~ShadowMapping()
{
	m_tiles.destroy(getAllocator());
	m_lightUuidToTileIdx.destroy(getAllocator());
}

Error ShadowMapping::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing shadowmapping");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize shadowmapping");
	}

	return err;
}

Error ShadowMapping::initScratch(const ConfigSet& cfg)
{
	// Init the shadowmaps and FBs
	{
		m_scratchTileCount = cfg.getNumber("r.shadowMapping.scratchTileCount");
		m_scratchTileResolution = cfg.getNumber("r.shadowMapping.resolution");

		// RT
		m_scratchRtDescr = m_r->create2DRenderTargetDescription(m_scratchTileResolution * m_scratchTileCount,
			m_scratchTileResolution,
			SHADOW_DEPTH_PIXEL_FORMAT,
			"Scratch ShadMap");
		m_scratchRtDescr.bake();

		// FB
		m_scratchFbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
		m_scratchFbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;
		m_scratchFbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
		m_scratchFbDescr.bake();
	}

	return Error::NONE;
}

Error ShadowMapping::initEsm(const ConfigSet& cfg)
{
	// Init RTs and FBs
	{
		m_tileResolution = cfg.getNumber("r.shadowMapping.resolution");
		m_tileCountPerRowOrColumn = cfg.getNumber("r.shadowMapping.tileCountPerRowOrColumn");
		m_atlasResolution = m_tileResolution * m_tileCountPerRowOrColumn;

		// RT
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(m_atlasResolution,
			m_atlasResolution,
			SHADOW_COLOR_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
				| TextureUsageBit::SAMPLED_COMPUTE,
			"esm");
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		ClearValue clearVal;
		clearVal.m_colorf[0] = 1.0f;
		m_esmAtlas = m_r->createAndClearRenderTarget(texinit, clearVal);

		// FB
		m_esmFbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::LOAD;
		m_esmFbDescr.m_colorAttachmentCount = 1;
		m_esmFbDescr.bake();
	}

	// Tiles
	{
		m_tiles.create(getAllocator(), m_tileCountPerRowOrColumn * m_tileCountPerRowOrColumn);

		for(U y = 0; y < m_tileCountPerRowOrColumn; ++y)
		{
			for(U x = 0; x < m_tileCountPerRowOrColumn; ++x)
			{
				const U tileIdx = y * m_tileCountPerRowOrColumn + x;
				Tile& tile = m_tiles[tileIdx];

				tile.m_uv[0] = F32(x) / m_tileCountPerRowOrColumn;
				tile.m_uv[1] = F32(y) / m_tileCountPerRowOrColumn;
				tile.m_uv[2] = 1.0f / m_tileCountPerRowOrColumn;
				tile.m_uv[3] = tile.m_uv[2];

				tile.m_viewport[0] = x * m_tileResolution;
				tile.m_viewport[1] = y * m_tileResolution;
				tile.m_viewport[2] = m_tileResolution;
				tile.m_viewport[3] = m_tileResolution;
			}
		}

		// The first tile is always pinned
		m_tiles[0].m_pinned = true;
	}

	// Programs and shaders
	{
		ANKI_CHECK(
			getResourceManager().loadResource("shaders/ExponentialShadowmappingResolve.glslp", m_esmResolveProg));

		ShaderProgramResourceConstantValueInitList<1> consts(m_esmResolveProg);
		consts.add("INPUT_TEXTURE_SIZE", UVec2(m_scratchTileCount * m_scratchTileResolution, m_scratchTileResolution));

		const ShaderProgramResourceVariant* variant;
		m_esmResolveProg->getOrCreateVariant(consts.get(), variant);
		m_esmResolveGrProg = variant->getProgram();
	}

	return Error::NONE;
}

Error ShadowMapping::initInternal(const ConfigSet& cfg)
{
	ANKI_CHECK(initScratch(cfg));
	ANKI_CHECK(initEsm(cfg));

	return Error::NONE;
}

void ShadowMapping::runEsm(RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_esmResolveWorkItems.getSize());
	ANKI_TRACE_SCOPED_EVENT(R_SM);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_esmResolveGrProg);
	rgraphCtx.bindTextureAndSampler(
		0, 0, m_scratchRt, TextureSubresourceInfo(DepthStencilAspectBit::DEPTH), m_r->getLinearSampler());

	for(const EsmResolveWorkItem& workItem : m_esmResolveWorkItems)
	{
		ANKI_TRACE_INC_COUNTER(R_SHADOW_PASSES, 1);

		cmdb->setViewport(
			workItem.m_viewportOut[0], workItem.m_viewportOut[1], workItem.m_viewportOut[2], workItem.m_viewportOut[3]);
		cmdb->setScissor(
			workItem.m_viewportOut[0], workItem.m_viewportOut[1], workItem.m_viewportOut[2], workItem.m_viewportOut[3]);

		Vec4* unis = allocateAndBindUniforms<Vec4*>(sizeof(Vec4) * 2, cmdb, 0, 0);
		unis[0] = Vec4(workItem.m_cameraNear, workItem.m_cameraFar, 0.0f, 0.0f);
		unis[1] = workItem.m_uvIn;

		drawQuad(cmdb);
	}

	// Restore GR state
	cmdb->setScissor(0, 0, MAX_U32, MAX_U32);
}

void ShadowMapping::runShadowMapping(RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_scratchWorkItems.getSize());
	ANKI_TRACE_SCOPED_EVENT(R_SM);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const U threadIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;

	for(ScratchBufferWorkItem& work : m_scratchWorkItems)
	{
		if(work.m_threadPoolTaskIdx != threadIdx)
		{
			continue;
		}

		// Set state
		cmdb->setViewport(work.m_viewport[0], work.m_viewport[1], work.m_viewport[2], work.m_viewport[3]);
		cmdb->setScissor(work.m_viewport[0], work.m_viewport[1], work.m_viewport[2], work.m_viewport[3]);

		m_r->getSceneDrawer().drawRange(Pass::SM,
			work.m_renderQueue->m_viewMatrix,
			work.m_renderQueue->m_viewProjectionMatrix,
			Mat4::getIdentity(), // Don't care about prev matrices here
			cmdb,
			work.m_renderQueue->m_renderables.getBegin() + work.m_firstRenderableElement,
			work.m_renderQueue->m_renderables.getBegin() + work.m_firstRenderableElement
				+ work.m_renderableElementCount);
	}
}

void ShadowMapping::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_SM);

	// First process the lights
	U32 threadCountForScratchPass = 0;
	processLights(ctx, threadCountForScratchPass);

	// Build the render graph
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	if(m_scratchWorkItems.getSize())
	{
		// Will have to create render passes

		// Scratch pass
		{
			// Compute render area
			const U32 minx = 0, miny = 0, height = m_scratchTileResolution;
			const U32 width = m_scratchTileResolution * m_scratchWorkItems.getSize();

			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("SM scratch");

			m_scratchRt = rgraph.newRenderTarget(m_scratchRtDescr);
			pass.setFramebufferInfo(m_scratchFbDescr, {}, m_scratchRt, minx, miny, width, height);
			ANKI_ASSERT(
				threadCountForScratchPass && threadCountForScratchPass <= m_r->getThreadHive().getThreadCount());
			pass.setWork(runShadowmappingCallback, this, threadCountForScratchPass);

			TextureSubresourceInfo subresource = TextureSubresourceInfo(DepthStencilAspectBit::DEPTH);
			pass.newDependency({m_scratchRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
		}

		// ESM pass
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("ESM");

			m_esmRt = rgraph.importRenderTarget(m_esmAtlas, TextureUsageBit::SAMPLED_FRAGMENT);
			pass.setFramebufferInfo(m_esmFbDescr, {{m_esmRt}}, {});
			pass.setWork(runEsmCallback, this, 0);

			pass.newDependency(
				{m_scratchRt, TextureUsageBit::SAMPLED_FRAGMENT, TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});
			pass.newDependency({m_esmRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		}
	}
	else
	{
		// No need for shadowmapping passes, just import the ESM atlas
		m_esmRt = rgraph.importRenderTarget(m_esmAtlas, TextureUsageBit::SAMPLED_FRAGMENT);
	}
}

Mat4 ShadowMapping::createSpotLightTextureMatrix(const Tile& tile)
{
	return Mat4(tile.m_uv[2],
		0.0,
		0.0,
		tile.m_uv[0],
		0.0,
		tile.m_uv[3],
		0.0,
		tile.m_uv[1],
		0.0,
		0.0,
		1.0,
		0.0,
		0.0,
		0.0,
		0.0,
		1.0);
}

void ShadowMapping::processLights(RenderingContext& ctx, U32& threadCountForScratchPass)
{
	// Reset stuff
	m_freeScratchTiles = m_scratchTileCount;

	// Vars
	DynamicArrayAuto<LightToRenderToScratchInfo> lightsToRender(ctx.m_tempAllocator);
	U32 drawcallCount = 0;
	DynamicArrayAuto<EsmResolveWorkItem> esmWorkItems(ctx.m_tempAllocator);

	// Process the point lights first.
	for(PointLightQueueElement* light : ctx.m_renderQueue->m_shadowPointLights)
	{
		// Prepare data to allocate tiles and allocate
		Array<U32, 6> tiles;
		Array<U32, 6> scratchTiles;
		Array<U64, 6> timestamps;
		Array<U32, 6> faceIndices;
		Array<U32, 6> drawcallCounts;
		U numOfFacesThatHaveDrawcalls = 0;
		for(U face = 0; face < 6; ++face)
		{
			ANKI_ASSERT(light->m_shadowRenderQueues[face]);
			if(light->m_shadowRenderQueues[face]->m_renderables.getSize())
			{
				// Has renderables, need to allocate tiles for it so add it to the arrays

				faceIndices[numOfFacesThatHaveDrawcalls] = face;
				timestamps[numOfFacesThatHaveDrawcalls] =
					light->m_shadowRenderQueues[face]->m_shadowRenderablesLastUpdateTimestamp;

				drawcallCounts[numOfFacesThatHaveDrawcalls] =
					light->m_shadowRenderQueues[face]->m_renderables.getSize();

				++numOfFacesThatHaveDrawcalls;
			}
		}
		const Bool allocationFailed = numOfFacesThatHaveDrawcalls == 0
									  || allocateTilesAndScratchTiles(light->m_uuid,
											 numOfFacesThatHaveDrawcalls,
											 &timestamps[0],
											 &faceIndices[0],
											 &drawcallCounts[0],
											 &tiles[0],
											 &scratchTiles[0]);

		if(!allocationFailed)
		{
			// All good, update the lights

			light->m_atlasTiles = UVec2(0u);
			light->m_atlasTileSize = 1.0f / m_tileCountPerRowOrColumn;

			numOfFacesThatHaveDrawcalls = 0;
			for(U face = 0; face < 6; ++face)
			{
				if(light->m_shadowRenderQueues[face]->m_renderables.getSize())
				{
					// Has drawcalls, asigned it to a tile

					const U32 tileIdx = tiles[numOfFacesThatHaveDrawcalls];
					const U32 tileIdxX = tileIdx % m_tileCountPerRowOrColumn;
					const U32 tileIdxY = tileIdx / m_tileCountPerRowOrColumn;
					ANKI_ASSERT(tileIdxX <= 31u && tileIdxY <= 31u);
					light->m_atlasTiles.x() |= tileIdxX << (5u * face);
					light->m_atlasTiles.y() |= tileIdxY << (5u * face);

					if(scratchTiles[numOfFacesThatHaveDrawcalls] != MAX_U32)
					{
						newScratchAndEsmResloveRenderWorkItems(tiles[numOfFacesThatHaveDrawcalls],
							scratchTiles[numOfFacesThatHaveDrawcalls],
							light->m_shadowRenderQueues[face],
							lightsToRender,
							esmWorkItems,
							drawcallCount);
					}

					++numOfFacesThatHaveDrawcalls;
				}
				else
				{
					// Doesn't have renderables, point the face to the 1st tile (that is pinned)
					light->m_atlasTiles.x() |= 0u << (5u * face);
					light->m_atlasTiles.y() |= 0u << (5u * face);
				}
			}
		}
		else
		{
			// Light can't be a caster this frame
			memset(&light->m_shadowRenderQueues[0], 0, sizeof(light->m_shadowRenderQueues));
		}
	}

	// Process the spot lights
	for(SpotLightQueueElement* light : ctx.m_renderQueue->m_shadowSpotLights)
	{
		ANKI_ASSERT(light->m_shadowRenderQueue);

		// Allocate tiles
		U32 tileIdx, scratchTileIdx, faceIdx = 0;
		const U32 localDrawcallCount = light->m_shadowRenderQueue->m_renderables.getSize();
		const Bool allocationFailed = localDrawcallCount == 0
									  || allocateTilesAndScratchTiles(light->m_uuid,
											 1,
											 &light->m_shadowRenderQueue->m_shadowRenderablesLastUpdateTimestamp,
											 &faceIdx,
											 &localDrawcallCount,
											 &tileIdx,
											 &scratchTileIdx);

		if(!allocationFailed)
		{
			// All good, update the light

			// Update the texture matrix to point to the correct region in the atlas
			light->m_textureMatrix = createSpotLightTextureMatrix(m_tiles[tileIdx]) * light->m_textureMatrix;

			if(scratchTileIdx != MAX_U32)
			{
				newScratchAndEsmResloveRenderWorkItems(
					tileIdx, scratchTileIdx, light->m_shadowRenderQueue, lightsToRender, esmWorkItems, drawcallCount);
			}
		}
		else
		{
			// Doesn't have renderables or the allocation failed, won't be a shadow caster
			light->m_shadowRenderQueue = nullptr;
		}
	}

	// Split the work that will happen in the scratch buffer
	if(lightsToRender.getSize())
	{
		DynamicArrayAuto<ScratchBufferWorkItem> workItems(ctx.m_tempAllocator);
		LightToRenderToScratchInfo* lightToRender = lightsToRender.getBegin();
		U lightToRenderDrawcallCount = lightToRender->m_drawcallCount;
		const LightToRenderToScratchInfo* lightToRenderEnd = lightsToRender.getEnd();

		const U threadCount = computeNumberOfSecondLevelCommandBuffers(drawcallCount);
		threadCountForScratchPass = threadCount;
		for(U taskId = 0; taskId < threadCount; ++taskId)
		{
			PtrSize start, end;
			splitThreadedProblem(taskId, threadCount, drawcallCount, start, end);

			// While there are drawcalls in this task emit new work items
			U taskDrawcallCount = end - start;
			ANKI_ASSERT(taskDrawcallCount > 0 && "Because we used computeNumberOfSecondLevelCommandBuffers()");

			while(taskDrawcallCount)
			{
				ANKI_ASSERT(lightToRender != lightToRenderEnd);
				const U workItemDrawcallCount = min(lightToRenderDrawcallCount, taskDrawcallCount);

				ScratchBufferWorkItem workItem;
				workItem.m_viewport = lightToRender->m_viewport;
				workItem.m_renderQueue = lightToRender->m_renderQueue;
				workItem.m_firstRenderableElement = lightToRender->m_drawcallCount - lightToRenderDrawcallCount;
				workItem.m_renderableElementCount = workItemDrawcallCount;
				workItem.m_threadPoolTaskIdx = taskId;
				workItems.emplaceBack(workItem);

				// Decrease the drawcall counts for the task and the light
				ANKI_ASSERT(taskDrawcallCount >= workItemDrawcallCount);
				taskDrawcallCount -= workItemDrawcallCount;
				ANKI_ASSERT(lightToRenderDrawcallCount >= workItemDrawcallCount);
				lightToRenderDrawcallCount -= workItemDrawcallCount;

				// Move to the next light
				if(lightToRenderDrawcallCount == 0)
				{
					++lightToRender;
					lightToRenderDrawcallCount =
						(lightToRender != lightToRenderEnd) ? lightToRender->m_drawcallCount : 0;
				}
			}
		}

		ANKI_ASSERT(lightToRender == lightToRenderEnd);
		ANKI_ASSERT(lightsToRender.getSize() <= workItems.getSize());

		// All good, store the work items for the threads to pick up
		{
			ScratchBufferWorkItem* items;
			PtrSize itemSize;
			PtrSize itemStorageSize;
			workItems.moveAndReset(items, itemSize, itemStorageSize);

			ANKI_ASSERT(items && itemSize && itemStorageSize);
			m_scratchWorkItems = WeakArray<ScratchBufferWorkItem>(items, itemSize);

			EsmResolveWorkItem* esmItems;
			esmWorkItems.moveAndReset(esmItems, itemSize, itemStorageSize);
			ANKI_ASSERT(esmItems && itemSize && itemStorageSize);
			m_esmResolveWorkItems = WeakArray<EsmResolveWorkItem>(esmItems, itemSize);
		}
	}
	else
	{
		m_scratchWorkItems = WeakArray<ScratchBufferWorkItem>();
		m_esmResolveWorkItems = WeakArray<EsmResolveWorkItem>();
	}
}

void ShadowMapping::newScratchAndEsmResloveRenderWorkItems(U32 tileIdx,
	U32 scratchTileIdx,
	RenderQueue* lightRenderQueue,
	DynamicArrayAuto<LightToRenderToScratchInfo>& scratchWorkItem,
	DynamicArrayAuto<EsmResolveWorkItem>& esmResolveWorkItem,
	U32& drawcallCount) const
{
	// Scratch work item
	{
		Array<U32, 4> viewport;
		viewport[0] = scratchTileIdx * m_scratchTileResolution;
		viewport[1] = 0;
		viewport[2] = m_scratchTileResolution;
		viewport[3] = m_scratchTileResolution;

		LightToRenderToScratchInfo toRender = {
			viewport, lightRenderQueue, U32(lightRenderQueue->m_renderables.getSize())};
		scratchWorkItem.emplaceBack(toRender);
		drawcallCount += lightRenderQueue->m_renderables.getSize();
	}

	// ESM resolve work item
	{
		EsmResolveWorkItem esmItem;
		esmItem.m_uvIn[0] = F32(scratchTileIdx) / m_scratchTileCount;
		esmItem.m_uvIn[1] = 0.0f;
		esmItem.m_uvIn[2] = 1.0f / m_scratchTileCount;
		esmItem.m_uvIn[3] = 1.0f;

		esmItem.m_viewportOut = m_tiles[tileIdx].m_viewport;

		esmItem.m_cameraFar = lightRenderQueue->m_cameraFar;
		esmItem.m_cameraNear = lightRenderQueue->m_cameraNear;

		esmResolveWorkItem.emplaceBack(esmItem);
	}
}

Bool ShadowMapping::allocateTilesAndScratchTiles(U64 lightUuid,
	U32 faceCount,
	const U64* faceTimestamps,
	const U32* faceIndices,
	const U32* drawcallsCount,
	U32* tileIndices,
	U32* scratchTileIndices)
{
	ANKI_ASSERT(faceTimestamps);
	ANKI_ASSERT(lightUuid > 0);
	ANKI_ASSERT(faceCount > 0 && faceCount <= 6);
	ANKI_ASSERT(faceIndices && tileIndices && scratchTileIndices && drawcallsCount);

	Bool failed = false;
	Array<Bool, 6> inTheCache;

	// Allocate ESM tiles
	{
		memset(tileIndices, 0xFF, sizeof(*tileIndices) * faceCount);
		for(U i = 0; i < faceCount && !failed; ++i)
		{
			failed = allocateTile(faceTimestamps[i], lightUuid, faceIndices[i], tileIndices[i], inTheCache[i]);
		}

		// Unpin the tiles
		for(U i = 0; i < faceCount; ++i)
		{
			if(tileIndices[i] != MAX_U32)
			{
				m_tiles[tileIndices[i]].m_pinned = false;
			}
		}
	}

	// Allocate scratch tiles
	{
		U32 freeScratchTiles = m_freeScratchTiles;
		for(U i = 0; i < faceCount && !failed; ++i)
		{
			scratchTileIndices[i] = MAX_U32;
			const Bool shouldRender = shouldRenderTile(
				faceTimestamps[i], lightUuid, faceIndices[i], m_tiles[tileIndices[i]], drawcallsCount[i]);
			const Bool scratchTileFailed = shouldRender && freeScratchTiles == 0;

			if(scratchTileFailed)
			{
				ANKI_R_LOGW("Don't have enough space in the scratch shadow mapping buffer. "
							"If you see this message too often increase r.shadowMapping.scratchTileCount");
				failed = true;
			}
			else if(shouldRender)
			{
				ANKI_ASSERT(m_scratchTileCount >= freeScratchTiles);
				scratchTileIndices[i] = m_scratchTileCount - freeScratchTiles;
				--freeScratchTiles;
			}
		}

		if(!failed)
		{
			m_freeScratchTiles = freeScratchTiles;
		}
	}

	// Update the tiles if everything was successful
	if(!failed)
	{
		for(U i = 0; i < faceCount; ++i)
		{
			Tile& tile = m_tiles[tileIndices[i]];
			tile.m_face = faceIndices[i];
			tile.m_lightUuid = lightUuid;
			tile.m_lastUsedTimestamp = m_r->getGlobalTimestamp();
			tile.m_drawcallCount = drawcallsCount[i];

			// Update the cache
			if(!inTheCache[i])
			{
				TileKey key{lightUuid, faceIndices[i]};
				ANKI_ASSERT(m_lightUuidToTileIdx.find(key) == m_lightUuidToTileIdx.getEnd());
				m_lightUuidToTileIdx.emplace(getAllocator(), key, tileIndices[i]);
			}
		}
	}

	return failed;
}

Bool ShadowMapping::shouldRenderTile(
	U64 lightTimestamp, U64 lightUuid, U32 face, const Tile& tileIdx, U32 drawcallCount)
{
	if(tileIdx.m_face == face && tileIdx.m_lightUuid == lightUuid && tileIdx.m_lastUsedTimestamp >= lightTimestamp
		&& tileIdx.m_drawcallCount == drawcallCount)
	{
		return false;
	}
	else
	{
		return true;
	}
}

Bool ShadowMapping::allocateTile(U64 lightTimestamp, U64 lightUuid, U32 face, U32& tileAllocated, Bool& inTheCache)
{
	ANKI_ASSERT(lightTimestamp > 0);
	ANKI_ASSERT(lightUuid > 0);
	ANKI_ASSERT(face < 6);

	// First, try to see if the light/face is in the cache
	inTheCache = false;
	TileKey key = TileKey{lightUuid, U64(face)};
	auto it = m_lightUuidToTileIdx.find(key);
	if(it != m_lightUuidToTileIdx.getEnd())
	{
		const U32 tileIdx = *it;
		if(m_tiles[tileIdx].m_lightUuid == lightUuid && m_tiles[tileIdx].m_face == face && !m_tiles[tileIdx].m_pinned)
		{
			// Found it
			tileAllocated = tileIdx;
			inTheCache = true;
			return false;
		}
		else
		{
			// Cache entry is wrong, remove it
			m_lightUuidToTileIdx.erase(getAllocator(), it);
		}
	}

	// 2nd and 3rd choice, find an empty tile or some tile to re-use
	U32 emptyTile = MAX_U32;
	U32 tileToKick = MAX_U32;
	Timestamp tileToKickMinTimestamp = MAX_TIMESTAMP;
	for(U32 tileIdx = 0; tileIdx < m_tiles.getSize(); ++tileIdx)
	{
		if(m_tiles[tileIdx].m_pinned)
		{
			continue;
		}

		if(m_tiles[tileIdx].m_lightUuid == 0)
		{
			// Found an empty
			emptyTile = tileIdx;
			break;
		}
		else if(m_tiles[tileIdx].m_lastUsedTimestamp != m_r->getGlobalTimestamp()
				&& m_tiles[tileIdx].m_lastUsedTimestamp < tileToKickMinTimestamp)
		{
			// Found some with low timestamp
			tileToKick = tileIdx;
			tileToKickMinTimestamp = m_tiles[tileIdx].m_lastUsedTimestamp;
		}
	}

	Bool failed = false;
	if(emptyTile != MAX_U32)
	{
		tileAllocated = emptyTile;
	}
	else if(tileToKick != MAX_U32)
	{
		tileAllocated = tileToKick;
	}
	else
	{
		// We have a problem
		failed = true;
		ANKI_R_LOGW("There is not enough space in the shadow atlas for more shadow maps. "
					"Increase the r.shadowMapping.tileCountPerRowOrColumn or decrease the scene's shadow casters");
	}

	if(!failed)
	{
		ANKI_ASSERT(!m_tiles[tileAllocated].m_pinned);
		m_tiles[tileAllocated].m_pinned = true;
	}

	return failed;
}

} // end namespace anki
