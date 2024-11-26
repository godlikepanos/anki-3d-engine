// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Collision/Plane.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>
#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Scene/Components/CameraComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Core/App.h>

#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/GBufferPost.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/Bloom.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Renderer/ForwardShading.h>
#include <AnKi/Renderer/LensFlare.h>
#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Renderer/VolumetricFog.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/TemporalAA.h>
#include <AnKi/Renderer/UiStage.h>
#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Renderer/IndirectDiffuseProbes.h>
#include <AnKi/Renderer/ShadowmapsResolve.h>
#include <AnKi/Renderer/RtShadows.h>
#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/TemporalUpscaler.h>
#include <AnKi/Renderer/VrsSriGeneration.h>
#include <AnKi/Renderer/PrimaryNonRenderableVisibility.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/Ssao.h>
#include <AnKi/Renderer/Sky.h>
#include <AnKi/Renderer/MotionBlur.h>
#include <AnKi/Renderer/RtMaterialFetchDbg.h>
#include <AnKi/Renderer/Reflections.h>
#include <AnKi/Renderer/Utils/Drawer.h>
#include <AnKi/Renderer/Utils/GpuVisibility.h>
#include <AnKi/Renderer/Utils/MipmapGenerator.h>
#include <AnKi/Renderer/Utils/Readback.h>
#include <AnKi/Renderer/Utils/HzbGenerator.h>

namespace anki {

static StatCounter g_primitivesDrawnStatVar(StatCategory::kRenderer, "Primitives drawn", StatFlag::kMainThreadUpdates | StatFlag::kZeroEveryFrame);
static StatCounter g_rendererCpuTimeStatVar(StatCategory::kTime, "Renderer",
											StatFlag::kMilisecond | StatFlag::kShowAverage | StatFlag::kMainThreadUpdates);

/// Generate a Halton jitter in [-0.5, 0.5]
static Vec2 generateJitter(U32 frame)
{
	// Halton jitter
	Vec2 result(0.0f);

	constexpr U32 baseX = 2;
	U32 index = frame + 1;
	F32 invBase = 1.0f / baseX;
	F32 fraction = invBase;
	while(index > 0)
	{
		result.x() += F32(index % baseX) * fraction;
		index /= baseX;
		fraction *= invBase;
	}

	constexpr U32 baseY = 3;
	index = frame + 1;
	invBase = 1.0f / baseY;
	fraction = invBase;
	while(index > 0)
	{
		result.y() += F32(index % baseY) * fraction;
		index /= baseY;
		fraction *= invBase;
	}

	result.x() -= 0.5f;
	result.y() -= 0.5f;
	return result;
}

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
#define ANKI_RENDERER_OBJECT_DEF(name, name2, initCondition) deleteInstance(RendererMemoryPool::getSingleton(), m_##name2);
#include <AnKi/Renderer/RendererObject.def.h>
}

Error Renderer::init(const RendererInitInfo& inf)
{
	ANKI_TRACE_SCOPED_EVENT(RInit);

	const Error err = initInternal(inf);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize the renderer");
	}

	return err;
}

Error Renderer::initInternal(const RendererInitInfo& inf)
{
	RendererMemoryPool::allocateSingleton(inf.m_allocCallback, inf.m_allocCallbackUserData);

	m_framePool.init(inf.m_allocCallback, inf.m_allocCallbackUserData, 10_MB, 1.0f);
	m_frameCount = 0;
	m_swapchainResolution = inf.m_swapchainSize;
	m_rgraph = GrManager::getSingleton().newRenderGraph();

	// Set from the config
	m_postProcessResolution = UVec2(Vec2(m_swapchainResolution) * g_renderScalingCVar);
	alignRoundDown(2, m_postProcessResolution.x());
	alignRoundDown(2, m_postProcessResolution.y());

	m_internalResolution = UVec2(Vec2(m_postProcessResolution) * g_internalRenderScalingCVar);
	alignRoundDown(2, m_internalResolution.x());
	alignRoundDown(2, m_internalResolution.y());

	ANKI_R_LOGI("Initializing offscreen renderer. Resolution %ux%u. Internal resolution %ux%u", m_postProcessResolution.x(),
				m_postProcessResolution.y(), m_internalResolution.x(), m_internalResolution.y());

	m_tileCounts.x() = (m_internalResolution.x() + kClusteredShadingTileSize - 1) / kClusteredShadingTileSize;
	m_tileCounts.y() = (m_internalResolution.y() + kClusteredShadingTileSize - 1) / kClusteredShadingTileSize;
	m_zSplitCount = g_zSplitCountCVar;

	if(g_meshletRenderingCVar && !GrManager::getSingleton().getDeviceCapabilities().m_meshShaders)
	{
		m_meshletRenderingType = MeshletRenderingType::kSoftware;
	}
	else if(GrManager::getSingleton().getDeviceCapabilities().m_meshShaders)
	{
		m_meshletRenderingType = MeshletRenderingType::kMeshShaders;
	}
	else
	{
		m_meshletRenderingType = MeshletRenderingType::kNone;
	}

	// A few sanity checks
	if(m_internalResolution.x() < 64 || m_internalResolution.y() < 64)
	{
		ANKI_R_LOGE("Incorrect sizes");
		return Error::kUserData;
	}

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/ClearTextureCompute.ankiprogbin", m_clearTexComputeProg));

	// Dummy resources
	{
		TextureInitInfo texinit("RendererDummy");
		texinit.m_width = texinit.m_height = 4;
		texinit.m_usage = TextureUsageBit::kAllSrv | TextureUsageBit::kUavCompute;
		texinit.m_format = Format::kR8G8B8A8_Unorm;
		m_dummyTex2d = createAndClearRenderTarget(texinit, TextureUsageBit::kAllSrv);

		texinit.m_depth = 4;
		texinit.m_type = TextureType::k3D;
		m_dummyTex3d = createAndClearRenderTarget(texinit, TextureUsageBit::kAllSrv);

		m_dummyBuff = GrManager::getSingleton().newBuffer(
			BufferInitInfo(1024, BufferUsageBit::kAllConstant | BufferUsageBit::kAllUav, BufferMapAccessBit::kNone, "Dummy"));
	}

	// Init the stages
#define ANKI_RENDERER_OBJECT_DEF(name, name2, initCondition) \
	if(initCondition) \
	{ \
		m_##name2 = newInstance<name>(RendererMemoryPool::getSingleton()); \
		ANKI_R_LOGV("Initializing " ANKI_STRINGIZE(name)); \
		const Error err = m_##name2->init(); \
		if(err) \
		{ \
			ANKI_R_LOGE("Initialization failed: " ANKI_STRINGIZE(name)); \
			return err; \
		} \
	}
#include <AnKi/Renderer/RendererObject.def.h>

	// Init samplers
	{
		SamplerInitInfo sinit("NearestNearestClamp");
		sinit.m_addressing = SamplingAddressing::kClamp;
		sinit.m_mipmapFilter = SamplingFilter::kNearest;
		sinit.m_minMagFilter = SamplingFilter::kNearest;
		m_samplers.m_nearestNearestClamp = GrManager::getSingleton().newSampler(sinit);

		sinit.setName("TrilinearClamp");
		sinit.m_minMagFilter = SamplingFilter::kLinear;
		sinit.m_mipmapFilter = SamplingFilter::kLinear;
		m_samplers.m_trilinearClamp = GrManager::getSingleton().newSampler(sinit);

		sinit.setName("TrilinearRepeat");
		sinit.m_addressing = SamplingAddressing::kRepeat;
		m_samplers.m_trilinearRepeat = GrManager::getSingleton().newSampler(sinit);

		if(g_textureAnisotropyCVar <= 1u)
		{
			m_samplers.m_trilinearRepeatAniso = m_samplers.m_trilinearRepeat;
		}
		else
		{
			sinit.setName("TrilinearRepeatAniso");
			sinit.m_anisotropyLevel = g_textureAnisotropyCVar;
			m_samplers.m_trilinearRepeatAniso = GrManager::getSingleton().newSampler(sinit);
		}

		sinit.setName("TrilinearRepeatAnisoRezScalingBias");
		F32 scalingMipBias = log2(F32(m_internalResolution.x()) / F32(m_postProcessResolution.x()));
		if(getTemporalUpscaler().getEnabled())
		{
			// DLSS wants more bias
			scalingMipBias -= 1.0f;
		}

		sinit.m_lodBias = scalingMipBias;
		m_samplers.m_trilinearRepeatAnisoResolutionScalingBias = GrManager::getSingleton().newSampler(sinit);

		sinit = {};
		sinit.setName("TrilinearClampShadow");
		sinit.m_minMagFilter = SamplingFilter::kLinear;
		sinit.m_mipmapFilter = SamplingFilter::kLinear;
		sinit.m_compareOperation = CompareOperation::kLessEqual;
		m_samplers.m_trilinearClampShadow = GrManager::getSingleton().newSampler(sinit);
	}

	for(U32 i = 0; i < m_jitterOffsets.getSize(); ++i)
	{
		m_jitterOffsets[i] = generateJitter(i);
	}

	if(m_swapchainResolution != m_postProcessResolution)
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/Blit.ankiprogbin", m_blitProg));
		ShaderProgramResourceVariantInitInfo varInit(m_blitProg);
		const ShaderProgramResourceVariant* variant;
		varInit.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel);
		m_blitProg->getOrCreateVariant(varInit, variant);
		m_blitGrProg.reset(&variant->getProgram());

		ANKI_R_LOGI("There will be a blit pass to the swapchain because render scaling is not 1.0");
	}

	return Error::kNone;
}

Error Renderer::populateRenderGraph(RenderingContext& ctx)
{
	// Import RTs first
	m_bloom2->importRenderTargets(ctx);
	m_tonemapping->importRenderTargets(ctx);
	m_vrsSriGeneration->importRenderTargets(ctx);
	m_gbuffer->importRenderTargets(ctx);

	// Populate render graph. WARNING Watch the order
	gpuSceneCopy(ctx);
	m_primaryNonRenderableVisibility->populateRenderGraph(ctx);
	if(m_accelerationStructureBuilder)
	{
		m_accelerationStructureBuilder->populateRenderGraph(ctx);
	}
	m_gbuffer->populateRenderGraph(ctx);
	m_shadowMapping->populateRenderGraph(ctx);
	m_clusterBinning2->populateRenderGraph(ctx);
	m_generatedSky->populateRenderGraph(ctx);
	m_indirectDiffuseProbes->populateRenderGraph(ctx);
	m_probeReflections->populateRenderGraph(ctx);
	m_volumetricLightingAccumulation->populateRenderGraph(ctx);
	m_motionVectors->populateRenderGraph(ctx);
	m_gbufferPost->populateRenderGraph(ctx);
	m_depthDownscale->populateRenderGraph(ctx);
	if(m_rtShadows)
	{
		m_rtShadows->populateRenderGraph(ctx);
	}
	if(m_rtMaterialFetchDbg)
	{
		m_rtMaterialFetchDbg->populateRenderGraph(ctx);
	}

	m_reflections->populateRenderGraph(ctx);
	m_shadowmapsResolve->populateRenderGraph(ctx);
	m_volumetricFog->populateRenderGraph(ctx);
	m_lensFlare->populateRenderGraph(ctx);
	m_ssao->populateRenderGraph(ctx);
	m_forwardShading->populateRenderGraph(ctx); // This may feel out of place but it's only visibility. Keep it just before light shading
	m_lightShading->populateRenderGraph(ctx);
	if(getTemporalUpscaler().getEnabled())
	{
		m_temporalUpscaler->populateRenderGraph(ctx);
	}
	else
	{
		m_temporalAA->populateRenderGraph(ctx);
	}
	m_vrsSriGeneration->populateRenderGraph(ctx);
	m_tonemapping->populateRenderGraph(ctx);
	m_motionBlur->populateRenderGraph(ctx);
	m_bloom2->populateRenderGraph(ctx);
	m_dbg->populateRenderGraph(ctx);

	m_finalComposite->populateRenderGraph(ctx);

	return Error::kNone;
}

void Renderer::writeGlobalRendererConstants(RenderingContext& ctx, GlobalRendererConstants& consts)
{
	ANKI_TRACE_SCOPED_EVENT(RWriteGlobalRendererConstants);

	consts.m_renderingSize = Vec2(F32(m_internalResolution.x()), F32(m_internalResolution.y()));

	consts.m_time = F32(HighRezTimer::getCurrentTime());
	consts.m_frame = m_frameCount & kMaxU32;

	Plane nearPlane;
	extractClipPlane(ctx.m_matrices.m_viewProjection, FrustumPlaneType::kNear, nearPlane);
	consts.m_nearPlaneWSpace = Vec4(nearPlane.getNormal().xyz(), nearPlane.getOffset());
	consts.m_near = ctx.m_cameraNear;
	consts.m_far = ctx.m_cameraFar;
	consts.m_cameraPosition = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();

	consts.m_tileCounts = m_tileCounts;
	consts.m_zSplitCount = m_zSplitCount;
	consts.m_zSplitCountOverFrustumLength = F32(m_zSplitCount) / (ctx.m_cameraFar - ctx.m_cameraNear);
	consts.m_zSplitMagic.x() = (ctx.m_cameraNear - ctx.m_cameraFar) / (ctx.m_cameraNear * F32(m_zSplitCount));
	consts.m_zSplitMagic.y() = ctx.m_cameraFar / (ctx.m_cameraNear * F32(m_zSplitCount));
	consts.m_lightVolumeLastZSplit = min(g_volumetricLightingAccumulationFinalZSplitCVar - 1, m_zSplitCount);

	consts.m_reflectionProbesMipCount = F32(m_probeReflections->getReflectionTextureMipmapCount());

	consts.m_matrices = ctx.m_matrices;
	consts.m_previousMatrices = ctx.m_prevMatrices;

	// Directional light
	const LightComponent* dirLight = SceneGraph::getSingleton().getDirectionalLight();
	if(dirLight)
	{
		DirectionalLight& out = consts.m_directionalLight;
		const U32 shadowCascadeCount = (dirLight->getShadowEnabled()) ? g_shadowCascadeCountCVar : 0;

		out.m_diffuseColor = dirLight->getDiffuseColor().xyz();
		out.m_power = dirLight->getLightPower();
		out.m_shadowCascadeCount_31bit_active_1bit = shadowCascadeCount << 1u;
		out.m_shadowCascadeCount_31bit_active_1bit |= 1;
		out.m_direction = dirLight->getDirection();
		out.m_shadowCascadeDistances =
			Vec4(g_shadowCascade0DistanceCVar, g_shadowCascade1DistanceCVar, g_shadowCascade2DistanceCVar, g_shadowCascade3DistanceCVar);

		for(U cascade = 0; cascade < shadowCascadeCount; ++cascade)
		{
			ANKI_ASSERT(ctx.m_dirLightTextureMatrices[cascade] != Mat4::getZero());
			out.m_textureMatrices[cascade] = ctx.m_dirLightTextureMatrices[cascade];
			out.m_cascadeFarPlanes[cascade] = ctx.m_dirLightFarPlanes[cascade];
			out.m_cascadePcfTexelRadius[cascade] = ctx.m_dirLightPcfTexelRadius[cascade];
		}
	}
	else
	{
		consts.m_directionalLight.m_shadowCascadeCount_31bit_active_1bit = 0;
	}

	// Sky
	const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();

	const Bool isSolidColor =
		(!sky || sky->getSkyboxType() == SkyboxType::kSolidColor || (!dirLight && sky->getSkyboxType() == SkyboxType::kGenerated));
	if(isSolidColor)
	{
		consts.m_sky.m_solidColor = (sky) ? sky->getSolidColor() : Vec3(0.0);
		consts.m_sky.m_type = 0;
	}
	else if(sky->getSkyboxType() == SkyboxType::kImage2D)
	{
		consts.m_sky.m_type = 1;
	}
	else
	{
		consts.m_sky.m_type = 2;
	}
}

TextureInitInfo Renderer::create2DRenderTargetInitInfo(U32 w, U32 h, Format format, TextureUsageBit usage, CString name)
{
	ANKI_ASSERT(!!(usage & TextureUsageBit::kRtvDsvWrite) || !!(usage & TextureUsageBit::kUavCompute));
	TextureInitInfo init(name);

	init.m_width = w;
	init.m_height = h;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_type = TextureType::k2D;
	init.m_format = format;
	init.m_mipmapCount = 1;
	init.m_samples = 1;
	init.m_usage = usage;

	return init;
}

RenderTargetDesc Renderer::create2DRenderTargetDescription(U32 w, U32 h, Format format, CString name)
{
	RenderTargetDesc init(name);

	init.m_width = w;
	init.m_height = h;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_type = TextureType::k2D;
	init.m_format = format;
	init.m_mipmapCount = 1;
	init.m_samples = 1;
	init.m_usage = TextureUsageBit::kNone;

	return init;
}

TexturePtr Renderer::createAndClearRenderTarget(const TextureInitInfo& inf, TextureUsageBit initialUsage, const ClearValue& clearVal)
{
	ANKI_ASSERT(!!(inf.m_usage & TextureUsageBit::kRtvDsvWrite) || !!(inf.m_usage & TextureUsageBit::kUavCompute));

	const U faceCount = textureTypeIsCube(inf.m_type) ? 6 : 1;

	Bool useCompute = false;
	if(!!(inf.m_usage & TextureUsageBit::kRtvDsvWrite))
	{
		useCompute = false;
	}
	else if(!!(inf.m_usage & TextureUsageBit::kUavCompute))
	{
		useCompute = true;
	}
	else
	{
		ANKI_ASSERT(!"Can't handle that");
	}

	// Create tex
	TexturePtr tex = GrManager::getSingleton().newTexture(inf);

	// Clear all surfaces
	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::kGeneralWork;
	if((inf.m_mipmapCount * faceCount * inf.m_layerCount * 4) < kCommandBufferSmallBatchMaxCommands)
	{
		cmdbinit.m_flags |= CommandBufferFlag::kSmallBatch;
	}
	CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbinit);

	for(U32 mip = 0; mip < inf.m_mipmapCount; ++mip)
	{
		for(U32 face = 0; face < faceCount; ++face)
		{
			for(U32 layer = 0; layer < inf.m_layerCount; ++layer)
			{
				if(!useCompute)
				{
					RenderTarget rt;
					rt.m_clearValue = clearVal;

					if(getFormatInfo(inf.m_format).isDepthStencil())
					{
						DepthStencilAspectBit aspect = DepthStencilAspectBit::kNone;
						if(getFormatInfo(inf.m_format).isDepth())
						{
							aspect |= DepthStencilAspectBit::kDepth;
						}

						if(getFormatInfo(inf.m_format).isStencil())
						{
							aspect |= DepthStencilAspectBit::kStencil;
						}

						rt.m_textureView = TextureView(tex.get(), TextureSubresourceDesc::surface(mip, face, layer, aspect));
					}
					else
					{
						rt.m_textureView = TextureView(tex.get(), TextureSubresourceDesc::surface(mip, face, layer));
					}

					TextureBarrierInfo barrier = {rt.m_textureView, TextureUsageBit::kNone, TextureUsageBit::kRtvDsvWrite};
					cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

					if(getFormatInfo(inf.m_format).isDepthStencil())
					{
						cmdb->beginRenderPass({}, &rt);
					}
					else
					{
						cmdb->beginRenderPass({rt});
					}
					cmdb->endRenderPass();

					if(!!initialUsage)
					{
						barrier.m_previousUsage = TextureUsageBit::kRtvDsvWrite;
						barrier.m_nextUsage = initialUsage;
						cmdb->setPipelineBarrier({&barrier, 1}, {}, {});
					}
				}
				else
				{
					// Compute
					ShaderProgramResourceVariantInitInfo variantInitInfo(m_clearTexComputeProg);
					variantInitInfo.addMutation("TEXTURE_DIMENSIONS", I32((inf.m_type == TextureType::k3D) ? 3 : 2));

					const FormatInfo formatInfo = getFormatInfo(inf.m_format);
					I32 componentType = 0;
					if(formatInfo.m_shaderType == 0)
					{
						componentType = 0;
					}
					else if(formatInfo.m_shaderType == 1)
					{
						componentType = 1;
					}
					else
					{
						ANKI_ASSERT(!"Not supported");
					}
					variantInitInfo.addMutation("COMPONENT_TYPE", componentType);

					const ShaderProgramResourceVariant* variant;
					m_clearTexComputeProg->getOrCreateVariant(variantInitInfo, variant);

					cmdb->bindShaderProgram(&variant->getProgram());

					cmdb->setFastConstants(&clearVal.m_colorf[0], sizeof(clearVal.m_colorf));

					const TextureView view(tex.get(), TextureSubresourceDesc::surface(mip, face, layer));

					cmdb->bindUav(0, 0, view);

					const TextureBarrierInfo barrier = {view, TextureUsageBit::kNone, TextureUsageBit::kUavCompute};
					cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

					UVec3 wgSize;
					wgSize.x() = (8 - 1 + (tex->getWidth() >> mip)) / 8;
					wgSize.y() = (8 - 1 + (tex->getHeight() >> mip)) / 8;
					wgSize.z() = (inf.m_type == TextureType::k3D) ? ((8 - 1 + (tex->getDepth() >> mip)) / 8) : 1;

					cmdb->dispatchCompute(wgSize.x(), wgSize.y(), wgSize.z());

					if(!!initialUsage)
					{
						const TextureBarrierInfo barrier = {view, TextureUsageBit::kUavCompute, initialUsage};

						cmdb->setPipelineBarrier({&barrier, 1}, {}, {});
					}
				}
			}
		}
	}

	cmdb->endRecording();

	FencePtr fence;
	GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
	fence->clientWait(10.0_sec);

	return tex;
}

void Renderer::registerDebugRenderTarget(RendererObject* obj, CString rtName)
{
#if ANKI_ASSERTIONS_ENABLED
	for(const DebugRtInfo& inf : m_debugRts)
	{
		ANKI_ASSERT(inf.m_rtName != rtName && "Choose different name");
	}
#endif

	ANKI_ASSERT(obj);
	DebugRtInfo inf;
	inf.m_obj = obj;
	inf.m_rtName = rtName;

	m_debugRts.emplaceBack(std::move(inf));
}

Bool Renderer::getCurrentDebugRenderTarget(Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles, ShaderProgramPtr& optionalShaderProgram)
{
	if(m_currentDebugRtName.isEmpty()) [[likely]]
	{
		return false;
	}

	RendererObject* obj = nullptr;
	for(const DebugRtInfo& inf : m_debugRts)
	{
		if(inf.m_rtName == m_currentDebugRtName)
		{
			obj = inf.m_obj;
		}
	}

	if(obj)
	{
		obj->getDebugRenderTarget(m_currentDebugRtName, handles, optionalShaderProgram);
		return true;
	}
	else
	{
		ANKI_R_LOGE("Debug rendertarget doesn't exist: %s", m_currentDebugRtName.cstr());
		m_currentDebugRtName = {};
		return false;
	}
}

void Renderer::setCurrentDebugRenderTarget(CString rtName)
{
	m_currentDebugRtName.destroy();

	if(!rtName.isEmpty() && rtName.getLength() > 0)
	{
		m_currentDebugRtName = rtName;
	}
}

Format Renderer::getHdrFormat() const
{
	Format out;
	if(!g_highQualityHdrCVar)
	{
		out = Format::kB10G11R11_Ufloat_Pack32;
	}
	else if(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats)
	{
		out = Format::kR16G16B16_Sfloat;
	}
	else
	{
		out = Format::kR16G16B16A16_Sfloat;
	}
	return out;
}

Format Renderer::getDepthNoStencilFormat() const
{
	if(ANKI_PLATFORM_MOBILE)
	{
		return Format::kX8D24_Unorm_Pack32;
	}
	else
	{
		return Format::kD32_Sfloat;
	}
}

void Renderer::gpuSceneCopy(RenderingContext& ctx)
{
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_gpuSceneHandle =
		rgraph.importBuffer(GpuSceneBuffer::getSingleton().getBufferView(), GpuSceneBuffer::getSingleton().getBuffer().getBufferUsage());

	if(GpuSceneMicroPatcher::getSingleton().patchingIsNeeded())
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("GPU scene patching");
		rpass.newBufferDependency(m_runCtx.m_gpuSceneHandle, BufferUsageBit::kUavCompute);

		rpass.setWork([](RenderPassWorkContext& rgraphCtx) {
			GpuSceneMicroPatcher::getSingleton().patchGpuScene(*rgraphCtx.m_commandBuffer);
		});
	}
}

#if ANKI_STATS_ENABLED
void Renderer::updatePipelineStats()
{
	RendererDynamicArray<PipelineQueryPtr>& arr = m_pipelineQueries[m_frameCount % kMaxFramesInFlight];

	U64 sum = 0;
	for(PipelineQueryPtr& q : arr)
	{
		U64 value;
		const PipelineQueryResult res = q->getResult(value);
		if(res == PipelineQueryResult::kNotAvailable)
		{
			ANKI_R_LOGW("Pipeline query result is not available");
		}
		else
		{
			sum += value;
		}
	}

	arr.destroy();

	g_primitivesDrawnStatVar.set(sum);
}
#endif

Error Renderer::render(Texture* presentTex)
{
	ANKI_TRACE_SCOPED_EVENT(Render);

	const Second startTime = HighRezTimer::getCurrentTime();

	// First thing, reset the temp mem pool
	m_framePool.reset();

	RenderingContext ctx(&m_framePool);
	ctx.m_renderGraphDescr.setStatisticsEnabled(ANKI_STATS_ENABLED);
	ctx.m_swapchainRenderTarget = ctx.m_renderGraphDescr.importRenderTarget(presentTex, TextureUsageBit::kNone);

#if ANKI_STATS_ENABLED
	updatePipelineStats();
#endif

	const CameraComponent& cam = SceneGraph::getSingleton().getActiveCameraNode().getFirstComponentOfType<CameraComponent>();

	ctx.m_prevMatrices = m_prevMatrices;

	ctx.m_matrices.m_cameraTransform = Mat3x4(cam.getFrustum().getWorldTransform());
	ctx.m_matrices.m_view = cam.getFrustum().getViewMatrix();
	ctx.m_matrices.m_projection = cam.getFrustum().getProjectionMatrix();
	ctx.m_matrices.m_viewProjection = cam.getFrustum().getViewProjectionMatrix();

	Vec2 jitter = m_jitterOffsets[m_frameCount & (m_jitterOffsets.getSize() - 1)]; // In [-0.5, 0.5]
	jitter *= 2.0f; // In [-1, 1]
	const Vec2 ndcPixelSize = 1.0f / Vec2(m_internalResolution);
	jitter *= ndcPixelSize;
	ctx.m_matrices.m_jitter = Mat4::getIdentity();
	ctx.m_matrices.m_jitter.setTranslationPart(Vec3(jitter, 0.0f));
	ctx.m_matrices.m_jitterOffsetNdc = jitter;

	ctx.m_matrices.m_projectionJitter = ctx.m_matrices.m_jitter * ctx.m_matrices.m_projection;
	ctx.m_matrices.m_viewProjectionJitter = ctx.m_matrices.m_projectionJitter * Mat4(ctx.m_matrices.m_view, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	ctx.m_matrices.m_invertedViewProjectionJitter = ctx.m_matrices.m_viewProjectionJitter.getInverse();
	ctx.m_matrices.m_invertedViewProjection = ctx.m_matrices.m_viewProjection.getInverse();
	ctx.m_matrices.m_invertedProjectionJitter = ctx.m_matrices.m_projectionJitter.getInverse();

	ctx.m_matrices.m_reprojection = ctx.m_prevMatrices.m_viewProjection * ctx.m_matrices.m_invertedViewProjection;

	ctx.m_matrices.m_unprojectionParameters = ctx.m_matrices.m_projection.extractPerspectiveUnprojectionParams();

	ctx.m_matrices.m_projMat00_11_22_23 = Vec4(ctx.m_matrices.m_projection(0, 0), ctx.m_matrices.m_projection(1, 1),
											   ctx.m_matrices.m_projection(2, 2), ctx.m_matrices.m_projection(2, 3));

	ctx.m_cameraNear = cam.getNear();
	ctx.m_cameraFar = cam.getFar();

	// Allocate global constants
	GlobalRendererConstants* globalConsts;
	{
		U32 alignment = (GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
							? sizeof(*globalConsts)
							: GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment;
		alignment = computeCompoundAlignment(alignment, GrManager::getSingleton().getDeviceCapabilities().m_constantBufferBindOffsetAlignment);
		ctx.m_globalRenderingConstantsBuffer = RebarTransientMemoryPool::getSingleton().allocate(sizeof(*globalConsts), alignment, globalConsts);
	}

	ANKI_CHECK(populateRenderGraph(ctx));

	// Blit renderer's result to swapchain
	const Bool bNeedsBlit = m_postProcessResolution != m_swapchainResolution;
	if(bNeedsBlit)
	{
		GraphicsRenderPass& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Final Blit");
		pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(ctx.m_swapchainRenderTarget)});

		pass.newTextureDependency(ctx.m_swapchainRenderTarget, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(m_finalComposite->getRenderTarget(), TextureUsageBit::kSrvPixel);

		pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			cmdb.setViewport(0, 0, m_swapchainResolution.x(), m_swapchainResolution.y());

			cmdb.bindShaderProgram(m_blitGrProg.get());
			cmdb.bindSampler(0, 0, m_samplers.m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, m_finalComposite->getRenderTarget());

			cmdb.draw(PrimitiveTopology::kTriangles, 3);

			// Draw the UI
			m_uiStage->draw(m_swapchainResolution.x(), m_swapchainResolution.y(), cmdb);
		});
	}

	// Create a dummy pass to transition the presentable image to present
	{
		NonGraphicsRenderPass& pass = ctx.m_renderGraphDescr.newNonGraphicsRenderPass("Present");

		pass.setWork([]([[maybe_unused]] RenderPassWorkContext& rgraphCtx) {
			// Do nothing. This pass is dummy
		});
		pass.newTextureDependency(ctx.m_swapchainRenderTarget, TextureUsageBit::kPresent);
	}

	writeGlobalRendererConstants(ctx, *globalConsts);

	// Bake the render graph
	m_rgraph->compileNewGraph(ctx.m_renderGraphDescr, m_framePool);

	// Flush
	FencePtr fence;
	m_rgraph->recordAndSubmitCommandBuffers(&fence);

	// Misc
	m_rgraph->reset();
	++m_frameCount;
	m_prevMatrices = ctx.m_matrices;
	m_readbackManager->endFrame(fence.get());

	// Stats
	if(ANKI_STATS_ENABLED || ANKI_TRACING_ENABLED)
	{
		g_rendererCpuTimeStatVar.set((HighRezTimer::getCurrentTime() - startTime) * 1000.0);

		RenderGraphStatistics rgraphStats;
		m_rgraph->getStatistics(rgraphStats);
		g_rendererGpuTimeStatVar.set(rgraphStats.m_gpuTime * 1000.0);

		if(rgraphStats.m_gpuTime > 0.0)
		{
			// WARNING: The name of the event is somewhat special. Search it to see why
			ANKI_TRACE_CUSTOM_EVENT(GpuFrameTime, rgraphStats.m_cpuStartTime, rgraphStats.m_gpuTime);
		}
	}

	return Error::kNone;
}

} // end namespace anki
