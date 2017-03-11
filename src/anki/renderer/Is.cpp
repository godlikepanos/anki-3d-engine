// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Is.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Sm.h>
#include <anki/renderer/Ssao.h>
#include <anki/renderer/Ir.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/LightBin.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/HighRezTimer.h>

namespace anki
{

class ShaderCommonUniforms
{
public:
	Vec4 m_projectionParams;
	Vec4 m_rendererSizeTimePad1;
	Vec4 m_nearFarClustererMagicPad1;
	Mat3x4 m_invViewRotation;
	UVec4 m_tileCount;
	Mat4 m_invViewProjMat;
	Mat4 m_prevViewProjMat;
};

enum class ShaderVariantBit : U8
{
	P_LIGHTS = 1 << 0,
	S_LIGHTS = 1 << 1,
	DECALS = 1 << 2,
	INDIRECT = 1 << 3,
	P_LIGHTS_SHADOWS = 1 << 4,
	S_LIGHTS_SHADOWS = 1 << 5
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderVariantBit, inline)

Is::Is(Renderer* r)
	: RenderingPass(r)
{
}

Is::~Is()
{
	if(m_lightBin)
	{
		getAllocator().deleteInstance(m_lightBin);
	}
}

Error Is::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing light stage");
	Error err = initInternal(config);

	if(err)
	{
		ANKI_R_LOGE("Failed to init light stage");
	}

	return err;
}

Error Is::initInternal(const ConfigSet& config)
{
	m_maxLightIds = config.getNumber("is.maxLightsPerCluster");

	if(m_maxLightIds == 0)
	{
		ANKI_R_LOGE("Incorrect number of max light indices");
		return ErrorCode::USER_DATA;
	}

	m_clusterCounts[0] = config.getNumber("clusterSizeX");
	m_clusterCounts[1] = config.getNumber("clusterSizeY");
	m_clusterCounts[2] = config.getNumber("clusterSizeZ");
	m_clusterCount = m_clusterCounts[0] * m_clusterCounts[1] * m_clusterCounts[2];

	m_maxLightIds *= m_clusterCount;

	m_lightBin = getAllocator().newInstance<LightBin>(getAllocator(),
		m_clusterCounts[0],
		m_clusterCounts[1],
		m_clusterCounts[2],
		&m_r->getThreadPool(),
		&m_r->getStagingGpuMemoryManager());

	//
	// Load shaders and programs
	//
	StringAuto pps(getAllocator());

	pps.sprintf("#define CLUSTER_COUNT_X %u\n"
				"#define CLUSTER_COUNT_Y %u\n"
				"#define CLUSTER_COUNT %u\n"
				"#define RENDERER_WIDTH %u\n"
				"#define RENDERER_HEIGHT %u\n"
				"#define MAX_LIGHT_INDICES %u\n"
				"#define POISSON %u\n"
				"#define INDIRECT_ENABLED %u\n"
				"#define IR_MIPMAP_COUNT %u\n",
		m_clusterCounts[0],
		m_clusterCounts[1],
		m_clusterCount,
		m_r->getWidth(),
		m_r->getHeight(),
		m_maxLightIds,
		m_r->getSm().m_poissonEnabled,
		1,
		m_r->getIr().getReflectionTextureMipmapCount());

	ANKI_CHECK(m_r->createShader("shaders/Is.vert.glsl", m_lightVert, &pps[0]));
	ANKI_CHECK(m_r->createShader("shaders/Is.frag.glsl", m_lightFrag, &pps[0]));

	m_lightProg = getGrManager().newInstance<ShaderProgram>(m_lightVert->getGrShader(), m_lightFrag->getGrShader());

	//
	// Create framebuffer
	//
	m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		IS_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		SamplingFilter::LINEAR));

	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

Error Is::binLights(RenderingContext& ctx)
{
	updateCommonBlock(ctx);

	ANKI_CHECK(m_lightBin->bin(ctx.m_viewMat,
		ctx.m_projMat,
		ctx.m_viewProjMat,
		ctx.m_camTrfMat,
		*ctx.m_visResults,
		getFrameAllocator(),
		m_maxLightIds,
		true,
		ctx.m_is.m_pointLightsToken,
		ctx.m_is.m_spotLightsToken,
		&ctx.m_is.m_probesToken,
		ctx.m_is.m_decalsToken,
		ctx.m_is.m_clustersToken,
		ctx.m_is.m_lightIndicesToken,
		ctx.m_is.m_diffDecalTex,
		ctx.m_is.m_normRoughnessDecalTex));

	return ErrorCode::NONE;
}

void Is::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fb);
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindShaderProgram(m_lightProg);

	cmdb->bindTexture(0, 0, m_r->getMs().m_rt0);
	cmdb->bindTexture(0, 1, m_r->getMs().m_rt1);
	cmdb->bindTexture(0, 2, m_r->getMs().m_rt2);
	cmdb->bindTexture(0, 3, m_r->getMs().m_depthRt, DepthStencilAspectBit::DEPTH);
	cmdb->bindTexture(0, 4, m_r->getSm().m_spotTexArray);
	cmdb->bindTexture(0, 5, m_r->getSm().m_omniTexArray);
	cmdb->bindTexture(0, 6, m_r->getIr().getReflectionTexture());
	cmdb->bindTexture(0, 7, m_r->getIr().getIrradianceTexture());
	cmdb->bindTextureAndSampler(0, 8, m_r->getIr().getIntegrationLut(), m_r->getIr().getIntegrationLutSampler());

	cmdb->bindTexture(1, 0, (ctx.m_is.m_diffDecalTex) ? ctx.m_is.m_diffDecalTex : m_r->getDummyTexture());
	cmdb->bindTexture(
		1, 1, (ctx.m_is.m_normRoughnessDecalTex) ? ctx.m_is.m_normRoughnessDecalTex : m_r->getDummyTexture());
	cmdb->informTextureCurrentUsage(m_r->getSsao().m_main.m_rt, TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->bindTexture(1, 2, m_r->getSsao().m_main.m_rt);

	bindUniforms(cmdb, 0, 0, ctx.m_is.m_commonToken);
	bindUniforms(cmdb, 0, 1, ctx.m_is.m_pointLightsToken);
	bindUniforms(cmdb, 0, 2, ctx.m_is.m_spotLightsToken);
	bindUniforms(cmdb, 0, 3, ctx.m_is.m_probesToken);
	bindUniforms(cmdb, 0, 4, ctx.m_is.m_decalsToken);

	bindStorage(cmdb, 0, 0, ctx.m_is.m_clustersToken);
	bindStorage(cmdb, 0, 1, ctx.m_is.m_lightIndicesToken);

	cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
	cmdb->endRenderPass();
}

void Is::updateCommonBlock(RenderingContext& ctx)
{
	ShaderCommonUniforms* blk =
		allocateUniforms<ShaderCommonUniforms*>(sizeof(ShaderCommonUniforms), ctx.m_is.m_commonToken);

	// Start writing
	blk->m_projectionParams = ctx.m_unprojParams;
	blk->m_nearFarClustererMagicPad1 =
		Vec4(ctx.m_near, ctx.m_far, m_lightBin->getClusterer().getShaderMagicValue(), 0.0);

	blk->m_invViewRotation = Mat3x4(ctx.m_viewMat.getInverse().getRotationPart());

	blk->m_rendererSizeTimePad1 = Vec4(m_r->getWidth(), m_r->getHeight(), HighRezTimer::getCurrentTime(), 0.0);

	blk->m_tileCount = UVec4(m_clusterCounts[0], m_clusterCounts[1], m_clusterCounts[2], m_clusterCount);

	blk->m_invViewProjMat = ctx.m_viewProjMat.getInverse();
	blk->m_prevViewProjMat = ctx.m_prevViewProjMat;
}

void Is::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

Error Is::getOrCreateProgram(ShaderVariantBit variantMask, RenderingContext& ctx, ShaderProgramPtr& prog)
{
	auto it = m_shaderVariantMap.find(variantMask);
	if(it != m_shaderVariantMap.getEnd())
	{
		prog = it->m_lightProg;
	}
	else
	{
		ShaderVariant variant;

		ANKI_CHECK(m_r->createShaderf("shaders/Is.frag.glsl",
			variant.m_lightFrag,
			"#define TILE_COUNT_X %u\n"
			"#define TILE_COUNT_Y %u\n"
			"#define CLUSTER_COUNT %u\n"
			"#define RENDERER_WIDTH %u\n"
			"#define RENDERER_HEIGHT %u\n"
			"#define MAX_LIGHT_INDICES %u\n"
			"#define POISSON %u\n"
			"#define INDIRECT_ENABLED %u\n"
			"#define IR_MIPMAP_COUNT %u\n"
			"#define POINT_LIGHTS_ENABLED %u\n"
			"#define SPOT_LIGHTS_ENABLED %u\n"
			"#define DECALS_ENABLED %u\n"
			"#define POINT_LIGHTS_SHADOWS_ENABLED %u\n"
			"#define SPOT_LIGHTS_SHADOWS_ENABLED %u\n",
			m_clusterCounts[0],
			m_clusterCounts[1],
			m_clusterCount,
			m_r->getWidth(),
			m_r->getHeight(),
			m_maxLightIds,
			m_r->getSm().m_poissonEnabled,
			!!(variantMask & ShaderVariantBit::INDIRECT),
			m_r->getIr().getReflectionTextureMipmapCount(),
			!!(variantMask & ShaderVariantBit::P_LIGHTS),
			!!(variantMask & ShaderVariantBit::S_LIGHTS),
			!!(variantMask & ShaderVariantBit::DECALS),
			!!(variantMask & ShaderVariantBit::P_LIGHTS_SHADOWS),
			!!(variantMask & ShaderVariantBit::S_LIGHTS_SHADOWS)));

		variant.m_lightProg =
			getGrManager().newInstance<ShaderProgram>(m_lightVert->getGrShader(), variant.m_lightFrag->getGrShader());

		prog = variant.m_lightProg;

		m_shaderVariantMap.pushBack(getAllocator(), variantMask, variant);
	}

	return ErrorCode::NONE;
}

} // end namespace anki
