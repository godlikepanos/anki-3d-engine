// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ir.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Pps.h>
#include <anki/core/Config.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/Visibility.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/ReflectionProbeComponent.h>
#include <anki/core/Trace.h>

namespace anki
{

//==============================================================================
Ir::Ir(Renderer* r)
	: RenderingPass(r)
{
}

//==============================================================================
Ir::~Ir()
{
	m_cacheEntries.destroy(getAllocator());
}

//==============================================================================
Error Ir::init(const ConfigSet& config)
{
	ANKI_LOGI("Initializing IR (Image Reflections)");
	m_fbSize = config.getNumber("ir.rendererSize");

	if(m_fbSize < TILE_SIZE)
	{
		ANKI_LOGE("Too low ir.rendererSize");
		return ErrorCode::USER_DATA;
	}

	m_cubemapArrSize = config.getNumber("ir.cubemapTextureArraySize");

	if(m_cubemapArrSize < 2)
	{
		ANKI_LOGE("Too low ir.cubemapTextureArraySize");
		return ErrorCode::USER_DATA;
	}

	m_cacheEntries.create(getAllocator(), m_cubemapArrSize);

	// Init the renderer
	Config nestedRConfig;
	nestedRConfig.set("dbg.enabled", false);
	nestedRConfig.set("is.groundLightEnabled", false);
	nestedRConfig.set("sm.enabled", false);
	nestedRConfig.set("lf.maxFlares", 8);
	nestedRConfig.set("pps.enabled", false);
	nestedRConfig.set("renderingQuality", 1.0);
	nestedRConfig.set("clusterSizeZ", 16);
	nestedRConfig.set("width", m_fbSize);
	nestedRConfig.set("height", m_fbSize);
	nestedRConfig.set("lodDistance", 10.0);
	nestedRConfig.set("samples", 1);
	nestedRConfig.set("ir.enabled", false); // Very important to disable that
	nestedRConfig.set("sslr.enabled", false); // And that

	ANKI_CHECK(m_nestedR.init(&m_r->getThreadPool(),
		&m_r->getResourceManager(),
		&m_r->getGrManager(),
		m_r->getAllocator(),
		m_r->getFrameAllocator(),
		nestedRConfig,
		m_r->getGlobalTimestampPtr()));

	// Init the textures
	TextureInitInfo texinit;

	texinit.m_width = m_fbSize;
	texinit.m_height = m_fbSize;
	texinit.m_depth = m_cubemapArrSize;
	texinit.m_type = TextureType::CUBE_ARRAY;
	texinit.m_format = Is::RT_PIXEL_FORMAT;
	texinit.m_mipmapsCount = MAX_U8;
	texinit.m_samples = 1;
	texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	texinit.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;

	m_envCubemapArr = getGrManager().newInstance<Texture>(texinit);

	texinit.m_width = IRRADIANCE_TEX_SIZE;
	texinit.m_height = IRRADIANCE_TEX_SIZE;
	m_irradianceCubemapArr = getGrManager().newInstance<Texture>(texinit);

	m_cubemapArrMipCount = computeMaxMipmapCount(m_fbSize, m_fbSize);

	// Create irradiance stuff
	ANKI_CHECK(initIrradiance());

	// Clear the textures
	CommandBufferInitInfo cinf;
	CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>(cinf);
	U irrMipCount =
		computeMaxMipmapCount(IRRADIANCE_TEX_SIZE, IRRADIANCE_TEX_SIZE);
	ClearValue clear;
	for(U i = 0; i < m_cubemapArrSize; ++i)
	{
		for(U f = 0; f < 6; ++f)
		{
			for(U l = 0; l < m_cubemapArrMipCount; ++l)
			{
				// Do env
				cmdb->clearTexture(
					m_envCubemapArr, TextureSurfaceInfo(l, i, f), clear);
			}

			for(U l = 0; l < irrMipCount; ++l)
			{
				cmdb->clearTexture(
					m_irradianceCubemapArr, TextureSurfaceInfo(l, i, f), clear);
			}
		}
	}
	cmdb->flush();

	// Load split sum integration LUT
	ANKI_CHECK(getResourceManager().loadResource(
		"engine_data/SplitSumIntegration.ankitex", m_integrationLut));

	SamplerInitInfo sinit;
	sinit.m_minMagFilter = SamplingFilter::LINEAR;
	sinit.m_mipmapFilter = SamplingFilter::BASE;
	sinit.m_minLod = 0.0;
	sinit.m_maxLod = 1.0;
	sinit.m_repeat = false;
	m_integrationLutSampler = getGrManager().newInstance<Sampler>(sinit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::initIrradiance()
{
	// Create the shader
	StringAuto pps(getFrameAllocator());
	pps.sprintf("#define CUBEMAP_SIZE %u\n", IRRADIANCE_TEX_SIZE);

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_computeIrradianceFrag,
		"shaders/Irradiance.frag.glsl",
		pps.toCString(),
		"r_ir_"));

	// Create the ppline
	ColorStateInfo colorInf;
	colorInf.m_attachmentCount = 1;
	colorInf.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;

	m_r->createDrawQuadPipeline(m_computeIrradianceFrag->getGrShader(),
		colorInf,
		m_computeIrradiancePpline);

	// Create the resources
	ResourceGroupInitInfo rcInit;
	rcInit.m_uniformBuffers[0].m_dynamic = true;
	rcInit.m_textures[0].m_texture = m_envCubemapArr;

	m_computeIrradianceResources =
		getGrManager().newInstance<ResourceGroup>(rcInit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::run(RenderingContext& rctx)
{
	ANKI_TRACE_START_EVENT(RENDER_IR);
	FrustumComponent& frc = *rctx.m_frustumComponent;
	VisibilityTestResults& visRez = frc.getVisibilityTestResults();

	if(visRez.getCount(VisibilityGroupType::REFLECTION_PROBES)
		> m_cubemapArrSize)
	{
		ANKI_LOGW("Increase the ir.cubemapTextureArraySize");
	}

	// Render some of the probes
	const VisibleNode* it =
		visRez.getBegin(VisibilityGroupType::REFLECTION_PROBES);
	const VisibleNode* end =
		visRez.getEnd(VisibilityGroupType::REFLECTION_PROBES);

	U probesRendered = 0;
	U probeIdx = 0;
	while(it != end)
	{
		// Write and render probe
		ANKI_CHECK(tryRender(rctx, *it->m_node, probesRendered));

		++it;
		++probeIdx;
	}
	ANKI_ASSERT(
		probeIdx == visRez.getCount(VisibilityGroupType::REFLECTION_PROBES));

	// Bye
	ANKI_TRACE_STOP_EVENT(RENDER_IR);
	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::tryRender(RenderingContext& ctx, SceneNode& node, U& probesRendered)
{
	ReflectionProbeComponent& reflc =
		node.getComponent<ReflectionProbeComponent>();

	Bool render = false;
	U entry;
	findCacheEntry(node, entry, render);

	// Write shader var
	reflc.setTextureArrayIndex(entry);

	if(reflc.getMarkedForRendering()
		&& probesRendered < MAX_PROBE_RENDERS_PER_FRAME)
	{
		++probesRendered;
		reflc.setMarkedForRendering(false);
		ANKI_CHECK(renderReflection(ctx, node, reflc, entry));
	}

	// If you need to render it mark it for the next frame
	if(render)
	{
		reflc.setMarkedForRendering(true);
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::renderReflection(RenderingContext& ctx,
	SceneNode& node,
	ReflectionProbeComponent& reflc,
	U cubemapIdx)
{
	ANKI_TRACE_INC_COUNTER(RENDERER_REFLECTIONS, 1);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Get the frustum components
	Array<FrustumComponent*, 6> frustumComponents;
	U count = 0;
	Error err = node.iterateComponentsOfType<FrustumComponent>(
		[&](FrustumComponent& frc) -> Error {
			frustumComponents[count++] = &frc;
			return ErrorCode::NONE;
		});
	(void)err;
	ANKI_ASSERT(count == 6);

	// Render cubemap
	for(U i = 0; i < 6; ++i)
	{
		// Render
		RenderingContext nestedCtx(ctx.m_tempAllocator);
		nestedCtx.m_frustumComponent = frustumComponents[i];
		nestedCtx.m_commandBuffer = cmdb;

		ANKI_CHECK(m_nestedR.render(nestedCtx));

		// Copy env texture
		cmdb->copyTextureToTexture(m_nestedR.getIs().getRt(),
			TextureSurfaceInfo(0, 0, 0),
			m_envCubemapArr,
			TextureSurfaceInfo(0, cubemapIdx, i));

		// Gen mips of env tex
		cmdb->generateMipmaps(m_envCubemapArr, cubemapIdx, i);
	}

	// Compute irradiance
	cmdb->setViewport(0, 0, IRRADIANCE_TEX_SIZE, IRRADIANCE_TEX_SIZE);
	for(U i = 0; i < 6; ++i)
	{
		DynamicBufferInfo dinf;
		UVec4* faceIdxArrayIdx =
			static_cast<UVec4*>(getGrManager().allocateFrameHostVisibleMemory(
				sizeof(UVec4), BufferUsage::UNIFORM, dinf.m_uniformBuffers[0]));
		faceIdxArrayIdx->x() = i;
		faceIdxArrayIdx->y() = cubemapIdx;

		cmdb->bindResourceGroup(m_computeIrradianceResources, 0, &dinf);

		FramebufferInitInfo fbinit;
		fbinit.m_colorAttachmentsCount = 1;
		fbinit.m_colorAttachments[0].m_texture = m_irradianceCubemapArr;
		fbinit.m_colorAttachments[0].m_arrayIndex = cubemapIdx;
		fbinit.m_colorAttachments[0].m_faceIndex = i;
		fbinit.m_colorAttachments[0].m_format = Is::RT_PIXEL_FORMAT;
		fbinit.m_colorAttachments[0].m_loadOperation =
			AttachmentLoadOperation::DONT_CARE;
		FramebufferPtr fb = getGrManager().newInstance<Framebuffer>(fbinit);
		cmdb->beginRenderPass(fb);

		cmdb->bindPipeline(m_computeIrradiancePpline);
		m_r->drawQuad(cmdb);
		cmdb->endRenderPass();

		cmdb->generateMipmaps(m_irradianceCubemapArr, cubemapIdx, i);
	}

	return ErrorCode::NONE;
}

//==============================================================================
void Ir::findCacheEntry(SceneNode& node, U& entry, Bool& render)
{
	CacheEntry* it = m_cacheEntries.getBegin();
	const CacheEntry* const end = m_cacheEntries.getEnd();

	CacheEntry* canditate = nullptr;
	CacheEntry* empty = nullptr;
	CacheEntry* kick = nullptr;
	Timestamp kickTime = MAX_TIMESTAMP;

	while(it != end)
	{
		if(it->m_node == &node)
		{
			// Already there
			canditate = it;
			break;
		}
		else if(empty == nullptr && it->m_node == nullptr)
		{
			// Found empty
			empty = it;
		}
		else if(it->m_timestamp < kickTime)
		{
			// Found one to kick
			kick = it;
			kickTime = it->m_timestamp;
		}

		++it;
	}

	if(canditate)
	{
		// Update timestamp
		canditate->m_timestamp = m_r->getFrameCount();
		it = canditate;
		render = false;
	}
	else if(empty)
	{
		ANKI_ASSERT(empty->m_node == nullptr);
		empty->m_node = &node;
		empty->m_timestamp = m_r->getFrameCount();

		it = empty;
		render = true;
	}
	else if(kick)
	{
		kick->m_node = &node;
		kick->m_timestamp = m_r->getFrameCount();

		it = kick;
		render = true;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	entry = it - m_cacheEntries.getBegin();
}

} // end namespace anki
