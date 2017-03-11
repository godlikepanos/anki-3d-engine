// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ir.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Pps.h>
#include <anki/renderer/Ms.h>
#include <anki/core/Config.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/Visibility.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/LightComponent.h>
#include <anki/scene/ReflectionProbeComponent.h>
#include <anki/core/Trace.h>
#include <anki/resource/MeshLoader.h>

namespace anki
{

class IrVertex
{
public:
	Mat4 m_mvp;
};

class IrPointLight
{
public:
	Vec4 m_projectionParams;
	Vec4 m_posRadius;
	Vec4 m_diffuseColorPad1;
	Vec4 m_specularColorPad1;
};

class IrSpotLight
{
public:
	Vec4 m_projectionParams;
	Vec4 m_posRadius;
	Vec4 m_diffuseColorOuterCos;
	Vec4 m_specularColorInnerCos;
	Vec4 m_lightDirPad1;
};

Ir::Ir(Renderer* r)
	: RenderingPass(r)
{
}

Ir::~Ir()
{
	m_cacheEntries.destroy(getAllocator());
}

Error Ir::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing image reflections");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize image reflections");
	}

	return err;
}

Error Ir::initInternal(const ConfigSet& config)
{
	m_fbSize = config.getNumber("ir.rendererSize");

	if(m_fbSize < TILE_SIZE)
	{
		ANKI_R_LOGE("Too low ir.rendererSize");
		return ErrorCode::USER_DATA;
	}

	m_cubemapArrSize = config.getNumber("ir.cubemapTextureArraySize");

	if(m_cubemapArrSize < 2)
	{
		ANKI_R_LOGE("Too low ir.cubemapTextureArraySize");
		return ErrorCode::USER_DATA;
	}

	m_cacheEntries.create(getAllocator(), m_cubemapArrSize);

	ANKI_CHECK(initIs());
	ANKI_CHECK(initIrradiance());

	// Load split sum integration LUT
	ANKI_CHECK(getResourceManager().loadResource("engine_data/SplitSumIntegration.ankitex", m_integrationLut));

	SamplerInitInfo sinit;
	sinit.m_minMagFilter = SamplingFilter::LINEAR;
	sinit.m_mipmapFilter = SamplingFilter::BASE;
	sinit.m_minLod = 0.0;
	sinit.m_maxLod = 1.0;
	sinit.m_repeat = false;
	m_integrationLutSampler = getGrManager().newInstance<Sampler>(sinit);

	return ErrorCode::NONE;
}

Error Ir::loadMesh(CString fname, BufferPtr& vert, BufferPtr& idx, U32& idxCount)
{
	MeshLoader loader(&getResourceManager());
	ANKI_CHECK(loader.load(fname));

	PtrSize vertBuffSize = loader.getHeader().m_totalVerticesCount * sizeof(Vec3);
	vert = getGrManager().newInstance<Buffer>(
		vertBuffSize, BufferUsageBit::VERTEX | BufferUsageBit::BUFFER_UPLOAD_DESTINATION, BufferMapAccessBit::NONE);

	idx = getGrManager().newInstance<Buffer>(loader.getIndexDataSize(),
		BufferUsageBit::INDEX | BufferUsageBit::BUFFER_UPLOAD_DESTINATION,
		BufferMapAccessBit::NONE);

	// Upload data
	CommandBufferInitInfo init;
	init.m_flags = CommandBufferFlag::SMALL_BATCH;
	CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>(init);

	StagingGpuMemoryToken token;
	Vec3* verts = static_cast<Vec3*>(
		m_r->getStagingGpuMemoryManager().allocateFrame(vertBuffSize, StagingGpuMemoryType::TRANSFER, token));

	const U8* ptr = loader.getVertexData();
	for(U i = 0; i < loader.getHeader().m_totalVerticesCount; ++i)
	{
		*verts = *reinterpret_cast<const Vec3*>(ptr);
		++verts;
		ptr += loader.getVertexSize();
	}

	cmdb->copyBufferToBuffer(token.m_buffer, token.m_offset, vert, 0, token.m_range);

	void* cpuIds = m_r->getStagingGpuMemoryManager().allocateFrame(
		loader.getIndexDataSize(), StagingGpuMemoryType::TRANSFER, token);

	memcpy(cpuIds, loader.getIndexData(), loader.getIndexDataSize());

	cmdb->copyBufferToBuffer(token.m_buffer, token.m_offset, idx, 0, token.m_range);
	idxCount = loader.getHeader().m_totalIndicesCount;

	cmdb->flush();

	return ErrorCode::NONE;
}

void Ir::initFaceInfo(U cacheEntryIdx, U faceIdx)
{
	FaceInfo& face = m_cacheEntries[cacheEntryIdx].m_faces[faceIdx];
	ANKI_ASSERT(!face.created());

	TextureInitInfo texinit;

	texinit.m_width = m_fbSize;
	texinit.m_height = m_fbSize;
	texinit.m_layerCount = 1;
	texinit.m_depth = 1;
	texinit.m_type = TextureType::_2D;
	texinit.m_mipmapsCount = 1;
	texinit.m_samples = 1;
	texinit.m_sampling.m_minMagFilter = SamplingFilter::NEAREST;
	texinit.m_sampling.m_mipmapFilter = SamplingFilter::NEAREST;
	texinit.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;

	// Create color attachments
	for(U i = 0; i < MS_COLOR_ATTACHMENT_COUNT; ++i)
	{
		texinit.m_format = MS_COLOR_ATTACHMENT_PIXEL_FORMATS[i];

		face.m_gbufferColorRts[i] = m_r->createAndClearRenderTarget(texinit);
	}

	// Create depth attachment
	texinit.m_usage |= TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;
	texinit.m_format = MS_DEPTH_ATTACHMENT_PIXEL_FORMAT;
	face.m_gbufferDepthRt = m_r->createAndClearRenderTarget(texinit);

	// Create MS FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = MS_COLOR_ATTACHMENT_COUNT;

	for(U j = 0; j < MS_COLOR_ATTACHMENT_COUNT; ++j)
	{
		fbInit.m_colorAttachments[j].m_texture = face.m_gbufferColorRts[j];
		fbInit.m_colorAttachments[j].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	}

	fbInit.m_depthStencilAttachment.m_texture = face.m_gbufferDepthRt;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;

	face.m_msFb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Create IS FB
	ANKI_ASSERT(m_is.m_lightRt.isCreated());
	fbInit = FramebufferInitInfo();
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_is.m_lightRt;
	fbInit.m_colorAttachments[0].m_surface.m_layer = cacheEntryIdx;
	fbInit.m_colorAttachments[0].m_surface.m_face = faceIdx;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;
	fbInit.m_depthStencilAttachment.m_texture = face.m_gbufferDepthRt;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;

	face.m_isFb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Create irradiance FB
	fbInit = FramebufferInitInfo();
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_irradiance.m_cubeArr;
	fbInit.m_colorAttachments[0].m_surface.m_layer = cacheEntryIdx;
	fbInit.m_colorAttachments[0].m_surface.m_face = faceIdx;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;

	face.m_irradianceFb = getGrManager().newInstance<Framebuffer>(fbInit);
}

Error Ir::initIs()
{
	m_is.m_lightRtMipCount = computeMaxMipmapCount2d(m_fbSize, m_fbSize, 4);

	// Init texture
	TextureInitInfo texinit;
	texinit.m_width = m_fbSize;
	texinit.m_height = m_fbSize;
	texinit.m_layerCount = m_cubemapArrSize;
	texinit.m_depth = 1;
	texinit.m_type = TextureType::CUBE_ARRAY;
	texinit.m_mipmapsCount = m_is.m_lightRtMipCount;
	texinit.m_samples = 1;
	texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	texinit.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;
	texinit.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE
		| TextureUsageBit::CLEAR | TextureUsageBit::GENERATE_MIPMAPS;
	texinit.m_format = IS_COLOR_ATTACHMENT_PIXEL_FORMAT;

	m_is.m_lightRt = m_r->createAndClearRenderTarget(texinit);

	// Init shaders
	ANKI_CHECK(getResourceManager().loadResource("shaders/Light.vert.glsl", m_is.m_lightVert));

	ANKI_CHECK(m_r->createShaderf("shaders/Light.frag.glsl",
		m_is.m_plightFrag,
		"#define POINT_LIGHT\n"
		"#define RENDERING_WIDTH %d\n"
		"#define RENDERING_HEIGHT %d\n",
		m_fbSize,
		m_fbSize));

	ANKI_CHECK(m_r->createShaderf("shaders/Light.frag.glsl",
		m_is.m_slightFrag,
		"#define SPOT_LIGHT\n"
		"#define RENDERING_WIDTH %d\n"
		"#define RENDERING_HEIGHT %d\n",
		m_fbSize,
		m_fbSize));

	// Init the progs
	m_is.m_plightProg =
		getGrManager().newInstance<ShaderProgram>(m_is.m_lightVert->getGrShader(), m_is.m_plightFrag->getGrShader());

	m_is.m_slightProg =
		getGrManager().newInstance<ShaderProgram>(m_is.m_lightVert->getGrShader(), m_is.m_slightFrag->getGrShader());

	// Init vert/idx buffers
	ANKI_CHECK(
		loadMesh("engine_data/Plight.ankimesh", m_is.m_plightPositions, m_is.m_plightIndices, m_is.m_plightIdxCount));

	ANKI_CHECK(
		loadMesh("engine_data/Slight.ankimesh", m_is.m_slightPositions, m_is.m_slightIndices, m_is.m_slightIdxCount));

	return ErrorCode::NONE;
}

Error Ir::initIrradiance()
{
	m_irradiance.m_cubeArrMipCount = computeMaxMipmapCount2d(IRRADIANCE_TEX_SIZE, IRRADIANCE_TEX_SIZE, 4);

	// Init texture
	TextureInitInfo texinit;
	texinit.m_width = IRRADIANCE_TEX_SIZE;
	texinit.m_height = IRRADIANCE_TEX_SIZE;
	texinit.m_layerCount = m_cubemapArrSize;
	texinit.m_depth = 1;
	texinit.m_type = TextureType::CUBE_ARRAY;
	texinit.m_mipmapsCount = m_irradiance.m_cubeArrMipCount;
	texinit.m_samples = 1;
	texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	texinit.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;
	texinit.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
		| TextureUsageBit::CLEAR | TextureUsageBit::GENERATE_MIPMAPS;
	texinit.m_format = IS_COLOR_ATTACHMENT_PIXEL_FORMAT;

	m_irradiance.m_cubeArr = m_r->createAndClearRenderTarget(texinit);

	// Create the shader
	ANKI_CHECK(m_r->createShaderf(
		"shaders/Irradiance.frag.glsl", m_irradiance.m_frag, "#define CUBEMAP_SIZE %u\n", IRRADIANCE_TEX_SIZE));

	// Create the prog
	m_r->createDrawQuadShaderProgram(m_irradiance.m_frag->getGrShader(), m_irradiance.m_prog);

	return ErrorCode::NONE;
}

Error Ir::runMs(RenderingContext& rctx, FrustumComponent& frc, U layer, U faceIdx)
{
	CommandBufferPtr& cmdb = rctx.m_commandBuffer;
	VisibilityTestResults& vis = frc.getVisibilityTestResults();

	FaceInfo& face = m_cacheEntries[layer].m_faces[faceIdx];

	if(!face.created())
	{
		initFaceInfo(layer, faceIdx);
	}

	// Set barriers
	for(U i = 0; i < MS_COLOR_ATTACHMENT_COUNT; ++i)
	{
		cmdb->setTextureSurfaceBarrier(face.m_gbufferColorRts[i],
			TextureUsageBit::NONE,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureSurfaceInfo(0, 0, 0, 0));
	}

	cmdb->setTextureSurfaceBarrier(face.m_gbufferDepthRt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));

	// Start render pass
	cmdb->beginRenderPass(face.m_msFb);
	cmdb->setViewport(0, 0, m_fbSize, m_fbSize);

	/// Draw
	ANKI_CHECK(m_r->getSceneDrawer().drawRange(Pass::MS_FS,
		frc.getViewMatrix(),
		frc.getViewProjectionMatrix(),
		cmdb,
		vis.getBegin(VisibilityGroupType::RENDERABLES_MS),
		vis.getEnd(VisibilityGroupType::RENDERABLES_MS)));

	// End and set barriers
	cmdb->endRenderPass();

	for(U i = 0; i < MS_COLOR_ATTACHMENT_COUNT; ++i)
	{
		cmdb->setTextureSurfaceBarrier(face.m_gbufferColorRts[i],
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, 0));
	}

	cmdb->setTextureSurfaceBarrier(face.m_gbufferDepthRt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ,
		TextureSurfaceInfo(0, 0, 0, 0));

	return ErrorCode::NONE;
}

void Ir::runIs(RenderingContext& rctx, FrustumComponent& frc, U layer, U faceIdx)
{
	CommandBufferPtr& cmdb = rctx.m_commandBuffer;
	VisibilityTestResults& vis = frc.getVisibilityTestResults();
	FaceInfo& face = m_cacheEntries[layer].m_faces[faceIdx];

	// Set barriers
	cmdb->setTextureSurfaceBarrier(m_is.m_lightRt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, faceIdx, layer));

	// Set state
	cmdb->beginRenderPass(face.m_isFb);

	cmdb->bindTexture(0, 0, face.m_gbufferColorRts[0]);
	cmdb->bindTexture(0, 1, face.m_gbufferColorRts[1]);
	cmdb->bindTexture(0, 2, face.m_gbufferColorRts[2]);
	cmdb->bindTexture(0, 3, face.m_gbufferDepthRt);

	cmdb->setVertexAttribute(0, 0, PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT), 0);

	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);
	cmdb->setDepthCompareOperation(CompareOperation::GREATER);
	cmdb->setDepthWrite(false);
	cmdb->setCullMode(FaceSelectionBit::FRONT);

	// Process all lights
	const Mat4& vpMat = frc.getViewProjectionMatrix();
	const Mat4& vMat = frc.getViewMatrix();

	cmdb->bindShaderProgram(m_is.m_plightProg);
	cmdb->bindVertexBuffer(0, m_is.m_plightPositions, 0, sizeof(F32) * 3);
	cmdb->bindIndexBuffer(m_is.m_plightIndices, 0, IndexType::U16);

	const VisibleNode* it = vis.getBegin(VisibilityGroupType::LIGHTS_POINT);
	const VisibleNode* end = vis.getEnd(VisibilityGroupType::LIGHTS_POINT);
	while(it != end)
	{
		const LightComponent& lightc = it->m_node->getComponent<LightComponent>();
		const MoveComponent& movec = it->m_node->getComponent<MoveComponent>();

		// Update uniforms
		IrVertex* vert = allocateAndBindUniforms<IrVertex*>(sizeof(IrVertex), cmdb, 0, 0);

		Mat4 modelM(movec.getWorldTransform().getOrigin().xyz1(),
			movec.getWorldTransform().getRotation().getRotationPart(),
			lightc.getRadius());

		vert->m_mvp = vpMat * modelM;

		IrPointLight* light = allocateAndBindUniforms<IrPointLight*>(sizeof(IrPointLight), cmdb, 0, 1);

		Vec4 pos = vMat * movec.getWorldTransform().getOrigin().xyz1();

		light->m_projectionParams = frc.getProjectionMatrix().extractPerspectiveUnprojectionParams();
		light->m_posRadius = Vec4(pos.xyz(), 1.0 / (lightc.getRadius() * lightc.getRadius()));
		light->m_diffuseColorPad1 = lightc.getDiffuseColor();
		light->m_specularColorPad1 = lightc.getSpecularColor();

		// Draw
		cmdb->drawElements(PrimitiveTopology::TRIANGLES, m_is.m_plightIdxCount);

		++it;
	}

	cmdb->bindShaderProgram(m_is.m_slightProg);
	cmdb->bindVertexBuffer(0, m_is.m_slightPositions, 0, sizeof(F32) * 3);
	cmdb->bindIndexBuffer(m_is.m_slightIndices, 0, IndexType::U16);

	it = vis.getBegin(VisibilityGroupType::LIGHTS_SPOT);
	end = vis.getEnd(VisibilityGroupType::LIGHTS_SPOT);
	while(it != end)
	{
		const LightComponent& lightc = it->m_node->getComponent<LightComponent>();
		const MoveComponent& movec = it->m_node->getComponent<MoveComponent>();

		// Compute the model matrix
		//
		Mat4 modelM(movec.getWorldTransform().getOrigin().xyz1(),
			movec.getWorldTransform().getRotation().getRotationPart(),
			1.0);

		// Calc the scale of the cone
		Mat4 scaleM(Mat4::getIdentity());
		scaleM(0, 0) = tan(lightc.getOuterAngle() / 2.0) * lightc.getDistance();
		scaleM(1, 1) = scaleM(0, 0);
		scaleM(2, 2) = lightc.getDistance();

		modelM = modelM * scaleM;

		// Update vertex uniforms
		IrVertex* vert = allocateAndBindUniforms<IrVertex*>(sizeof(IrVertex), cmdb, 0, 0);
		vert->m_mvp = vpMat * modelM;

		// Update fragment uniforms
		IrSpotLight* light = allocateAndBindUniforms<IrSpotLight*>(sizeof(IrSpotLight), cmdb, 0, 1);

		light->m_projectionParams = frc.getProjectionMatrix().extractPerspectiveUnprojectionParams();

		Vec4 pos = vMat * movec.getWorldTransform().getOrigin().xyz1();
		light->m_posRadius = Vec4(pos.xyz(), 1.0 / (lightc.getDistance() * lightc.getDistance()));

		light->m_diffuseColorOuterCos = Vec4(lightc.getDiffuseColor().xyz(), lightc.getOuterAngleCos());

		light->m_specularColorInnerCos = Vec4(lightc.getSpecularColor().xyz(), lightc.getInnerAngleCos());

		Vec3 lightDir = -movec.getWorldTransform().getRotation().getZAxis();
		lightDir = vMat.getRotationPart() * lightDir;
		light->m_lightDirPad1 = lightDir.xyz0();

		// Draw
		cmdb->drawElements(PrimitiveTopology::TRIANGLES, m_is.m_slightIdxCount);

		++it;
	}

	// Generate mips
	cmdb->endRenderPass();

	cmdb->setTextureSurfaceBarrier(m_is.m_lightRt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::GENERATE_MIPMAPS,
		TextureSurfaceInfo(0, 0, faceIdx, layer));

	cmdb->generateMipmaps2d(m_is.m_lightRt, faceIdx, layer);

	cmdb->setTextureSurfaceBarrier(m_is.m_lightRt,
		TextureUsageBit::GENERATE_MIPMAPS,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, faceIdx, layer));

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setDepthCompareOperation(CompareOperation::LESS);
	cmdb->setDepthWrite(true);
	cmdb->setCullMode(FaceSelectionBit::BACK);
}

void Ir::computeIrradiance(RenderingContext& rctx, U layer, U faceIdx)
{
	CommandBufferPtr& cmdb = rctx.m_commandBuffer;
	FaceInfo& face = m_cacheEntries[layer].m_faces[faceIdx];

	// Set barrier
	cmdb->setTextureSurfaceBarrier(m_irradiance.m_cubeArr,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, faceIdx, layer));

	// Set state and draw
	cmdb->setViewport(0, 0, IRRADIANCE_TEX_SIZE, IRRADIANCE_TEX_SIZE);

	UVec4* faceIdxArrayIdx = allocateAndBindUniforms<UVec4*>(sizeof(UVec4), cmdb, 0, 0);
	faceIdxArrayIdx->x() = faceIdx;
	faceIdxArrayIdx->y() = layer;

	cmdb->bindTexture(0, 0, m_is.m_lightRt);
	cmdb->bindShaderProgram(m_irradiance.m_prog);
	cmdb->beginRenderPass(face.m_irradianceFb);

	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();

	// Gen mips
	cmdb->setTextureSurfaceBarrier(m_irradiance.m_cubeArr,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::GENERATE_MIPMAPS,
		TextureSurfaceInfo(0, 0, faceIdx, layer));

	cmdb->generateMipmaps2d(m_irradiance.m_cubeArr, faceIdx, layer);

	cmdb->setTextureSurfaceBarrier(m_irradiance.m_cubeArr,
		TextureUsageBit::GENERATE_MIPMAPS,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, faceIdx, layer));
}

Error Ir::run(RenderingContext& rctx)
{
	ANKI_TRACE_START_EVENT(RENDER_IR);
	const VisibilityTestResults& visRez = *rctx.m_visResults;

	if(visRez.getCount(VisibilityGroupType::REFLECTION_PROBES) > m_cubemapArrSize)
	{
		ANKI_R_LOGW("Increase the ir.cubemapTextureArraySize");
	}

	// Render some of the probes
	const VisibleNode* it = visRez.getBegin(VisibilityGroupType::REFLECTION_PROBES);
	const VisibleNode* end = visRez.getEnd(VisibilityGroupType::REFLECTION_PROBES);

	U probesRendered = 0;
	U probeIdx = 0;
	while(it != end)
	{
		// Write and render probe
		ANKI_CHECK(tryRender(rctx, *it->m_node, probesRendered));

		++it;
		++probeIdx;
	}
	ANKI_ASSERT(probeIdx == visRez.getCount(VisibilityGroupType::REFLECTION_PROBES));

	// Inform on tex usage
	CommandBufferPtr& cmdb = rctx.m_commandBuffer;
	cmdb->informTextureCurrentUsage(m_irradiance.m_cubeArr, TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->informTextureCurrentUsage(m_is.m_lightRt, TextureUsageBit::SAMPLED_FRAGMENT);

	// Bye
	ANKI_TRACE_STOP_EVENT(RENDER_IR);
	return ErrorCode::NONE;
}

Error Ir::tryRender(RenderingContext& ctx, SceneNode& node, U& probesRendered)
{
	ReflectionProbeComponent& reflc = node.getComponent<ReflectionProbeComponent>();

	Bool render = false;
	U entry;
	findCacheEntry(node, entry, render);

	// Write shader var
	reflc.setTextureArrayIndex(entry);

	if(reflc.getMarkedForRendering() && probesRendered < MAX_PROBE_RENDERS_PER_FRAME)
	{
		++probesRendered;
		reflc.setMarkedForRendering(false);
		ANKI_CHECK(renderReflection(ctx, node, entry));
	}

	// If you need to render it mark it for the next frame
	if(render)
	{
		reflc.setMarkedForRendering(true);
	}

	return ErrorCode::NONE;
}

Error Ir::renderReflection(RenderingContext& ctx, SceneNode& node, U cubemapIdx)
{
	ANKI_TRACE_INC_COUNTER(RENDERER_REFLECTIONS, 1);

	// Gather the frustum components
	Array<FrustumComponent*, 6> frustumComponents;
	U count = 0;
	Error err = node.iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
		frustumComponents[count++] = &frc;
		return ErrorCode::NONE;
	});
	(void)err;
	ANKI_ASSERT(count == 6);

	// Render cubemap
	for(U i = 0; i < 6; ++i)
	{
		ANKI_CHECK(runMs(ctx, *frustumComponents[i], cubemapIdx, i));
		runIs(ctx, *frustumComponents[i], cubemapIdx, i);
	}

	for(U i = 0; i < 6; ++i)
	{
		computeIrradiance(ctx, cubemapIdx, i);
	}

	return ErrorCode::NONE;
}

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
		if(it->m_nodeUuid == node.getUuid())
		{
			// Already there
			ANKI_ASSERT(it->m_node == &node);
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
		render = m_r->resourcesLoaded();
	}
	else if(empty)
	{
		ANKI_ASSERT(empty->m_node == nullptr);
		empty->m_node = &node;
		empty->m_nodeUuid = node.getUuid();
		empty->m_timestamp = m_r->getFrameCount();

		it = empty;
		render = true;
	}
	else
	{
		ANKI_ASSERT(kick);

		kick->m_node = &node;
		kick->m_nodeUuid = node.getUuid();
		kick->m_timestamp = m_r->getFrameCount();

		it = kick;
		render = true;
	}

	entry = it - m_cacheEntries.getBegin();
}

} // end namespace anki
