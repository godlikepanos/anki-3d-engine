// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Lf.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Renderer.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/LensFlareComponent.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/Functions.h>

namespace anki
{

class Sprite
{
public:
	Vec2 m_pos; ///< Position in NDC
	Vec2 m_scale; ///< Scale of the quad
	Vec4 m_color;
	F32 m_depth; ///< Texture depth
	U32 m_padding[3];
};

Lf::~Lf()
{
	m_queries.destroy(getAllocator());
}

Error Lf::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to init lens flare pass");
	}

	return err;
}

Error Lf::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(initSprite(config));
	ANKI_CHECK(initOcclusion(config));

	getGrManager().finish();
	return ErrorCode::NONE;
}

Error Lf::initSprite(const ConfigSet& config)
{
	m_maxSpritesPerFlare = config.getNumber("lf.maxSpritesPerFlare");
	m_maxFlares = config.getNumber("lf.maxFlares");

	if(m_maxSpritesPerFlare < 1 || m_maxFlares < 1)
	{
		ANKI_LOGE("Incorrect m_maxSpritesPerFlare or m_maxFlares");
		return ErrorCode::USER_DATA;
	}

	m_maxSprites = m_maxSpritesPerFlare * m_maxFlares;

	// Load shaders
	StringAuto pps(getAllocator());

	pps.sprintf("#define MAX_SPRITES %u\n", m_maxSprites);

	ANKI_CHECK(
		getResourceManager().loadResourceToCache(m_realVert, "shaders/LfSpritePass.vert.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(
		getResourceManager().loadResourceToCache(m_realFrag, "shaders/LfSpritePass.frag.glsl", pps.toCString(), "r_"));

	// Create ppline.
	// Writes to IS with blending
	PipelineInitInfo init;
	init.m_inputAssembler.m_topology = PrimitiveTopology::TRIANGLE_STRIP;
	init.m_depthStencil.m_depthWriteEnabled = false;
	init.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;
	init.m_depthStencil.m_format = MS_DEPTH_ATTACHMENT_PIXEL_FORMAT;
	init.m_color.m_attachmentCount = 1;
	init.m_color.m_attachments[0].m_format = IS_COLOR_ATTACHMENT_PIXEL_FORMAT;
	init.m_color.m_attachments[0].m_srcBlendMethod = BlendMethod::ONE;
	init.m_color.m_attachments[0].m_dstBlendMethod = BlendMethod::ONE;
	init.m_shaders[U(ShaderType::VERTEX)] = m_realVert->getGrShader();
	init.m_shaders[U(ShaderType::FRAGMENT)] = m_realFrag->getGrShader();
	m_realPpline = getGrManager().newInstance<Pipeline>(init);

	return ErrorCode::NONE;
}

Error Lf::initOcclusion(const ConfigSet& config)
{
	GrManager& gr = getGrManager();

	m_queries.create(getAllocator(), m_maxFlares);

	m_queryResultBuff = gr.newInstance<Buffer>(m_maxFlares * sizeof(U32),
		BufferUsageBit::STORAGE_COMPUTE_READ | BufferUsageBit::QUERY_RESULT,
		BufferMapAccessBit::NONE);

	m_indirectBuff = gr.newInstance<Buffer>(m_maxFlares * sizeof(DrawArraysIndirectInfo),
		BufferUsageBit::INDIRECT | BufferUsageBit::STORAGE_COMPUTE_WRITE | BufferUsageBit::BUFFER_UPLOAD_DESTINATION,
		BufferMapAccessBit::NONE);

	ANKI_CHECK(getResourceManager().loadResource("shaders/LfOcclusion.vert.glsl", m_occlusionVert));
	ANKI_CHECK(getResourceManager().loadResource("shaders/LfOcclusion.frag.glsl", m_occlusionFrag));
	ANKI_CHECK(getResourceManager().loadResource("shaders/LfUpdateIndirectInfo.comp.glsl", m_writeIndirectBuffComp));

	PipelineInitInfo ppinit;
	ppinit.m_shaders[ShaderType::COMPUTE] = m_writeIndirectBuffComp->getGrShader();
	m_updateIndirectBuffPpline = gr.newInstance<Pipeline>(ppinit);

	ResourceGroupInitInfo rcinit;
	rcinit.m_storageBuffers[0].m_buffer = m_queryResultBuff;
	rcinit.m_storageBuffers[0].m_usage = BufferUsageBit::STORAGE_COMPUTE_READ;
	rcinit.m_storageBuffers[1].m_buffer = m_indirectBuff;
	rcinit.m_storageBuffers[1].m_usage = BufferUsageBit::STORAGE_COMPUTE_WRITE;
	m_updateIndirectBuffRsrc = gr.newInstance<ResourceGroup>(rcinit);

	// Create ppline
	// - only position attribute
	// - points
	// - test depth no write
	// - will run after MS
	// - will not update color
	PipelineInitInfo init;
	init.m_vertex.m_bindingCount = 1;
	init.m_vertex.m_bindings[0].m_stride = sizeof(Vec3);
	init.m_vertex.m_attributeCount = 1;
	init.m_vertex.m_attributes[0].m_format = PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT);
	init.m_inputAssembler.m_topology = PrimitiveTopology::POINTS;
	init.m_depthStencil.m_depthWriteEnabled = false;
	init.m_depthStencil.m_format = MS_DEPTH_ATTACHMENT_PIXEL_FORMAT;
	ANKI_ASSERT(MS_COLOR_ATTACHMENT_COUNT == 3);
	init.m_color.m_attachmentCount = MS_COLOR_ATTACHMENT_COUNT;
	init.m_color.m_attachments[0].m_format = MS_COLOR_ATTACHMENT_PIXEL_FORMATS[0];
	init.m_color.m_attachments[0].m_channelWriteMask = ColorBit::NONE;
	init.m_color.m_attachments[1].m_format = MS_COLOR_ATTACHMENT_PIXEL_FORMATS[1];
	init.m_color.m_attachments[1].m_channelWriteMask = ColorBit::NONE;
	init.m_color.m_attachments[2].m_format = MS_COLOR_ATTACHMENT_PIXEL_FORMATS[2];
	init.m_color.m_attachments[2].m_channelWriteMask = ColorBit::NONE;
	init.m_shaders[U(ShaderType::VERTEX)] = m_occlusionVert->getGrShader();
	init.m_shaders[U(ShaderType::FRAGMENT)] = m_occlusionFrag->getGrShader();
	m_occlusionPpline = gr.newInstance<Pipeline>(init);

	rcinit = ResourceGroupInitInfo();
	rcinit.m_vertexBuffers[0].m_uploadedMemory = true;
	rcinit.m_uniformBuffers[0].m_uploadedMemory = true;
	rcinit.m_uniformBuffers[0].m_usage = BufferUsageBit::UNIFORM_FRAGMENT | BufferUsageBit::UNIFORM_VERTEX;
	m_occlusionRcGroup = gr.newInstance<ResourceGroup>(rcinit);

	return ErrorCode::NONE;
}

void Lf::resetOcclusionQueries(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	const FrustumComponent& camFr = *ctx.m_frustumComponent;
	const VisibilityTestResults& vi = camFr.getVisibilityTestResults();

	if(vi.getCount(VisibilityGroupType::FLARES) > m_maxFlares)
	{
		ANKI_LOGW("Visible flares exceed the limit. Increase lf.maxFlares");
	}

	const U count = min<U>(vi.getCount(VisibilityGroupType::FLARES), m_maxFlares);
	for(U i = 0; i < count; ++i)
	{
		if(!m_queries[i])
		{
			m_queries[i] = getGrManager().newInstance<OcclusionQuery>();
		}

		cmdb->resetOcclusionQuery(m_queries[i]);
	}
}

void Lf::runOcclusionTests(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	// Retrieve some things
	const FrustumComponent& camFr = *ctx.m_frustumComponent;
	const VisibilityTestResults& vi = camFr.getVisibilityTestResults();

	const U count = min<U>(vi.getCount(VisibilityGroupType::FLARES), m_maxFlares);
	Vec3* positions = nullptr;
	const Vec3* initialPositions;
	if(count)
	{
		TransientMemoryInfo dyn;

		// Setup MVP UBO
		Mat4* mvp = static_cast<Mat4*>(getGrManager().allocateFrameTransientMemory(
			sizeof(Mat4), BufferUsageBit::UNIFORM_ALL, dyn.m_uniformBuffers[0]));
		*mvp = camFr.getViewProjectionMatrix();

		// Alloc dyn mem
		positions = static_cast<Vec3*>(getGrManager().allocateFrameTransientMemory(
			sizeof(Vec3) * count, BufferUsageBit::VERTEX, dyn.m_vertexBuffers[0]));
		initialPositions = positions;

		// Setup state
		cmdb->bindPipeline(m_occlusionPpline);
		cmdb->bindResourceGroup(m_occlusionRcGroup, 0, &dyn);
	}

	for(U i = 0; i < count; ++i)
	{
		// Iterate lens flare
		auto it = vi.getBegin(VisibilityGroupType::FLARES) + i;
		const LensFlareComponent& lf = (it->m_node)->getComponent<LensFlareComponent>();

		*positions = lf.getWorldPosition().xyz();

		// Draw and query
		cmdb->beginOcclusionQuery(m_queries[i]);
		cmdb->drawArrays(1, 1, positions - initialPositions);
		cmdb->endOcclusionQuery(m_queries[i]);

		++positions;
	}
}

void Lf::updateIndirectInfo(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	// Retrieve some things
	FrustumComponent& camFr = *ctx.m_frustumComponent;
	VisibilityTestResults& vi = camFr.getVisibilityTestResults();

	U count = min<U>(vi.getCount(VisibilityGroupType::FLARES), m_maxFlares);
	if(count == 0)
	{
		return;
	}

	// Write results to buffer
	for(U i = 0; i < count; ++i)
	{
		cmdb->writeOcclusionQueryResultToBuffer(m_queries[i], sizeof(U32) * i, m_queryResultBuff);
	}

	// Set barrier
	cmdb->setBufferBarrier(m_queryResultBuff,
		BufferUsageBit::QUERY_RESULT,
		BufferUsageBit::STORAGE_COMPUTE_READ,
		0,
		sizeof(DrawArraysIndirectInfo) * count);

	// Update the indirect info
	cmdb->bindPipeline(m_updateIndirectBuffPpline);
	cmdb->bindResourceGroup(m_updateIndirectBuffRsrc, 0, nullptr);
	cmdb->dispatchCompute(count, 1, 1);

	// Set barrier
	cmdb->setBufferBarrier(m_indirectBuff,
		BufferUsageBit::STORAGE_COMPUTE_WRITE,
		BufferUsageBit::INDIRECT,
		0,
		sizeof(DrawArraysIndirectInfo) * count);
}

void Lf::run(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	// Retrieve some things
	FrustumComponent& camFr = *ctx.m_frustumComponent;
	VisibilityTestResults& vi = camFr.getVisibilityTestResults();

	U count = min<U>(vi.getCount(VisibilityGroupType::FLARES), m_maxFlares);
	if(count == 0)
	{
		return;
	}

	cmdb->bindPipeline(m_realPpline);
	for(U i = 0; i < count; ++i)
	{
		auto it = vi.getBegin(VisibilityGroupType::FLARES) + i;
		const LensFlareComponent& lf = (it->m_node)->getComponent<LensFlareComponent>();

		// Compute position
		Vec4 lfPos = Vec4(lf.getWorldPosition().xyz(), 1.0);
		Vec4 posClip = camFr.getViewProjectionMatrix() * lfPos;

		/*if(posClip.x() > posClip.w() || posClip.x() < -posClip.w() || posClip.y() > posClip.w()
			|| posClip.y() < -posClip.w())
		{
			// Outside clip
			ANKI_ASSERT(0 && "Check that before");
		}*/

		U c = 0;
		U spritesCount = max<U>(1, m_maxSpritesPerFlare);

		// Get uniform memory
		TransientMemoryToken token;
		Sprite* tmpSprites = static_cast<Sprite*>(getGrManager().allocateFrameTransientMemory(
			spritesCount * sizeof(Sprite), BufferUsageBit::UNIFORM_ALL, token));
		WeakArray<Sprite> sprites(tmpSprites, spritesCount);

		// misc
		Vec2 posNdc = posClip.xy() / posClip.w();

		// First flare
		sprites[c].m_pos = posNdc;
		sprites[c].m_scale = lf.getFirstFlareSize() * Vec2(1.0, m_r->getAspectRatio());
		sprites[c].m_depth = 0.0;
		F32 alpha = lf.getColorMultiplier().w() * (1.0 - pow(absolute(posNdc.x()), 6.0))
			* (1.0 - pow(absolute(posNdc.y()), 6.0)); // Fade the flare on the edges
		sprites[c].m_color = Vec4(lf.getColorMultiplier().xyz(), alpha);
		++c;

		// Render
		TransientMemoryInfo dyn;
		dyn.m_uniformBuffers[0] = token;
		cmdb->bindResourceGroup(lf.getResourceGroup(), 0, &dyn);
		cmdb->drawArraysIndirect(1, i * sizeof(DrawArraysIndirectInfo), m_indirectBuff);
	}
}

} // end namespace anki
