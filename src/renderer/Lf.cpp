// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Lf.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Renderer.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/LensFlareComponent.h>
#include <anki/scene/Camera.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/Functions.h>

namespace anki
{

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
struct Sprite
{
	Vec2 m_pos; ///< Position in NDC
	Vec2 m_scale; ///< Scale of the quad
	Vec4 m_color;
	F32 m_depth; ///< Texture depth
	U32 m_padding[3];
};

//==============================================================================
// Lf                                                                          =
//==============================================================================

//==============================================================================
Lf::~Lf()
{
}

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
		m_realVert, "shaders/LfSpritePass.vert.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_realFrag, "shaders/LfSpritePass.frag.glsl", pps.toCString(), "r_"));

	// Create ppline.
	// Writes to IS with blending
	PipelineInitInfo init;
	init.m_inputAssembler.m_topology = PrimitiveTopology::TRIANGLE_STRIP;
	init.m_depthStencil.m_depthWriteEnabled = false;
	init.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;
	init.m_color.m_attachmentCount = 1;
	init.m_color.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;
	init.m_color.m_attachments[0].m_srcBlendMethod = BlendMethod::ONE;
	init.m_color.m_attachments[0].m_dstBlendMethod = BlendMethod::ONE;
	init.m_shaders[U(ShaderType::VERTEX)] = m_realVert->getGrShader();
	init.m_shaders[U(ShaderType::FRAGMENT)] = m_realFrag->getGrShader();
	m_realPpline = getGrManager().newInstance<Pipeline>(init);

	return ErrorCode::NONE;
}

//==============================================================================
Error Lf::initOcclusion(const ConfigSet& config)
{
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
	PipelineInitInfo init;
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
	{
		ResourceGroupInitInfo rcInit;
		rcInit.m_vertexBuffers[0].m_dynamic = true;
		rcInit.m_uniformBuffers[0].m_dynamic = true;
		m_occlusionRcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error Lf::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(initSprite(config));
	ANKI_CHECK(initOcclusion(config));

	getGrManager().finish();
	return ErrorCode::NONE;
}

//==============================================================================
void Lf::runOcclusionTests(CommandBufferPtr& cmdb)
{
	// Retrieve some things
	FrustumComponent& camFr = m_r->getActiveFrustumComponent();
	VisibilityTestResults& vi = camFr.getVisibilityTestResults();

	if(vi.getCount(VisibilityGroupType::FLARES) > m_maxFlares)
	{
		ANKI_LOGW("Visible flares exceed the limit. Increase lf.maxFlares");
	}

	U totalCount =
		min<U>(vi.getCount(VisibilityGroupType::FLARES), m_maxFlares);
	if(totalCount > 0)
	{
		// Setup MVP UBO
		DynamicBufferToken token;
		Mat4* mvp =
			static_cast<Mat4*>(getGrManager().allocateFrameHostVisibleMemory(
				sizeof(Mat4), BufferUsage::UNIFORM, token));
		*mvp = camFr.getViewProjectionMatrix();

		// Alloc dyn mem
		DynamicBufferToken token2;
		Vec3* positions =
			static_cast<Vec3*>(getGrManager().allocateFrameHostVisibleMemory(
				sizeof(Vec3) * totalCount, BufferUsage::VERTEX, token2));
		const Vec3* initialPositions = positions;

		// Setup state
		cmdb->bindPipeline(m_occlusionPpline);
		DynamicBufferInfo dyn;
		dyn.m_uniformBuffers[0] = token;
		dyn.m_vertexBuffers[0] = token2;
		cmdb->bindResourceGroup(m_occlusionRcGroup, 0, &dyn);

		// Iterate lens flare
		auto it = vi.getBegin(VisibilityGroupType::FLARES);
		auto end = vi.getBegin(VisibilityGroupType::FLARES) + totalCount;
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

		ANKI_ASSERT(positions == initialPositions + totalCount);
	}
}

//==============================================================================
void Lf::run(CommandBufferPtr& cmdb)
{
	// Retrieve some things
	FrustumComponent& camFr = m_r->getActiveFrustumComponent();
	VisibilityTestResults& vi = camFr.getVisibilityTestResults();

	U totalCount =
		min<U>(vi.getCount(VisibilityGroupType::FLARES), m_maxFlares);
	if(totalCount > 0)
	{
		// Set common rendering state
		cmdb->bindPipeline(m_realPpline);

		// Iterate lens flare
		auto it = vi.getBegin(VisibilityGroupType::FLARES);
		auto end = vi.getBegin(VisibilityGroupType::FLARES) + totalCount;
		for(; it != end; ++it)
		{
			LensFlareComponent& lf =
				(it->m_node)->getComponent<LensFlareComponent>();

			// Compute position
			Vec4 lfPos = Vec4(lf.getWorldPosition().xyz(), 1.0);
			Vec4 posClip = camFr.getViewProjectionMatrix() * lfPos;

			if(posClip.x() > posClip.w() || posClip.x() < -posClip.w()
				|| posClip.y() > posClip.w()
				|| posClip.y() < -posClip.w())
			{
				// Outside clip
				continue;
			}

			U count = 0;
			U spritesCount = max<U>(1, m_maxSpritesPerFlare); // TODO

			// Get uniform memory
			DynamicBufferToken token;
			Sprite* tmpSprites = static_cast<Sprite*>(
				getGrManager().allocateFrameHostVisibleMemory(
					spritesCount * sizeof(Sprite),
					BufferUsage::UNIFORM,
					token));
			SArray<Sprite> sprites(tmpSprites, spritesCount);

			// misc
			Vec2 posNdc = posClip.xy() / posClip.w();

			// First flare
			sprites[count].m_pos = posNdc;
			sprites[count].m_scale =
				lf.getFirstFlareSize() * Vec2(1.0, m_r->getAspectRatio());
			sprites[count].m_depth = 0.0;
			// Fade the flare on the edges
			F32 alpha = lf.getColorMultiplier().w()
				* (1.0 - pow(abs(posNdc.x()), 6.0))
				* (1.0 - pow(abs(posNdc.y()), 6.0));

			sprites[count].m_color = Vec4(lf.getColorMultiplier().xyz(), alpha);
			++count;

			// Render
			OcclusionQueryPtr query;
			Bool queryInvalid;
			lf.getOcclusionQueryToCheck(query, queryInvalid);

			if(!queryInvalid)
			{
				DynamicBufferInfo dyn;
				dyn.m_uniformBuffers[0] = token;
				cmdb->bindResourceGroup(lf.getResourceGroup(), 0, &dyn);
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
