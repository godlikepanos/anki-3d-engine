// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Lf.h"
#include "anki/renderer/Bloom.h"
#include "anki/renderer/Is.h"
#include "anki/renderer/Ms.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/LensFlareComponent.h"
#include "anki/scene/Camera.h"
#include "anki/misc/ConfigSet.h"
#include "anki/util/Functions.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
struct Sprite
{
	Vec2 m_pos; ///< Position in NDC
	Vec2 m_scale; ///< Scale of the quad
	F32 m_alpha; ///< Alpha value
	F32 m_depth; ///< Texture depth
	U32 m_padding[2];
};

//==============================================================================
// Lf                                                                          =
//==============================================================================

//==============================================================================
Lf::~Lf()
{}

//==============================================================================
Error Lf::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to init lens flare pass");
	}

	return err;
}

//==============================================================================
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

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_realVert,	"shaders/LfSpritePass.vert.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_realFrag,	"shaders/LfSpritePass.frag.glsl", pps.toCString(), "r_"));

	// Create ppline.
	// Writes to IS with blending
	PipelineInitializer init;
	init.m_inputAssembler.m_topology = PrimitiveTopology::TRIANGLE_STRIP;
	init.m_depthStencil.m_depthWriteEnabled = false;
	init.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;
	init.m_color.m_attachmentCount = 1;
	init.m_color.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;
	init.m_color.m_attachments[0].m_srcBlendMethod = BlendMethod::ONE;
	init.m_color.m_attachments[0].m_dstBlendMethod = BlendMethod::ONE;
	init.m_shaders[U(ShaderType::VERTEX)] = m_realVert->getGrShader();
	init.m_shaders[U(ShaderType::FRAGMENT)] = m_realFrag->getGrShader();
	m_realPpline= getGrManager().newInstance<Pipeline>(init);

	return ErrorCode::NONE;
}

//==============================================================================
Error Lf::initOcclusion(const ConfigSet& config)
{
	// Init vert buff
	m_positionsVertBuffSize = sizeof(Vec3) * m_maxFlares;

	for(U i = 0; i < m_positionsVertBuff.getSize(); ++i)
	{
		m_positionsVertBuff[i] = getGrManager().newInstance<Buffer>(
			m_positionsVertBuffSize, BufferUsageBit::VERTEX,
			BufferAccessBit::CLIENT_MAP_WRITE);
	}

	// Shaders
	ANKI_CHECK(getResourceManager().loadResource(
		"shaders/LfOcclusion.vert.glsl", m_occlusionVert));

	ANKI_CHECK(getResourceManager().loadResource(
		"shaders/LfOcclusion.frag.glsl", m_occlusionFrag));

	// Create ppline
	// - only position attribute
	// - points
	// - test depth no write
	// - will run after MS
	// - will not update color
	PipelineInitializer init;
	init.m_vertex.m_bindingCount = 1;
	init.m_vertex.m_bindings[0].m_stride = sizeof(Vec3);
	init.m_vertex.m_attributeCount = 1;
	init.m_vertex.m_attributes[0].m_format =
		PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT);
	init.m_inputAssembler.m_topology = PrimitiveTopology::POINTS;
	init.m_depthStencil.m_depthWriteEnabled = false;
	init.m_depthStencil.m_format = Ms::DEPTH_RT_PIXEL_FORMAT;
	ANKI_ASSERT(Ms::ATTACHMENT_COUNT == 3);
	init.m_color.m_attachmentCount = Ms::ATTACHMENT_COUNT;
	init.m_color.m_attachments[0].m_format = Ms::RT_PIXEL_FORMATS[0];
	init.m_color.m_attachments[0].m_channelWriteMask = ColorBit::NONE;
	init.m_color.m_attachments[1].m_format = Ms::RT_PIXEL_FORMATS[1];
	init.m_color.m_attachments[1].m_channelWriteMask = ColorBit::NONE;
	init.m_color.m_attachments[2].m_format = Ms::RT_PIXEL_FORMATS[2];
	init.m_color.m_attachments[2].m_channelWriteMask = ColorBit::NONE;
	init.m_shaders[U(ShaderType::VERTEX)] = m_occlusionVert->getGrShader();
	init.m_shaders[U(ShaderType::FRAGMENT)] = m_occlusionFrag->getGrShader();
	m_occlusionPpline = getGrManager().newInstance<Pipeline>(init);

	// Init resource group
	for(U i = 0; i < m_occlusionRcGroups.getSize(); ++i)
	{
		ResourceGroupInitializer rcInit;
		rcInit.m_vertexBuffers[0].m_buffer = m_positionsVertBuff[i];
		m_occlusionRcGroups[i] =
			getGrManager().newInstance<ResourceGroup>(rcInit);
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error Lf::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(initSprite(config));
	ANKI_CHECK(initOcclusion(config));

	return ErrorCode::NONE;
}

//==============================================================================
void Lf::runOcclusionTests(CommandBufferPtr& cmdb)
{
	// Retrieve some things
	FrustumComponent& camFr =
		m_r->getActiveCamera().getComponent<FrustumComponent>();
	VisibilityTestResults& vi = camFr.getVisibilityTestResults();

	U totalCount = min<U>(vi.getLensFlaresCount(), m_maxFlares);
	if(totalCount > 0)
	{
		ANKI_ASSERT(sizeof(Vec3) * totalCount <= m_positionsVertBuffSize);

		if(vi.getLensFlaresCount() > m_maxFlares)
		{
			ANKI_LOGW("Visible flares exceed the limit");
		}

		U frame = getGlobalTimestamp() % m_positionsVertBuff.getSize();

		// Setup state
		cmdb->bindPipeline(m_occlusionPpline);
		cmdb->bindResourceGroup(m_occlusionRcGroups[frame]);

		// Setup MVP UBO
		Mat4* mvp = nullptr;
		cmdb->updateDynamicUniforms(sizeof(Mat4), mvp);
		*mvp = camFr.getViewProjectionMatrix().getTransposed();

		// Allocate vertices and fire write job
		BufferPtr& positionsVertBuff = m_positionsVertBuff[frame];

		Vec3* positions = static_cast<Vec3*>(positionsVertBuff->map(
			0, sizeof(Vec3) * totalCount, BufferAccessBit::CLIENT_MAP_WRITE));
		Vec3* initialPositions = positions;

		// Iterate lens flare
		auto it = vi.getLensFlaresBegin();
		auto end = vi.getLensFlaresBegin() + totalCount;
		for(; it != end; ++it)
		{
			LensFlareComponent& lf =
				(it->m_node)->getComponent<LensFlareComponent>();

			*positions = lf.getWorldPosition().xyz();

			// Draw and query
			OcclusionQueryPtr& query = lf.getOcclusionQueryToTest();
			cmdb->beginOcclusionQuery(query);

			cmdb->drawArrays(1, 1, positions - initialPositions);

			cmdb->endOcclusionQuery(query);

			++positions;
		}

		positionsVertBuff->unmap();

		ANKI_ASSERT(positions == initialPositions + totalCount);
	}
}

//==============================================================================
void Lf::run(CommandBufferPtr& cmdb)
{
	// Retrieve some things
	SceneNode& cam = m_r->getActiveCamera();
	FrustumComponent& camFr = cam.getComponent<FrustumComponent>();
	VisibilityTestResults& vi = camFr.getVisibilityTestResults();

	U totalCount = min<U>(vi.getLensFlaresCount(), m_maxFlares);
	if(totalCount > 0)
	{
		// Set common rendering state
		cmdb->bindPipeline(m_realPpline);

		// Iterate lens flare
		auto it = vi.getLensFlaresBegin();
		auto end = vi.getLensFlaresBegin() + totalCount;
		for(; it != end; ++it)
		{
			LensFlareComponent& lf =
				(it->m_node)->getComponent<LensFlareComponent>();

			// Compute position
			Vec4 lfPos = Vec4(lf.getWorldPosition().xyz(), 1.0);
			Vec4 posClip = camFr.getViewProjectionMatrix() * lfPos;

			if(posClip.x() > posClip.w() || posClip.x() < -posClip.w()
				|| posClip.y() > posClip.w() || posClip.y() < -posClip.w())
			{
				// Outside clip
				continue;
			}

			U count = 0;
			U spritesCount = max<U>(1, m_maxSpritesPerFlare); // TODO

			cmdb->bindResourceGroup(lf.getResourceGroup());

			// Get uniform memory
			Sprite* tmpSprites = nullptr;
			cmdb->updateDynamicUniforms(
				sizeof(Sprite) * spritesCount, tmpSprites);
			SArray<Sprite> sprites(tmpSprites, spritesCount);

			// misc
			Vec2 posNdc = posClip.xy() / posClip.w();

			// First flare
			sprites[count].m_pos = posNdc;
			sprites[count].m_scale =
				lf.getFirstFlareSize() * Vec2(1.0, m_r->getAspectRatio());
			sprites[count].m_depth = 0.0;
			sprites[count].m_alpha = lf.getColorMultiplier().w();
			++count;

			// Render
			OcclusionQueryPtr query;
			Bool queryInvalid;
			lf.getOcclusionQueryToCheck(query, queryInvalid);

			if(!queryInvalid)
			{
				cmdb->drawArraysConditional(query, 4);
			}
			else
			{
				// Skip the drawcall. If the flare appeared suddenly inside the
				// view we don't want to draw it.
			}
		}
	}
}

} // end namespace anki
