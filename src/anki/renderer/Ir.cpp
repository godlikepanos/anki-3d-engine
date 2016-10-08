// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

	TransientMemoryToken token;
	Vec3* verts = static_cast<Vec3*>(
		getGrManager().allocateFrameTransientMemory(vertBuffSize, BufferUsageBit::BUFFER_UPLOAD_SOURCE, token));

	const U8* ptr = loader.getVertexData();
	for(U i = 0; i < loader.getHeader().m_totalVerticesCount; ++i)
	{
		*verts = *reinterpret_cast<const Vec3*>(ptr);
		++verts;
		ptr += loader.getVertexSize();
	}

	cmdb->uploadBuffer(vert, 0, token);

	void* cpuIds = getGrManager().allocateFrameTransientMemory(
		loader.getIndexDataSize(), BufferUsageBit::BUFFER_UPLOAD_SOURCE, token);

	memcpy(cpuIds, loader.getIndexData(), loader.getIndexDataSize());

	cmdb->uploadBuffer(idx, 0, token);
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

		face.m_gbufferColorRts[i] = getGrManager().newInstance<Texture>(texinit);
	}

	// Create depth attachment
	texinit.m_usage |= TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;
	texinit.m_format = MS_DEPTH_ATTACHMENT_PIXEL_FORMAT;
	face.m_gbufferDepthRt = getGrManager().newInstance<Texture>(texinit);

	// Create MS FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = MS_COLOR_ATTACHMENT_COUNT;

	for(U j = 0; j < MS_COLOR_ATTACHMENT_COUNT; ++j)
	{
		fbInit.m_colorAttachments[j].m_texture = face.m_gbufferColorRts[j];
		fbInit.m_colorAttachments[j].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		fbInit.m_colorAttachments[j].m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
	}

	fbInit.m_depthStencilAttachment.m_texture = face.m_gbufferDepthRt;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectMask::DEPTH;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;
	fbInit.m_depthStencilAttachment.m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;

	face.m_msFb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Create IS FB
	ANKI_ASSERT(m_is.m_lightRt.isCreated());
	fbInit = FramebufferInitInfo();
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_is.m_lightRt;
	fbInit.m_colorAttachments[0].m_surface.m_layer = cacheEntryIdx;
	fbInit.m_colorAttachments[0].m_surface.m_face = faceIdx;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_colorAttachments[0].m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;
	fbInit.m_depthStencilAttachment.m_texture = face.m_gbufferDepthRt;
	fbInit.m_depthStencilAttachment.m_usageInsideRenderPass =
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ | TextureUsageBit::SAMPLED_FRAGMENT;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectMask::DEPTH;

	face.m_isFb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Create the IS resource group
	ResourceGroupInitInfo rcinit;
	rcinit.m_textures[0].m_texture = face.m_gbufferColorRts[0];
	rcinit.m_textures[1].m_texture = face.m_gbufferColorRts[1];
	rcinit.m_textures[2].m_texture = face.m_gbufferColorRts[2];
	rcinit.m_textures[3].m_texture = face.m_gbufferDepthRt;
	rcinit.m_textures[3].m_usage = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ | TextureUsageBit::SAMPLED_FRAGMENT;

	rcinit.m_uniformBuffers[0].m_uploadedMemory = true;
	rcinit.m_uniformBuffers[0].m_usage = BufferUsageBit::UNIFORM_FRAGMENT | BufferUsageBit::UNIFORM_VERTEX;
	rcinit.m_uniformBuffers[1].m_uploadedMemory = true;
	rcinit.m_uniformBuffers[1].m_usage = BufferUsageBit::UNIFORM_FRAGMENT | BufferUsageBit::UNIFORM_VERTEX;

	rcinit.m_vertexBuffers[0].m_buffer = m_is.m_plightPositions;
	rcinit.m_indexBuffer.m_buffer = m_is.m_plightIndices;
	rcinit.m_indexSize = 2;

	face.m_plightRsrc = getGrManager().newInstance<ResourceGroup>(rcinit);

	rcinit.m_vertexBuffers[0].m_buffer = m_is.m_slightPositions;
	rcinit.m_indexBuffer.m_buffer = m_is.m_slightIndices;
	face.m_slightRsrc = getGrManager().newInstance<ResourceGroup>(rcinit);

	// Create irradiance FB
	fbInit = FramebufferInitInfo();
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_irradiance.m_cubeArr;
	fbInit.m_colorAttachments[0].m_surface.m_layer = cacheEntryIdx;
	fbInit.m_colorAttachments[0].m_surface.m_face = faceIdx;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_colorAttachments[0].m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;

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

	m_is.m_lightRt = getGrManager().newInstance<Texture>(texinit);

	// Clear the texture
	CommandBufferInitInfo cinf;
	CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>(cinf);
	ClearValue clear;
	for(U i = 0; i < m_cubemapArrSize; ++i)
	{
		for(U f = 0; f < 6; ++f)
		{
			for(U l = 0; l < m_is.m_lightRtMipCount; ++l)
			{
				TextureSurfaceInfo surf(l, 0, f, i);

				cmdb->setTextureSurfaceBarrier(m_is.m_lightRt, TextureUsageBit::NONE, TextureUsageBit::CLEAR, surf);

				cmdb->clearTextureSurface(m_is.m_lightRt, surf, clear);

				cmdb->setTextureSurfaceBarrier(
					m_is.m_lightRt, TextureUsageBit::CLEAR, TextureUsageBit::SAMPLED_FRAGMENT, surf);
			}
		}
	}
	cmdb->flush();

	// Init shaders
	ANKI_CHECK(getResourceManager().loadResource("shaders/Light.vert.glsl", m_is.m_lightVert));

	StringAuto pps(getAllocator());
	pps.sprintf("#define POINT_LIGHT\n"
				"#define RENDERING_WIDTH %d\n"
				"#define RENDERING_HEIGHT %d\n",
		m_fbSize,
		m_fbSize);

	ANKI_CHECK(
		getResourceManager().loadResourceToCache(m_is.m_plightFrag, "shaders/Light.frag.glsl", pps.toCString(), "r_"));

	pps.destroy();
	pps.sprintf("#define SPOT_LIGHT\n"
				"#define RENDERING_WIDTH %d\n"
				"#define RENDERING_HEIGHT %d\n",
		m_fbSize,
		m_fbSize);

	ANKI_CHECK(
		getResourceManager().loadResourceToCache(m_is.m_slightFrag, "shaders/Light.frag.glsl", pps.toCString(), "r_"));

	// Init the pplines
	PipelineInitInfo pinit;
	pinit.m_vertex.m_attributeCount = 1;
	pinit.m_vertex.m_attributes[0].m_format = PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT);
	pinit.m_vertex.m_bindingCount = 1;
	pinit.m_vertex.m_bindings[0].m_stride = sizeof(F32) * 3;

	pinit.m_rasterizer.m_cullMode = FaceSelectionMask::FRONT;

	pinit.m_depthStencil.m_depthWriteEnabled = false;
	pinit.m_depthStencil.m_depthCompareFunction = CompareOperation::GREATER;
	pinit.m_depthStencil.m_format = MS_DEPTH_ATTACHMENT_PIXEL_FORMAT;

	pinit.m_color.m_attachmentCount = 1;
	auto& att = pinit.m_color.m_attachments[0];
	att.m_format = IS_COLOR_ATTACHMENT_PIXEL_FORMAT;
	att.m_srcBlendMethod = BlendMethod::ONE;
	att.m_dstBlendMethod = BlendMethod::ONE;

	pinit.m_shaders[ShaderType::VERTEX] = m_is.m_lightVert->getGrShader();
	pinit.m_shaders[ShaderType::FRAGMENT] = m_is.m_plightFrag->getGrShader();

	m_is.m_plightPpline = getGrManager().newInstance<Pipeline>(pinit);

	pinit.m_shaders[ShaderType::FRAGMENT] = m_is.m_slightFrag->getGrShader();
	m_is.m_slightPpline = getGrManager().newInstance<Pipeline>(pinit);

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

	m_irradiance.m_cubeArr = getGrManager().newInstance<Texture>(texinit);

	// Create the shader
	StringAuto pps(getFrameAllocator());
	pps.sprintf("#define CUBEMAP_SIZE %u\n", IRRADIANCE_TEX_SIZE);

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_irradiance.m_frag, "shaders/Irradiance.frag.glsl", pps.toCString(), "r_"));

	// Create the ppline
	ColorStateInfo colorInf;
	colorInf.m_attachmentCount = 1;
	colorInf.m_attachments[0].m_format = IS_COLOR_ATTACHMENT_PIXEL_FORMAT;

	m_r->createDrawQuadPipeline(m_irradiance.m_frag->getGrShader(), colorInf, m_irradiance.m_ppline);

	// Create the resources
	ResourceGroupInitInfo rcInit;
	rcInit.m_uniformBuffers[0].m_uploadedMemory = true;
	rcInit.m_uniformBuffers[0].m_usage = BufferUsageBit::UNIFORM_FRAGMENT | BufferUsageBit::UNIFORM_VERTEX;
	rcInit.m_textures[0].m_texture = m_is.m_lightRt;

	m_irradiance.m_rsrc = getGrManager().newInstance<ResourceGroup>(rcInit);

	// Clear texture
	CommandBufferInitInfo cinf;
	cinf.m_flags = CommandBufferFlag::SMALL_BATCH;
	CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>(cinf);
	ClearValue clear;
	for(U i = 0; i < m_cubemapArrSize; ++i)
	{
		for(U f = 0; f < 6; ++f)
		{
			for(U l = 0; l < m_irradiance.m_cubeArrMipCount; ++l)
			{
				TextureSurfaceInfo surf(l, 0, f, i);

				cmdb->setTextureSurfaceBarrier(m_is.m_lightRt, TextureUsageBit::NONE, TextureUsageBit::CLEAR, surf);

				cmdb->clearTextureSurface(m_irradiance.m_cubeArr, surf, clear);

				cmdb->setTextureSurfaceBarrier(
					m_is.m_lightRt, TextureUsageBit::CLEAR, TextureUsageBit::SAMPLED_FRAGMENT, surf);
			}
		}
	}
	cmdb->flush();

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
	cmdb->setPolygonOffset(0.0, 0.0);

	/// Draw
	ANKI_CHECK(m_r->getSceneDrawer().drawRange(Pass::MS_FS,
		frc,
		cmdb,
		*m_r->getMs().m_pplineCache,
		m_r->getMs().m_state,
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

	cmdb->beginRenderPass(face.m_isFb);

	// Process all lights
	const Mat4& vpMat = frc.getViewProjectionMatrix();
	const Mat4& vMat = frc.getViewMatrix();

	cmdb->bindPipeline(m_is.m_plightPpline);

	const VisibleNode* it = vis.getBegin(VisibilityGroupType::LIGHTS_POINT);
	const VisibleNode* end = vis.getEnd(VisibilityGroupType::LIGHTS_POINT);
	while(it != end)
	{
		const LightComponent& lightc = it->m_node->getComponent<LightComponent>();
		const MoveComponent& movec = it->m_node->getComponent<MoveComponent>();

		TransientMemoryInfo transient;

		// Update uniforms
		IrVertex* vert = static_cast<IrVertex*>(getGrManager().allocateFrameTransientMemory(
			sizeof(IrVertex), BufferUsageBit::UNIFORM_ALL, transient.m_uniformBuffers[0]));

		Mat4 modelM(movec.getWorldTransform().getOrigin().xyz1(),
			movec.getWorldTransform().getRotation().getRotationPart(),
			lightc.getRadius());

		vert->m_mvp = vpMat * modelM;

		IrPointLight* light = static_cast<IrPointLight*>(getGrManager().allocateFrameTransientMemory(
			sizeof(IrPointLight), BufferUsageBit::UNIFORM_ALL, transient.m_uniformBuffers[1]));

		Vec4 pos = vMat * movec.getWorldTransform().getOrigin().xyz1();

		light->m_projectionParams = frc.getProjectionParameters();
		light->m_posRadius = Vec4(pos.xyz(), 1.0 / (lightc.getRadius() * lightc.getRadius()));
		light->m_diffuseColorPad1 = lightc.getDiffuseColor();
		light->m_specularColorPad1 = lightc.getSpecularColor();

		// Bind stuff
		cmdb->bindResourceGroup(face.m_plightRsrc, 0, &transient);

		// Draw
		cmdb->drawElements(m_is.m_plightIdxCount);

		++it;
	}

	cmdb->bindPipeline(m_is.m_slightPpline);

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

		TransientMemoryInfo transient;

		// Update vertex uniforms
		IrVertex* vert = static_cast<IrVertex*>(getGrManager().allocateFrameTransientMemory(
			sizeof(IrVertex), BufferUsageBit::UNIFORM_ALL, transient.m_uniformBuffers[0]));

		vert->m_mvp = vpMat * modelM;

		// Update fragment uniforms
		IrSpotLight* light = static_cast<IrSpotLight*>(getGrManager().allocateFrameTransientMemory(
			sizeof(IrSpotLight), BufferUsageBit::UNIFORM_ALL, transient.m_uniformBuffers[1]));

		light->m_projectionParams = frc.getProjectionParameters();

		Vec4 pos = vMat * movec.getWorldTransform().getOrigin().xyz1();
		light->m_posRadius = Vec4(pos.xyz(), 1.0 / (lightc.getDistance() * lightc.getDistance()));

		light->m_diffuseColorOuterCos = Vec4(lightc.getDiffuseColor().xyz(), lightc.getOuterAngleCos());

		light->m_specularColorInnerCos = Vec4(lightc.getSpecularColor().xyz(), lightc.getInnerAngleCos());

		Vec3 lightDir = -movec.getWorldTransform().getRotation().getZAxis();
		lightDir = vMat.getRotationPart() * lightDir;
		light->m_lightDirPad1 = lightDir.xyz0();

		// Bind stuff
		cmdb->bindResourceGroup(face.m_slightRsrc, 0, &transient);

		// Draw
		cmdb->drawElements(m_is.m_slightIdxCount);

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

	TransientMemoryInfo dinf;
	UVec4* faceIdxArrayIdx = static_cast<UVec4*>(getGrManager().allocateFrameTransientMemory(
		sizeof(UVec4), BufferUsageBit::UNIFORM_ALL, dinf.m_uniformBuffers[0]));
	faceIdxArrayIdx->x() = faceIdx;
	faceIdxArrayIdx->y() = layer;

	cmdb->bindResourceGroup(m_irradiance.m_rsrc, 0, &dinf);
	cmdb->bindPipeline(m_irradiance.m_ppline);
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
	FrustumComponent& frc = *rctx.m_frustumComponent;
	VisibilityTestResults& visRez = frc.getVisibilityTestResults();

	if(visRez.getCount(VisibilityGroupType::REFLECTION_PROBES) > m_cubemapArrSize)
	{
		ANKI_LOGW("Increase the ir.cubemapTextureArraySize");
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
