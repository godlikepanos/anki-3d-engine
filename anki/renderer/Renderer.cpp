// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/util/Tracer.h>
#include <anki/core/ConfigSet.h>
#include <anki/util/HighRezTimer.h>
#include <anki/collision/Aabb.h>
#include <anki/shaders/include/ClusteredShadingTypes.h>

#include <anki/renderer/ProbeReflections.h>
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
#include <anki/renderer/VolumetricFog.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/TemporalAA.h>
#include <anki/renderer/UiStage.h>
#include <anki/renderer/Ssr.h>
#include <anki/renderer/Ssgi.h>
#include <anki/renderer/VolumetricLightingAccumulation.h>
#include <anki/renderer/GlobalIllumination.h>
#include <anki/renderer/GenericCompute.h>
#include <anki/renderer/ShadowmapsResolve.h>
#include <anki/renderer/RtShadows.h>
#include <anki/renderer/AccelerationStructureBuilder.h>

namespace anki
{

Renderer::Renderer()
	: m_sceneDrawer(this)
{
}

Renderer::~Renderer()
{
	for(DebugRtInfo& info : m_debugRts)
	{
		info.m_rtName.destroy(getAllocator());
	}
	m_debugRts.destroy(getAllocator());
	m_currentDebugRtName.destroy(getAllocator());
}

Error Renderer::init(ThreadHive* hive, ResourceManager* resources, GrManager* gl, StagingGpuMemoryManager* stagingMem,
					 UiManager* ui, HeapAllocator<U8> alloc, const ConfigSet& config, Timestamp* globTimestamp)
{
	ANKI_TRACE_SCOPED_EVENT(R_INIT);

	m_globTimestamp = globTimestamp;
	m_threadHive = hive;
	m_resources = resources;
	m_gr = gl;
	m_stagingMem = stagingMem;
	m_ui = ui;
	m_alloc = alloc;

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
	m_width = config.getNumberU32("width");
	m_height = config.getNumberU32("height");
	ANKI_R_LOGI("Initializing offscreen renderer. Size %ux%u", m_width, m_height);

	ANKI_ASSERT(m_lodDistances.getSize() == 2);
	m_lodDistances[0] = config.getNumberF32("r_lodDistance0");
	m_lodDistances[1] = config.getNumberF32("r_lodDistance1");
	m_frameCount = 0;

	m_clusterCount[0] = config.getNumberU32("r_clusterSizeX");
	m_clusterCount[1] = config.getNumberU32("r_clusterSizeY");
	m_clusterCount[2] = config.getNumberU32("r_clusterSizeZ");
	m_clusterCount[3] = m_clusterCount[0] * m_clusterCount[1] * m_clusterCount[2];

	m_clusterBin.init(m_alloc, m_clusterCount[0], m_clusterCount[1], m_clusterCount[2], config);

	// A few sanity checks
	if(m_width < 10 || m_height < 10)
	{
		ANKI_R_LOGE("Incorrect sizes");
		return Error::USER_DATA;
	}

	{
		TextureInitInfo texinit;
		texinit.m_width = texinit.m_height = 4;
		texinit.m_usage = TextureUsageBit::ALL_SAMPLED;
		texinit.m_format = Format::R8G8B8A8_UNORM;
		texinit.m_initialUsage = TextureUsageBit::ALL_SAMPLED;
		TexturePtr tex = getGrManager().newTexture(texinit);

		TextureViewInitInfo viewinit(tex);
		m_dummyTexView2d = getGrManager().newTextureView(viewinit);

		texinit.m_depth = 4;
		texinit.m_type = TextureType::_3D;
		tex = getGrManager().newTexture(texinit);
		viewinit = TextureViewInitInfo(tex);
		m_dummyTexView3d = getGrManager().newTextureView(viewinit);
	}

	m_dummyBuff = getGrManager().newBuffer(BufferInitInfo(
		1024, BufferUsageBit::ALL_UNIFORM | BufferUsageBit::ALL_STORAGE, BufferMapAccessBit::NONE, "Dummy"));

	ANKI_CHECK(m_resources->loadResource("shaders/ClearTextureCompute.ankiprog", m_clearTexComputeProg));

	// Init the stages. Careful with the order!!!!!!!!!!
	m_genericCompute.reset(m_alloc.newInstance<GenericCompute>(this));
	ANKI_CHECK(m_genericCompute->init(config));

	m_volLighting.reset(m_alloc.newInstance<VolumetricLightingAccumulation>(this));
	ANKI_CHECK(m_volLighting->init(config));

	m_gi.reset(m_alloc.newInstance<GlobalIllumination>(this));
	ANKI_CHECK(m_gi->init(config));

	m_probeReflections.reset(m_alloc.newInstance<ProbeReflections>(this));
	ANKI_CHECK(m_probeReflections->init(config));

	m_gbuffer.reset(m_alloc.newInstance<GBuffer>(this));
	ANKI_CHECK(m_gbuffer->init(config));

	m_gbufferPost.reset(m_alloc.newInstance<GBufferPost>(this));
	ANKI_CHECK(m_gbufferPost->init(config));

	m_shadowMapping.reset(m_alloc.newInstance<ShadowMapping>(this));
	ANKI_CHECK(m_shadowMapping->init(config));

	m_volFog.reset(m_alloc.newInstance<VolumetricFog>(this));
	ANKI_CHECK(m_volFog->init(config));

	m_lightShading.reset(m_alloc.newInstance<LightShading>(this));
	ANKI_CHECK(m_lightShading->init(config));

	m_depth.reset(m_alloc.newInstance<DepthDownscale>(this));
	ANKI_CHECK(m_depth->init(config));

	m_forwardShading.reset(m_alloc.newInstance<ForwardShading>(this));
	ANKI_CHECK(m_forwardShading->init(config));

	m_lensFlare.reset(m_alloc.newInstance<LensFlare>(this));
	ANKI_CHECK(m_lensFlare->init(config));

	m_ssao.reset(m_alloc.newInstance<Ssao>(this));
	ANKI_CHECK(m_ssao->init(config));

	m_downscale.reset(getAllocator().newInstance<DownscaleBlur>(this));
	ANKI_CHECK(m_downscale->init(config));

	m_ssr.reset(m_alloc.newInstance<Ssr>(this));
	ANKI_CHECK(m_ssr->init(config));

	m_ssgi.reset(m_alloc.newInstance<Ssgi>(this));
	ANKI_CHECK(m_ssgi->init(config));

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

	if(getGrManager().getDeviceCapabilities().m_rayTracingEnabled && config.getBool("scene_rayTracedShadows"))
	{
		m_accelerationStructureBuilder.reset(m_alloc.newInstance<AccelerationStructureBuilder>(this));
		ANKI_CHECK(m_accelerationStructureBuilder->init(config));

		m_rtShadows.reset(m_alloc.newInstance<RtShadows>(this));
		ANKI_CHECK(m_rtShadows->init(config));
	}
	else
	{
		m_smResolve.reset(m_alloc.newInstance<ShadowmapsResolve>(this));
		ANKI_CHECK(m_smResolve->init(config));
	}

	// Init samplers
	{
		SamplerInitInfo sinit("Renderer");
		sinit.m_addressing = SamplingAddressing::CLAMP;
		sinit.m_mipmapFilter = SamplingFilter::NEAREST;
		sinit.m_minMagFilter = SamplingFilter::NEAREST;
		m_samplers.m_nearestNearestClamp = m_gr->newSampler(sinit);

		sinit.m_minMagFilter = SamplingFilter::LINEAR;
		sinit.m_mipmapFilter = SamplingFilter::LINEAR;
		m_samplers.m_trilinearClamp = m_gr->newSampler(sinit);

		sinit.m_addressing = SamplingAddressing::REPEAT;
		m_samplers.m_trilinearRepeat = m_gr->newSampler(sinit);

		sinit.m_anisotropyLevel = U8(config.getNumberU32("r_textureAnisotropy"));
		m_samplers.m_trilinearRepeatAniso = m_gr->newSampler(sinit);
	}

	initJitteredMats();

	return Error::NONE;
}

void Renderer::initJitteredMats()
{
	static const Array<Vec2, 16> SAMPLE_LOCS_16 = {
		{Vec2(-8.0, 0.0), Vec2(-6.0, -4.0), Vec2(-3.0, -2.0), Vec2(-2.0, -6.0), Vec2(1.0, -1.0), Vec2(2.0, -5.0),
		 Vec2(6.0, -7.0), Vec2(5.0, -3.0), Vec2(4.0, 1.0), Vec2(7.0, 4.0), Vec2(3.0, 5.0), Vec2(0.0, 7.0),
		 Vec2(-1.0, 3.0), Vec2(-4.0, 6.0), Vec2(-7.0, 8.0), Vec2(-5.0, 2.0)}};

	for(U i = 0; i < 16; ++i)
	{
		Vec2 texSize(1.0f / Vec2(F32(m_width), F32(m_height))); // Texel size
		texSize *= 2.0f; // Move it to NDC

		Vec2 S = SAMPLE_LOCS_16[i] / 8.0f; // In [-1, 1]

		Vec2 subSample = S * texSize; // In [-texSize, texSize]
		subSample *= 0.5f; // In [-texSize / 2, texSize / 2]

		m_jitteredMats16x[i] = Mat4::getIdentity();
		m_jitteredMats16x[i].setTranslationPart(Vec4(subSample, 0.0, 1.0));
	}

	static const Array<Vec2, 8> SAMPLE_LOCS_8 = {Vec2(-7.0, 1.0), Vec2(-5.0, -5.0), Vec2(-1.0, -3.0), Vec2(3.0, -7.0),
												 Vec2(5.0, -1.0), Vec2(7.0, 7.0),   Vec2(1.0, 3.0),   Vec2(-3.0, 5.0)};

	for(U i = 0; i < 8; ++i)
	{
		Vec2 texSize(1.0f / Vec2(F32(m_width), F32(m_height))); // Texel size
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
	ctx.m_matrices.m_cameraTransform = ctx.m_renderQueue->m_cameraTransform;
	ctx.m_matrices.m_view = ctx.m_renderQueue->m_viewMatrix;
	ctx.m_matrices.m_projection = ctx.m_renderQueue->m_projectionMatrix;
	ctx.m_matrices.m_viewProjection = ctx.m_renderQueue->m_viewProjectionMatrix;

	ctx.m_matrices.m_jitter = m_jitteredMats8x[m_frameCount & (m_jitteredMats8x.getSize() - 1)];
	ctx.m_matrices.m_projectionJitter = ctx.m_matrices.m_jitter * ctx.m_matrices.m_projection;
	ctx.m_matrices.m_viewProjectionJitter = ctx.m_matrices.m_projectionJitter * ctx.m_matrices.m_view;

	ctx.m_prevMatrices = m_prevMatrices;

	ctx.m_unprojParams = ctx.m_renderQueue->m_projectionMatrix.extractPerspectiveUnprojectionParams();

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
	m_tonemapping->importRenderTargets(ctx);
	m_depth->importRenderTargets(ctx);

	// Populate render graph. WARNING Watch the order
	m_genericCompute->populateRenderGraph(ctx);
	if(m_accelerationStructureBuilder)
	{
		m_accelerationStructureBuilder->populateRenderGraph(ctx);
	}
	m_shadowMapping->populateRenderGraph(ctx);
	m_gi->populateRenderGraph(ctx);
	m_probeReflections->populateRenderGraph(ctx);
	m_volLighting->populateRenderGraph(ctx);
	m_gbuffer->populateRenderGraph(ctx);
	m_gbufferPost->populateRenderGraph(ctx);
	m_depth->populateRenderGraph(ctx);
	if(m_rtShadows)
	{
		m_rtShadows->populateRenderGraph(ctx);
	}
	else
	{
		m_smResolve->populateRenderGraph(ctx);
	}
	m_volFog->populateRenderGraph(ctx);
	m_ssao->populateRenderGraph(ctx);
	m_lensFlare->populateRenderGraph(ctx);
	m_ssr->populateRenderGraph(ctx);
	m_ssgi->populateRenderGraph(ctx);
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

	// Bin lights and update uniforms
	m_stats.m_lightBinTime = (m_statsEnabled) ? HighRezTimer::getCurrentTime() : -1.0;
	ClusterBinIn cin;
	cin.m_renderQueue = ctx.m_renderQueue;
	cin.m_tempAlloc = ctx.m_tempAllocator;
	cin.m_shadowsEnabled = true; // TODO
	cin.m_stagingMem = m_stagingMem;
	cin.m_threadHive = m_threadHive;
	m_clusterBin.bin(cin, ctx.m_clusterBinOut);

	ctx.m_prevClustererMagicValues =
		(m_frameCount > 0) ? m_prevClustererMagicValues : ctx.m_clusterBinOut.m_shaderMagicValues;
	m_prevClustererMagicValues = ctx.m_clusterBinOut.m_shaderMagicValues;

	updateLightShadingUniforms(ctx);
	m_stats.m_lightBinTime = (m_statsEnabled) ? (HighRezTimer::getCurrentTime() - m_stats.m_lightBinTime) : -1.0;

	return Error::NONE;
}

void Renderer::finalize(const RenderingContext& ctx)
{
	++m_frameCount;

	m_prevMatrices = ctx.m_matrices;

	// Inform about the HiZ map. Do it as late as possible
	if(ctx.m_renderQueue->m_fillCoverageBufferCallback)
	{
		F32* depthValues;
		U32 width;
		U32 height;
		m_depth->getClientDepthMapInfo(depthValues, width, height);
		ctx.m_renderQueue->m_fillCoverageBufferCallback(ctx.m_renderQueue->m_fillCoverageBufferCallbackUserData,
														depthValues, width, height);
	}
}

TextureInitInfo Renderer::create2DRenderTargetInitInfo(U32 w, U32 h, Format format, TextureUsageBit usage, CString name)
{
	ANKI_ASSERT(!!(usage & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE)
				|| !!(usage & TextureUsageBit::IMAGE_COMPUTE_WRITE));
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

RenderTargetDescription Renderer::create2DRenderTargetDescription(U32 w, U32 h, Format format, CString name)
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
	init.m_usage = TextureUsageBit::NONE;

	return init;
}

TexturePtr Renderer::createAndClearRenderTarget(const TextureInitInfo& inf, const ClearValue& clearVal)
{
	ANKI_ASSERT(!!(inf.m_usage & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE)
				|| !!(inf.m_usage & TextureUsageBit::IMAGE_COMPUTE_WRITE));

	const U faceCount = (inf.m_type == TextureType::CUBE || inf.m_type == TextureType::CUBE_ARRAY) ? 6 : 1;

	Bool useCompute = false;
	if(!!(inf.m_usage & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE))
	{
		useCompute = false;
	}
	else if(!!(inf.m_usage & TextureUsageBit::IMAGE_COMPUTE_WRITE))
	{
		useCompute = true;
	}
	else
	{
		ANKI_ASSERT(!"Can't handle that");
	}

	// Create tex
	TexturePtr tex = m_gr->newTexture(inf);

	// Clear all surfaces
	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = (useCompute) ? CommandBufferFlag::COMPUTE_WORK : CommandBufferFlag::GRAPHICS_WORK;
	if((inf.m_mipmapCount * faceCount * inf.m_layerCount * 4) < COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS)
	{
		cmdbinit.m_flags |= CommandBufferFlag::SMALL_BATCH;
	}
	CommandBufferPtr cmdb = m_gr->newCommandBuffer(cmdbinit);

	for(U32 mip = 0; mip < inf.m_mipmapCount; ++mip)
	{
		for(U32 face = 0; face < faceCount; ++face)
		{
			for(U32 layer = 0; layer < inf.m_layerCount; ++layer)
			{
				TextureSurfaceInfo surf(mip, 0, face, layer);

				if(!useCompute)
				{
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

					cmdb->setTextureSurfaceBarrier(tex, TextureUsageBit::NONE,
												   TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf);

					cmdb->beginRenderPass(fb, colUsage, dsUsage);
					cmdb->endRenderPass();

					if(!!inf.m_initialUsage)
					{
						cmdb->setTextureSurfaceBarrier(tex, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
													   inf.m_initialUsage, surf);
					}
				}
				else
				{
					// Compute
					ShaderProgramResourceVariantInitInfo variantInitInfo(m_clearTexComputeProg);
					variantInitInfo.addMutation("IS_2D", I32((inf.m_type != TextureType::_3D) ? 1 : 0));

					const ShaderProgramResourceVariant* variant;
					m_clearTexComputeProg->getOrCreateVariant(variantInitInfo, variant);

					cmdb->bindShaderProgram(variant->getProgram());

					cmdb->setPushConstants(&clearVal.m_colorf[0], sizeof(clearVal.m_colorf));

					TextureViewPtr view = getGrManager().newTextureView(TextureViewInitInfo(tex, surf));
					cmdb->bindImage(0, 0, view);

					cmdb->setTextureSurfaceBarrier(tex, TextureUsageBit::NONE, TextureUsageBit::IMAGE_COMPUTE_WRITE,
												   surf);

					UVec3 wgSize;
					wgSize.x() = (8 - 1 + (tex->getWidth() >> mip)) / 8;
					wgSize.y() = (8 - 1 + (tex->getHeight() >> mip)) / 8;
					wgSize.z() = (inf.m_type == TextureType::_3D) ? ((8 - 1 + (tex->getDepth() >> mip)) / 8) : 1;

					cmdb->dispatchCompute(wgSize.x(), wgSize.y(), wgSize.z());

					if(!!inf.m_initialUsage)
					{
						cmdb->setTextureSurfaceBarrier(tex, TextureUsageBit::IMAGE_COMPUTE_WRITE, inf.m_initialUsage,
													   surf);
					}
				}
			}
		}
	}

	cmdb->flush();

	return tex;
}

void Renderer::updateLightShadingUniforms(RenderingContext& ctx) const
{
	LightingUniforms* blk = static_cast<LightingUniforms*>(m_stagingMem->allocateFrame(
		sizeof(LightingUniforms), StagingGpuMemoryType::UNIFORM, ctx.m_lightShadingUniformsToken));

	// Start writing
	blk->m_unprojectionParams = ctx.m_unprojParams;

	blk->m_rendererSize = Vec2(F32(m_width), F32(m_height));
	blk->m_time = F32(HighRezTimer::getCurrentTime());
	blk->m_near = ctx.m_renderQueue->m_cameraNear;

	blk->m_clusterCount = UVec4(m_clusterCount[0], m_clusterCount[1], m_clusterCount[2], m_clusterCount[3]);

	blk->m_cameraPos = ctx.m_renderQueue->m_cameraTransform.getTranslationPart().xyz();
	blk->m_far = ctx.m_renderQueue->m_cameraFar;

	blk->m_clustererMagicValues = ctx.m_clusterBinOut.m_shaderMagicValues;
	blk->m_prevClustererMagicValues = ctx.m_prevClustererMagicValues;

	blk->m_lightVolumeLastCluster = m_volLighting->getFinalClusterInZ();
	blk->m_frameCount = m_frameCount & MAX_U32;

	// Matrices
	blk->m_viewMat = ctx.m_renderQueue->m_viewMatrix;
	blk->m_invViewMat = ctx.m_renderQueue->m_viewMatrix.getInverse();

	blk->m_projMat = ctx.m_matrices.m_projectionJitter;
	blk->m_invProjMat = ctx.m_matrices.m_projectionJitter.getInverse();

	blk->m_viewProjMat = ctx.m_matrices.m_viewProjectionJitter;
	blk->m_invViewProjMat = ctx.m_matrices.m_viewProjectionJitter.getInverse();

	blk->m_prevViewProjMat = ctx.m_prevMatrices.m_viewProjectionJitter;

	blk->m_prevViewProjMatMulInvViewProjMat =
		ctx.m_prevMatrices.m_viewProjection * ctx.m_matrices.m_viewProjectionJitter.getInverse();

	// Directional light
	if(ctx.m_renderQueue->m_directionalLight.m_uuid != 0)
	{
		DirectionalLight& out = blk->m_dirLight;
		const DirectionalLightQueueElement& in = ctx.m_renderQueue->m_directionalLight;

		out.m_diffuseColor = in.m_diffuseColor;
		out.m_cascadeCount = in.m_shadowCascadeCount;
		out.m_dir = in.m_direction;
		out.m_active = 1;
		out.m_effectiveShadowDistance = in.m_effectiveShadowDistance;
		out.m_shadowCascadesDistancePower = in.m_shadowCascadesDistancePower;
		out.m_shadowLayer = in.m_shadowLayer;

		for(U cascade = 0; cascade < in.m_shadowCascadeCount; ++cascade)
		{
			out.m_textureMatrices[cascade] = in.m_textureMatrices[cascade];
		}
	}
	else
	{
		blk->m_dirLight.m_active = 0;
	}
}

void Renderer::registerDebugRenderTarget(RendererObject* obj, CString rtName)
{
#if ANKI_ENABLE_ASSERTS
	for(const DebugRtInfo& inf : m_debugRts)
	{
		ANKI_ASSERT(inf.m_rtName != rtName && "Choose different name");
	}
#endif

	ANKI_ASSERT(obj);
	DebugRtInfo inf;
	inf.m_obj = obj;
	inf.m_rtName.create(getAllocator(), rtName);

	m_debugRts.emplaceBack(getAllocator(), std::move(inf));
}

void Renderer::getCurrentDebugRenderTarget(RenderTargetHandle& handle, Bool& handleValid)
{
	if(ANKI_LIKELY(m_currentDebugRtName.isEmpty()))
	{
		handleValid = false;
		return;
	}

	RendererObject* obj = nullptr;
	for(const DebugRtInfo& inf : m_debugRts)
	{
		if(inf.m_rtName == m_currentDebugRtName)
		{
			obj = inf.m_obj;
		}
	}
	ANKI_ASSERT(obj);

	obj->getDebugRenderTarget(m_currentDebugRtName, handle);
	handleValid = true;
}

void Renderer::setCurrentDebugRenderTarget(CString rtName)
{
	m_currentDebugRtName.destroy(getAllocator());

	if(!rtName.isEmpty() && rtName.getLength() > 0)
	{
		m_currentDebugRtName.create(getAllocator(), rtName);
	}
}

} // end namespace anki
