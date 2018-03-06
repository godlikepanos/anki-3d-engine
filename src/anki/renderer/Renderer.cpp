// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/core/Trace.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/HighRezTimer.h>

#include <anki/renderer/Indirect.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/GBufferPost.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/FinalComposite.h>
#include <anki/renderer/Ssao.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/Tonemapping.h>
#include <anki/renderer/ForwardShading.h>
#include <anki/renderer/LensFlare.h>
#include <anki/renderer/Dbg.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/Volumetric.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/TemporalAA.h>
#include <anki/renderer/Reflections.h>
#include <anki/renderer/UiStage.h>

namespace anki
{

Renderer::Renderer()
	: m_sceneDrawer(this)
{
}

Renderer::~Renderer()
{
}

Error Renderer::init(ThreadPool* threadpool,
	ResourceManager* resources,
	GrManager* gl,
	StagingGpuMemoryManager* stagingMem,
	UiManager* ui,
	HeapAllocator<U8> alloc,
	const ConfigSet& config,
	Timestamp* globTimestamp,
	Bool willDrawToDefaultFbo)
{
	ANKI_TRACE_SCOPED_EVENT(RENDERER_INIT);

	m_globTimestamp = globTimestamp;
	m_threadpool = threadpool;
	m_resources = resources;
	m_gr = gl;
	m_stagingMem = stagingMem;
	m_ui = ui;
	m_alloc = alloc;
	m_willDrawToDefaultFbo = willDrawToDefaultFbo;

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize the renderer");
	}

	return err;
}

Error Renderer::initInternal(const ConfigSet& config)
{
	// Set from the config
	m_width = config.getNumber("width");
	m_height = config.getNumber("height");
	ANKI_R_LOGI("Initializing offscreen renderer. Size %ux%u", m_width, m_height);

	ANKI_ASSERT(m_lodDistances.getSize() == 2);
	m_lodDistances[0] = config.getNumber("r.lodDistance0");
	m_lodDistances[1] = config.getNumber("r.lodDistance1");
	m_frameCount = 0;

	m_tessellation = config.getNumber("tessellation");

	// A few sanity checks
	if(m_width < 10 || m_height < 10)
	{
		ANKI_R_LOGE("Incorrect sizes");
		return Error::USER_DATA;
	}

	{
		TextureInitInfo texinit;
		texinit.m_width = texinit.m_height = 4;
		texinit.m_usage = TextureUsageBit::SAMPLED_FRAGMENT;
		texinit.m_format = Format::R8G8B8A8_UNORM;
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		TexturePtr tex = getGrManager().newTexture(texinit);

		TextureViewInitInfo viewinit(tex);
		m_dummyTexView = getGrManager().newTextureView(viewinit);
	}

	m_dummyBuff = getGrManager().newBuffer(BufferInitInfo(
		1024, BufferUsageBit::UNIFORM_ALL | BufferUsageBit::STORAGE_ALL, BufferMapAccessBit::NONE, "Dummy"));

	// Init the stages. Careful with the order!!!!!!!!!!
	m_indirect.reset(m_alloc.newInstance<Indirect>(this));
	ANKI_CHECK(m_indirect->init(config));

	m_gbuffer.reset(m_alloc.newInstance<GBuffer>(this));
	ANKI_CHECK(m_gbuffer->init(config));

	m_gbufferPost.reset(m_alloc.newInstance<GBufferPost>(this));
	ANKI_CHECK(m_gbufferPost->init(config));

	m_shadowMapping.reset(m_alloc.newInstance<ShadowMapping>(this));
	ANKI_CHECK(m_shadowMapping->init(config));

	m_lightShading.reset(m_alloc.newInstance<LightShading>(this));
	ANKI_CHECK(m_lightShading->init(config));

	m_depth.reset(m_alloc.newInstance<DepthDownscale>(this));
	ANKI_CHECK(m_depth->init(config));

	m_vol.reset(m_alloc.newInstance<Volumetric>(this));
	ANKI_CHECK(m_vol->init(config));

	m_forwardShading.reset(m_alloc.newInstance<ForwardShading>(this));
	ANKI_CHECK(m_forwardShading->init(config));

	m_lensFlare.reset(m_alloc.newInstance<LensFlare>(this));
	ANKI_CHECK(m_lensFlare->init(config));

	m_ssao.reset(m_alloc.newInstance<Ssao>(this));
	ANKI_CHECK(m_ssao->init(config));

	m_downscale.reset(getAllocator().newInstance<DownscaleBlur>(this));
	ANKI_CHECK(m_downscale->init(config));

	m_refl.reset(m_alloc.newInstance<Reflections>(this));
	ANKI_CHECK(m_refl->init(config));

	m_tonemapping.reset(getAllocator().newInstance<Tonemapping>(this));
	ANKI_CHECK(m_tonemapping->init(config));

	m_temporalAA.reset(getAllocator().newInstance<TemporalAA>(this));
	ANKI_CHECK(m_temporalAA->init(config));

	m_bloom.reset(m_alloc.newInstance<Bloom>(this));
	ANKI_CHECK(m_bloom->init(config));

	m_finalComposite.reset(m_alloc.newInstance<FinalComposite>(this));
	ANKI_CHECK(m_finalComposite->init(config));

	m_dbg.reset(m_alloc.newInstance<Dbg>(this));
	ANKI_CHECK(m_dbg->init(config));

	m_uiStage.reset(m_alloc.newInstance<UiStage>(this));
	ANKI_CHECK(m_uiStage->init(config));

	SamplerInitInfo sinit("Renderer");
	sinit.m_repeat = false;
	sinit.m_mipmapFilter = SamplingFilter::BASE;
	sinit.m_minMagFilter = SamplingFilter::NEAREST;
	m_nearestSampler = m_gr->newSampler(sinit);

	sinit.m_minMagFilter = SamplingFilter::LINEAR;
	m_linearSampler = m_gr->newSampler(sinit);

	sinit.m_mipmapFilter = SamplingFilter::LINEAR;
	sinit.m_repeat = true;
	m_trilinearRepeatSampler = m_gr->newSampler(sinit);

	sinit.m_mipmapFilter = SamplingFilter::NEAREST;
	sinit.m_minMagFilter = SamplingFilter::NEAREST;
	m_nearesetNearestSampler = m_gr->newSampler(sinit);

	initJitteredMats();

	return Error::NONE;
}

void Renderer::initJitteredMats()
{
	static const Array<Vec2, 16> SAMPLE_LOCS_16 = {{Vec2(-8.0, 0.0),
		Vec2(-6.0, -4.0),
		Vec2(-3.0, -2.0),
		Vec2(-2.0, -6.0),
		Vec2(1.0, -1.0),
		Vec2(2.0, -5.0),
		Vec2(6.0, -7.0),
		Vec2(5.0, -3.0),
		Vec2(4.0, 1.0),
		Vec2(7.0, 4.0),
		Vec2(3.0, 5.0),
		Vec2(0.0, 7.0),
		Vec2(-1.0, 3.0),
		Vec2(-4.0, 6.0),
		Vec2(-7.0, 8.0),
		Vec2(-5.0, 2.0)}};

	for(U i = 0; i < 16; ++i)
	{
		Vec2 texSize(1.0f / Vec2(m_width, m_height)); // Texel size
		texSize *= 2.0f; // Move it to NDC

		Vec2 S = SAMPLE_LOCS_16[i] / 8.0f; // In [-1, 1]

		Vec2 subSample = S * texSize; // In [-texSize, texSize]
		subSample *= 0.5f; // In [-texSize / 2, texSize / 2]

		m_jitteredMats16x[i] = Mat4::getIdentity();
		m_jitteredMats16x[i].setTranslationPart(Vec4(subSample, 0.0, 1.0));
	}

	static const Array<Vec2, 8> SAMPLE_LOCS_8 = {{Vec2(-7.0, 1.0),
		Vec2(-5.0, -5.0),
		Vec2(-1.0, -3.0),
		Vec2(3.0, -7.0),
		Vec2(5.0, -1.0),
		Vec2(7.0, 7.0),
		Vec2(1.0, 3.0),
		Vec2(-3.0, 5.0)}};

	for(U i = 0; i < 8; ++i)
	{
		Vec2 texSize(1.0f / Vec2(m_width, m_height)); // Texel size
		texSize *= 2.0f; // Move it to NDC

		Vec2 S = SAMPLE_LOCS_8[i] / 8.0f; // In [-1, 1]

		Vec2 subSample = S * texSize; // In [-texSize, texSize]
		subSample *= 0.5f; // In [-texSize / 2, texSize / 2]

		m_jitteredMats8x[i] = Mat4::getIdentity();
		m_jitteredMats8x[i].setTranslationPart(Vec4(subSample, 0.0, 1.0));
	}
}

Error Renderer::populateRenderGraph(RenderingContext& ctx)
{
	ctx.m_jitterMat = m_jitteredMats8x[m_frameCount & (8 - 1)];
	ctx.m_projMatJitter = ctx.m_jitterMat * ctx.m_renderQueue->m_projectionMatrix;
	ctx.m_viewProjMatJitter = ctx.m_projMatJitter * ctx.m_renderQueue->m_viewMatrix;

	ctx.m_prevViewProjMat = m_prevViewProjMat;
	ctx.m_prevCamTransform = m_prevCamTransform;

	// Check if resources got loaded
	if(m_prevLoadRequestCount != m_resources->getLoadingRequestCount()
		|| m_prevAsyncTasksCompleted != m_resources->getAsyncTaskCompletedCount())
	{
		m_prevLoadRequestCount = m_resources->getLoadingRequestCount();
		m_prevAsyncTasksCompleted = m_resources->getAsyncTaskCompletedCount();
		m_resourcesDirty = true;
	}
	else
	{
		m_resourcesDirty = false;
	}

	// Import RTs first
	m_downscale->importRenderTargets(ctx);

	// Populate render graph. WARNING Watch the order
	m_shadowMapping->populateRenderGraph(ctx);
	m_indirect->populateRenderGraph(ctx);
	m_gbuffer->populateRenderGraph(ctx);
	m_depth->populateRenderGraph(ctx);
	m_vol->populateRenderGraph(ctx);
	m_ssao->populateRenderGraph(ctx);
	m_gbufferPost->populateRenderGraph(ctx);
	m_refl->populateRenderGraph(ctx);
	m_lensFlare->populateRenderGraph(ctx);
	m_forwardShading->populateRenderGraph(ctx);
	m_lightShading->populateRenderGraph(ctx);
	m_temporalAA->populateRenderGraph(ctx);
	m_downscale->populateRenderGraph(ctx);
	m_tonemapping->populateRenderGraph(ctx);
	m_bloom->populateRenderGraph(ctx);

	if(m_dbg->getEnabled())
	{
		m_dbg->populateRenderGraph(ctx);
	}

	m_finalComposite->populateRenderGraph(ctx);

	m_stats.m_lightBinTime = HighRezTimer::getCurrentTime();
	ANKI_CHECK(m_lightShading->binLights(ctx));
	m_stats.m_lightBinTime = HighRezTimer::getCurrentTime() - m_stats.m_lightBinTime;

	return Error::NONE;
}

void Renderer::finalize(const RenderingContext& ctx)
{
	++m_frameCount;
	m_prevViewProjMat = ctx.m_renderQueue->m_viewProjectionMatrix;
	m_prevCamTransform = ctx.m_renderQueue->m_cameraTransform;
}

Vec3 Renderer::unproject(
	const Vec3& windowCoords, const Mat4& modelViewMat, const Mat4& projectionMat, const int view[4])
{
	Mat4 invPm = projectionMat * modelViewMat;
	invPm.invert();

	// the vec is in NDC space meaning: -1<=vec.x<=1 -1<=vec.y<=1 -1<=vec.z<=1
	Vec4 vec;
	vec.x() = (2.0 * (windowCoords.x() - view[0])) / view[2] - 1.0;
	vec.y() = (2.0 * (windowCoords.y() - view[1])) / view[3] - 1.0;
	vec.z() = 2.0 * windowCoords.z() - 1.0;
	vec.w() = 1.0;

	Vec4 out = invPm * vec;
	out /= out.w();
	return out.xyz();
}

TextureInitInfo Renderer::create2DRenderTargetInitInfo(U32 w, U32 h, Format format, TextureUsageBit usage, CString name)
{
	ANKI_ASSERT(!!(usage & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE));
	TextureInitInfo init(name);

	init.m_width = w;
	init.m_height = h;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_type = TextureType::_2D;
	init.m_format = format;
	init.m_mipmapCount = 1;
	init.m_samples = 1;
	init.m_usage = usage;

	return init;
}

RenderTargetDescription Renderer::create2DRenderTargetDescription(
	U32 w, U32 h, Format format, TextureUsageBit usage, CString name)
{
	RenderTargetDescription init(name);

	init.m_width = w;
	init.m_height = h;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_type = TextureType::_2D;
	init.m_format = format;
	init.m_mipmapCount = 1;
	init.m_samples = 1;
	init.m_usage = usage;

	return init;
}

TexturePtr Renderer::createAndClearRenderTarget(const TextureInitInfo& inf, const ClearValue& clearVal)
{
	ANKI_ASSERT(!!(inf.m_usage & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE));

	const U faceCount = (inf.m_type == TextureType::CUBE || inf.m_type == TextureType::CUBE_ARRAY) ? 6 : 1;

	// Create tex
	TexturePtr tex = m_gr->newTexture(inf);

	// Clear all surfaces
	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
	if((inf.m_mipmapCount * faceCount * inf.m_layerCount * 4) < COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS)
	{
		cmdbinit.m_flags |= CommandBufferFlag::SMALL_BATCH;
	}
	CommandBufferPtr cmdb = m_gr->newCommandBuffer(cmdbinit);

	for(U mip = 0; mip < inf.m_mipmapCount; ++mip)
	{
		for(U face = 0; face < faceCount; ++face)
		{
			for(U layer = 0; layer < inf.m_layerCount; ++layer)
			{
				TextureSurfaceInfo surf(mip, 0, face, layer);

				FramebufferInitInfo fbInit("RendererClearRT");
				Array<TextureUsageBit, MAX_COLOR_ATTACHMENTS> colUsage = {};
				TextureUsageBit dsUsage = TextureUsageBit::NONE;

				if(formatIsDepthStencil(inf.m_format))
				{
					DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE;
					if(formatIsDepth(inf.m_format))
					{
						aspect |= DepthStencilAspectBit::DEPTH;
					}

					if(formatIsStencil(inf.m_format))
					{
						aspect |= DepthStencilAspectBit::STENCIL;
					}

					TextureViewPtr view = getGrManager().newTextureView(TextureViewInitInfo(tex, surf, aspect));

					fbInit.m_depthStencilAttachment.m_textureView = view;
					fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
					fbInit.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::CLEAR;
					fbInit.m_depthStencilAttachment.m_clearValue = clearVal;

					dsUsage = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
				}
				else
				{
					TextureViewPtr view = getGrManager().newTextureView(TextureViewInitInfo(tex, surf));

					fbInit.m_colorAttachmentCount = 1;
					fbInit.m_colorAttachments[0].m_textureView = view;
					fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
					fbInit.m_colorAttachments[0].m_clearValue = clearVal;

					colUsage[0] = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
				}
				FramebufferPtr fb = m_gr->newFramebuffer(fbInit);

				cmdb->setTextureSurfaceBarrier(
					tex, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf);

				cmdb->beginRenderPass(fb, colUsage, dsUsage);
				cmdb->endRenderPass();

				if(!!inf.m_initialUsage)
				{
					cmdb->setTextureSurfaceBarrier(
						tex, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, inf.m_initialUsage, surf);
				}
			}
		}
	}

	cmdb->flush();

	return tex;
}

} // end namespace anki
