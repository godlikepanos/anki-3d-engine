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

class ShadowMapping::ScratchBufferWorkItem
{
public:
	Array<U32, 4> m_viewport;
	RenderQueue* m_renderQueue;
	U32 m_firstRenderableElement;
	U32 m_renderableElementCount;
	U32 m_threadPoolTaskIdx;
};

class ShadowMapping::LightToRenderToScratchInfo
{
public:
	Array<U32, 4> m_viewport;
	RenderQueue* m_renderQueue;
	U32 m_drawcallCount;
};

class ShadowMapping::EsmResolveWorkItem
{
public:
	Vec4 m_uvIn; ///< UV + size that point to the scratch buffer.
	Array<U32, 4> m_viewportOut; ///< Viewport in the ESM RT.
	F32 m_cameraNear;
	F32 m_cameraFar;
	Bool8 m_blur;
	Bool8 m_perspectiveProjection;
};

ShadowMapping::~ShadowMapping()
{
}

Error ShadowMapping::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing shadowmapping");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize shadowmapping");
	}

	ANKI_R_LOGI("\tScratch size %ux%u. ESM atlas size %ux%u",
		m_scratchTileCountX * m_scratchTileResolution,
		m_scratchTileCountY * m_scratchTileResolution,
		m_esmTileCountBothAxis * m_esmTileResolution,
		m_esmTileCountBothAxis * m_esmTileResolution);

	return err;
}

Error ShadowMapping::initScratch(const ConfigSet& cfg)
{
	// Init the shadowmaps and FBs
	{
		m_scratchTileCountX = cfg.getNumber("r.shadowMapping.scratchTileCountX");
		m_scratchTileCountY = cfg.getNumber("r.shadowMapping.scratchTileCountY");
		m_scratchTileResolution = cfg.getNumber("r.shadowMapping.tileResolution");

		// RT
		m_scratchRtDescr = m_r->create2DRenderTargetDescription(m_scratchTileResolution * m_scratchTileCountX,
			m_scratchTileResolution * m_scratchTileCountY,
			SHADOW_DEPTH_PIXEL_FORMAT,
			"Scratch ShadMap");
		m_scratchRtDescr.bake();

		// FB
		m_scratchFbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
		m_scratchFbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;
		m_scratchFbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
		m_scratchFbDescr.bake();
	}

	m_scratchTileAlloc.init(getAllocator(), m_scratchTileCountX, m_scratchTileCountY, m_lodCount, false);

	return Error::NONE;
}

Error ShadowMapping::initEsm(const ConfigSet& cfg)
{
	// Init RTs and FBs
	{
		m_esmTileResolution = cfg.getNumber("r.shadowMapping.tileResolution");
		m_esmTileCountBothAxis = cfg.getNumber("r.shadowMapping.tileCountPerRowOrColumn");

		// RT
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(m_esmTileResolution * m_esmTileCountBothAxis,
			m_esmTileResolution * m_esmTileCountBothAxis,
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
	m_esmTileAlloc.init(getAllocator(), m_esmTileCountBothAxis, m_esmTileCountBothAxis, m_lodCount, true);

	// Programs and shaders
	{
		ANKI_CHECK(
			getResourceManager().loadResource("shaders/ExponentialShadowmappingResolve.glslp", m_esmResolveProg));

		ShaderProgramResourceConstantValueInitList<1> consts(m_esmResolveProg);
		consts.add("INPUT_TEXTURE_SIZE",
			UVec2(m_scratchTileCountX * m_scratchTileResolution, m_scratchTileCountY * m_scratchTileResolution));

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

	m_lodDistances[0] = cfg.getNumber("r.shadowMapping.lightLodDistance0");
	m_lodDistances[1] = cfg.getNumber("r.shadowMapping.lightLodDistance1");

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

		struct Uniforms
		{
			Vec2 m_uvScale;
			Vec2 m_uvTranslation;
			F32 m_near;
			F32 m_far;
			U32 m_renderingTechnique;
			U32 m_padding;
		} unis;
		unis.m_uvScale = workItem.m_uvIn.zw();
		unis.m_uvTranslation = workItem.m_uvIn.xy();
		unis.m_near = workItem.m_cameraNear;
		unis.m_far = workItem.m_cameraFar;

		if(workItem.m_perspectiveProjection)
		{
			unis.m_renderingTechnique = (workItem.m_blur) ? 0 : 1;
		}
		else
		{
			unis.m_renderingTechnique = (workItem.m_blur) ? 2 : 3;
		}

		cmdb->setPushConstants(&unis, sizeof(unis));

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
			const U32 minx = 0, miny = 0;
			const U32 height = m_scratchMaxViewportHeight;
			const U32 width = m_scratchMaxViewportWidth;

			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("SM scratch");

			m_scratchRt = rgraph.newRenderTarget(m_scratchRtDescr);
			pass.setFramebufferInfo(m_scratchFbDescr, {}, m_scratchRt, minx, miny, width, height);
			ANKI_ASSERT(
				threadCountForScratchPass && threadCountForScratchPass <= m_r->getThreadHive().getThreadCount());
			pass.setWork(
				[](RenderPassWorkContext& rgraphCtx) {
					static_cast<ShadowMapping*>(rgraphCtx.m_userData)->runShadowMapping(rgraphCtx);
				},
				this,
				threadCountForScratchPass);

			TextureSubresourceInfo subresource = TextureSubresourceInfo(DepthStencilAspectBit::DEPTH);
			pass.newDependency({m_scratchRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
		}

		// ESM pass
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("ESM");

			m_esmRt = rgraph.importRenderTarget(m_esmAtlas, TextureUsageBit::SAMPLED_FRAGMENT);
			pass.setFramebufferInfo(m_esmFbDescr, {{m_esmRt}}, {});
			pass.setWork(
				[](RenderPassWorkContext& rgraphCtx) {
					static_cast<ShadowMapping*>(rgraphCtx.m_userData)->runEsm(rgraphCtx);
				},
				this,
				0);

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

Mat4 ShadowMapping::createSpotLightTextureMatrix(const Viewport& viewport) const
{
	const F32 atlasSize = m_esmTileResolution * m_esmTileCountBothAxis;
	const Vec2 uv(F32(viewport[0]) / atlasSize, F32(viewport[1]) / atlasSize);
	ANKI_ASSERT(uv >= Vec2(0.0f) && uv <= Vec2(1.0f));

	ANKI_ASSERT(viewport[2] == viewport[3]);
	const F32 sizeTextureSpace = F32(viewport[2]) / atlasSize;

	return Mat4(sizeTextureSpace,
		0.0f,
		0.0f,
		uv.x(),
		0.0f,
		sizeTextureSpace,
		0.0f,
		uv.y(),
		0.0f,
		0.0f,
		1.0f,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f);
}

U ShadowMapping::choseLod(const Vec4& cameraOrigin, const PointLightQueueElement& light, Bool& blurEsm) const
{
	const F32 distFromTheCamera = (cameraOrigin - light.m_worldPosition.xyz0()).getLength() - light.m_radius;
	if(distFromTheCamera < m_lodDistances[0])
	{
		ANKI_ASSERT(m_pointLightsMaxLod == 1);
		blurEsm = true;
		return 1;
	}
	else
	{
		blurEsm = false;
		return 0;
	}
}

U ShadowMapping::choseLod(const Vec4& cameraOrigin, const SpotLightQueueElement& light, Bool& blurEsm) const
{
	// Get some data
	const Vec4 coneOrigin = light.m_worldTransform.getTranslationPart().xyz0();
	const Vec4 coneDir = -light.m_worldTransform.getZAxis().xyz0();
	const F32 coneAngle = light.m_outerAngle;

	// Compute the distance from the camera to the light cone
	const Vec4 V = cameraOrigin - coneOrigin;
	const F32 VlenSq = V.dot(V);
	const F32 V1len = V.dot(coneDir);
	const F32 distFromTheCamera = cos(coneAngle) * sqrt(VlenSq - V1len * V1len) - V1len * sin(coneAngle);

	U lod;
	if(distFromTheCamera < m_lodDistances[0])
	{
		blurEsm = true;
		lod = 2;
	}
	else if(distFromTheCamera < m_lodDistances[1])
	{
		blurEsm = false;
		lod = 1;
	}
	else
	{
		blurEsm = false;
		lod = 0;
	}

	return lod;
}

TileAllocatorResult ShadowMapping::allocateTilesAndScratchTiles(U64 lightUuid,
	U32 faceCount,
	const U64* faceTimestamps,
	const U32* faceIndices,
	const U32* drawcallsCount,
	const U32* lods,
	Viewport* esmTileViewports,
	Viewport* scratchTileViewports,
	TileAllocatorResult* subResults)
{
	ANKI_ASSERT(lightUuid > 0);
	ANKI_ASSERT(faceCount > 0);
	ANKI_ASSERT(faceTimestamps);
	ANKI_ASSERT(faceIndices);
	ANKI_ASSERT(drawcallsCount);
	ANKI_ASSERT(lods);

	TileAllocatorResult res;

	// Allocate ESM tiles first. They may be cached and that will affect how many scratch tiles we'll need
	for(U i = 0; i < faceCount; ++i)
	{
		res = m_esmTileAlloc.allocate(m_r->getGlobalTimestamp(),
			faceTimestamps[i],
			lightUuid,
			faceIndices[i],
			drawcallsCount[i],
			lods[i],
			esmTileViewports[i]);

		if(res == TileAllocatorResult::ALLOCATION_FAILED)
		{
			ANKI_R_LOGW("There is not enough space in the shadow atlas for more shadow maps. "
						"Increase the r.shadowMapping.tileCountPerRowOrColumn or decrease the scene's shadow casters");

			// Invalidate cache entries for what we already allocated
			for(U j = 0; j < i; ++j)
			{
				m_esmTileAlloc.invalidateCache(lightUuid, faceIndices[j]);
			}

			return res;
		}

		subResults[i] = res;

		// Fix viewport
		esmTileViewports[i][0] *= m_esmTileResolution;
		esmTileViewports[i][1] *= m_esmTileResolution;
		esmTileViewports[i][2] *= m_esmTileResolution;
		esmTileViewports[i][3] *= m_esmTileResolution;
	}

	// Allocate scratch tiles
	for(U i = 0; i < faceCount; ++i)
	{
		if(subResults[i] == TileAllocatorResult::CACHED)
		{
			continue;
		}

		ANKI_ASSERT(subResults[i] == TileAllocatorResult::ALLOCATION_SUCCEEDED);

		res = m_scratchTileAlloc.allocate(m_r->getGlobalTimestamp(),
			faceTimestamps[i],
			lightUuid,
			faceIndices[i],
			drawcallsCount[i],
			lods[i],
			scratchTileViewports[i]);

		if(res == TileAllocatorResult::ALLOCATION_FAILED)
		{
			ANKI_R_LOGW("Don't have enough space in the scratch shadow mapping buffer. "
						"If you see this message too often increase r.shadowMapping.scratchTileCountX/Y");

			// Invalidate ESM tiles
			for(U j = 0; j < faceCount; ++j)
			{
				m_esmTileAlloc.invalidateCache(lightUuid, faceIndices[j]);
			}

			return res;
		}

		// Fix viewport
		scratchTileViewports[i][0] *= m_scratchTileResolution;
		scratchTileViewports[i][1] *= m_scratchTileResolution;
		scratchTileViewports[i][2] *= m_scratchTileResolution;
		scratchTileViewports[i][3] *= m_scratchTileResolution;

		// Update the max view width
		m_scratchMaxViewportWidth =
			max(m_scratchMaxViewportWidth, scratchTileViewports[i][0] + scratchTileViewports[i][2]);
		m_scratchMaxViewportHeight =
			max(m_scratchMaxViewportHeight, scratchTileViewports[i][1] + scratchTileViewports[i][3]);
	}

	return res;
}

void ShadowMapping::processLights(RenderingContext& ctx, U32& threadCountForScratchPass)
{
	// Reset the scratch viewport width
	m_scratchMaxViewportWidth = 0;
	m_scratchMaxViewportHeight = 0;

	// Vars
	const Vec4 cameraOrigin = ctx.m_renderQueue->m_cameraTransform.getTranslationPart().xyz0();
	DynamicArrayAuto<LightToRenderToScratchInfo> lightsToRender(ctx.m_tempAllocator);
	U32 drawcallCount = 0;
	DynamicArrayAuto<EsmResolveWorkItem> esmWorkItems(ctx.m_tempAllocator);

	// First thing, allocate an empty tile for empty faces of point lights
	Viewport emptyTileViewport;
	{
		const TileAllocatorResult res = m_esmTileAlloc.allocate(
			m_r->getGlobalTimestamp(), 1, MAX_U64, 0, 1, m_pointLightsMaxLod, emptyTileViewport);

		(void)res;
#if ANKI_ASSERTS_ENABLED
		static Bool firstRun = true;
		if(firstRun)
		{
			ANKI_ASSERT(res == TileAllocatorResult::ALLOCATION_SUCCEEDED);
			firstRun = false;
		}
		else
		{
			ANKI_ASSERT(res == TileAllocatorResult::CACHED);
		}
#endif
	}

	// Process the directional light first.
	if(ctx.m_renderQueue->m_directionalLight.m_shadowCascadeCount > 0)
	{
		DirectionalLightQueueElement& light = ctx.m_renderQueue->m_directionalLight;

		Array<U64, MAX_SHADOW_CASCADES> timestamps;
		Array<U32, MAX_SHADOW_CASCADES> cascadeIndices;
		Array<U32, MAX_SHADOW_CASCADES> drawcallCounts;
		Array<Viewport, MAX_SHADOW_CASCADES> esmViewports;
		Array<Viewport, MAX_SHADOW_CASCADES> scratchViewports;
		Array<TileAllocatorResult, MAX_SHADOW_CASCADES> subResults;
		Array<U32, MAX_SHADOW_CASCADES> lods;
		Array<Bool, MAX_SHADOW_CASCADES> blurEsms;

		for(U cascade = 0; cascade < light.m_shadowCascadeCount; ++cascade)
		{
			ANKI_ASSERT(light.m_shadowRenderQueues[cascade]);
			timestamps[cascade] = m_r->getGlobalTimestamp(); // This light is always updated
			cascadeIndices[cascade] = cascade;
			drawcallCounts[cascade] = 1; // Doesn't matter

			// Change the quality per cascade
			blurEsms[cascade] = (cascade <= 1);
			lods[cascade] = (cascade <= 1) ? (m_lodCount - 1) : (lods[0] - 1);
		}

		const Bool allocationFailed = allocateTilesAndScratchTiles(light.m_uuid,
										  light.m_shadowCascadeCount,
										  &timestamps[0],
										  &cascadeIndices[0],
										  &drawcallCounts[0],
										  &lods[0],
										  &esmViewports[0],
										  &scratchViewports[0],
										  &subResults[0])
									  == TileAllocatorResult::ALLOCATION_FAILED;

		if(!allocationFailed)
		{
			for(U cascade = 0; cascade < light.m_shadowCascadeCount; ++cascade)
			{
				// Update the texture matrix to point to the correct region in the atlas
				light.m_textureMatrices[cascade] =
					createSpotLightTextureMatrix(esmViewports[cascade]) * light.m_textureMatrices[cascade];

				// Push work
				newScratchAndEsmResloveRenderWorkItems(esmViewports[cascade],
					scratchViewports[cascade],
					blurEsms[cascade],
					false,
					light.m_shadowRenderQueues[cascade],
					lightsToRender,
					esmWorkItems,
					drawcallCount);
			}
		}
		else
		{
			// Light can't be a caster this frame
			light.m_shadowCascadeCount = 0;
			zeroMemory(light.m_shadowRenderQueues);
		}
	}

	// Process the point lights.
	for(PointLightQueueElement* light : ctx.m_renderQueue->m_shadowPointLights)
	{
		// Prepare data to allocate tiles and allocate
		Array<U64, 6> timestamps;
		Array<U32, 6> faceIndices;
		Array<U32, 6> drawcallCounts;
		Array<Viewport, 6> esmViewports;
		Array<Viewport, 6> scratchViewports;
		Array<TileAllocatorResult, 6> subResults;
		Array<U32, 6> lods;
		U numOfFacesThatHaveDrawcalls = 0;

		Bool blurEsm;
		const U lod = choseLod(cameraOrigin, *light, blurEsm);

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

				lods[numOfFacesThatHaveDrawcalls] = lod;

				++numOfFacesThatHaveDrawcalls;
			}
		}

		const Bool allocationFailed = numOfFacesThatHaveDrawcalls == 0
									  || allocateTilesAndScratchTiles(light->m_uuid,
											 numOfFacesThatHaveDrawcalls,
											 &timestamps[0],
											 &faceIndices[0],
											 &drawcallCounts[0],
											 &lods[0],
											 &esmViewports[0],
											 &scratchViewports[0],
											 &subResults[0])
											 == TileAllocatorResult::ALLOCATION_FAILED;

		if(!allocationFailed)
		{
			// All good, update the lights

			const F32 atlasResolution = F32(m_esmTileResolution * m_esmTileCountBothAxis);
			F32 superTileSize = esmViewports[0][2]; // Should be the same for all tiles and faces
			superTileSize -= 1.0f; // Remove 2 half texels to avoid bilinear filtering bleeding

			light->m_shadowAtlasTileSize = superTileSize / atlasResolution;

			numOfFacesThatHaveDrawcalls = 0;
			for(U face = 0; face < 6; ++face)
			{
				if(light->m_shadowRenderQueues[face]->m_renderables.getSize())
				{
					// Has drawcalls, asigned it to a tile

					const Viewport& esmViewport = esmViewports[numOfFacesThatHaveDrawcalls];
					const Viewport& scratchViewport = scratchViewports[numOfFacesThatHaveDrawcalls];

					// Add a half texel to the viewport's start to avoid bilinear filtering bleeding
					light->m_shadowAtlasTileOffsets[face].x() = (F32(esmViewport[0]) + 0.5f) / atlasResolution;
					light->m_shadowAtlasTileOffsets[face].y() = (F32(esmViewport[1]) + 0.5f) / atlasResolution;

					if(subResults[numOfFacesThatHaveDrawcalls] != TileAllocatorResult::CACHED)
					{
						newScratchAndEsmResloveRenderWorkItems(esmViewport,
							scratchViewport,
							blurEsm,
							true,
							light->m_shadowRenderQueues[face],
							lightsToRender,
							esmWorkItems,
							drawcallCount);
					}

					++numOfFacesThatHaveDrawcalls;
				}
				else
				{
					// Doesn't have renderables, point the face to the empty tile
					Viewport esmViewport = emptyTileViewport;
					ANKI_ASSERT(esmViewport[2] <= superTileSize && esmViewport[3] <= superTileSize);
					esmViewport[2] = superTileSize;
					esmViewport[3] = superTileSize;

					light->m_shadowAtlasTileOffsets[face].x() = (F32(esmViewport[0]) + 0.5f) / atlasResolution;
					light->m_shadowAtlasTileOffsets[face].y() = (F32(esmViewport[1]) + 0.5f) / atlasResolution;
				}
			}
		}
		else
		{
			// Light can't be a caster this frame
			zeroMemory(light->m_shadowRenderQueues);
		}
	}

	// Process the spot lights
	for(SpotLightQueueElement* light : ctx.m_renderQueue->m_shadowSpotLights)
	{
		ANKI_ASSERT(light->m_shadowRenderQueue);

		// Allocate tiles
		U32 faceIdx = 0;
		TileAllocatorResult subResult;
		Viewport esmViewport;
		Viewport scratchViewport;
		const U32 localDrawcallCount = light->m_shadowRenderQueue->m_renderables.getSize();

		Bool blurEsm;
		const U32 lod = choseLod(cameraOrigin, *light, blurEsm);
		const Bool allocationFailed = localDrawcallCount == 0
									  || allocateTilesAndScratchTiles(light->m_uuid,
											 1,
											 &light->m_shadowRenderQueue->m_shadowRenderablesLastUpdateTimestamp,
											 &faceIdx,
											 &localDrawcallCount,
											 &lod,
											 &esmViewport,
											 &scratchViewport,
											 &subResult)
											 == TileAllocatorResult::ALLOCATION_FAILED;

		if(!allocationFailed)
		{
			// All good, update the light

			// Update the texture matrix to point to the correct region in the atlas
			light->m_textureMatrix = createSpotLightTextureMatrix(esmViewport) * light->m_textureMatrix;

			if(subResult != TileAllocatorResult::CACHED)
			{
				newScratchAndEsmResloveRenderWorkItems(esmViewport,
					scratchViewport,
					blurEsm,
					true,
					light->m_shadowRenderQueue,
					lightsToRender,
					esmWorkItems,
					drawcallCount);
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

void ShadowMapping::newScratchAndEsmResloveRenderWorkItems(const Viewport& esmViewport,
	const Viewport& scratchVewport,
	Bool blurEsm,
	Bool perspectiveProjection,
	RenderQueue* lightRenderQueue,
	DynamicArrayAuto<LightToRenderToScratchInfo>& scratchWorkItem,
	DynamicArrayAuto<EsmResolveWorkItem>& esmResolveWorkItem,
	U32& drawcallCount) const
{
	// Scratch work item
	{
		LightToRenderToScratchInfo toRender = {
			scratchVewport, lightRenderQueue, U32(lightRenderQueue->m_renderables.getSize())};
		scratchWorkItem.emplaceBack(toRender);
		drawcallCount += lightRenderQueue->m_renderables.getSize();
	}

	// ESM resolve work item
	{
		const F32 scratchAtlasWidth = m_scratchTileCountX * m_scratchTileResolution;
		const F32 scratchAtlasHeight = m_scratchTileCountY * m_scratchTileResolution;

		EsmResolveWorkItem esmItem;
		esmItem.m_uvIn[0] = F32(scratchVewport[0]) / scratchAtlasWidth;
		esmItem.m_uvIn[1] = F32(scratchVewport[1]) / scratchAtlasHeight;
		esmItem.m_uvIn[2] = F32(scratchVewport[2]) / scratchAtlasWidth;
		esmItem.m_uvIn[3] = F32(scratchVewport[3]) / scratchAtlasHeight;

		esmItem.m_viewportOut = esmViewport;

		esmItem.m_cameraFar = lightRenderQueue->m_cameraFar;
		esmItem.m_cameraNear = lightRenderQueue->m_cameraNear;

		esmItem.m_blur = blurEsm;
		esmItem.m_perspectiveProjection = perspectiveProjection;

		esmResolveWorkItem.emplaceBack(esmItem);
	}
}

} // end namespace anki
