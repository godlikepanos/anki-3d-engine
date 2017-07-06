// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/LightShading.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/Ssao.h>
#include <anki/renderer/Indirect.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/LightBin.h>
#include <anki/renderer/RenderQueue.h>
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
	Mat4 m_invProjMat;
};

enum class IsShaderVariantBit : U8
{
	P_LIGHTS = 1 << 0,
	S_LIGHTS = 1 << 1,
	DECALS = 1 << 2,
	INDIRECT = 1 << 3,
	P_LIGHTS_SHADOWS = 1 << 4,
	S_LIGHTS_SHADOWS = 1 << 5
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(IsShaderVariantBit, inline)

LightShading::LightShading(Renderer* r)
	: RenderingPass(r)
{
}

LightShading::~LightShading()
{
	if(m_lightBin)
	{
		getAllocator().deleteInstance(m_lightBin);
	}
}

Error LightShading::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing light stage");
	Error err = initInternal(config);

	if(err)
	{
		ANKI_R_LOGE("Failed to init light stage");
	}

	return err;
}

Error LightShading::initInternal(const ConfigSet& config)
{
	m_maxLightIds = config.getNumber("r.maxLightsPerCluster");

	if(m_maxLightIds == 0)
	{
		ANKI_R_LOGE("Incorrect number of max light indices");
		return ErrorCode::USER_DATA;
	}

	m_clusterCounts[0] = config.getNumber("r.clusterSizeX");
	m_clusterCounts[1] = config.getNumber("r.clusterSizeY");
	m_clusterCounts[2] = config.getNumber("r.clusterSizeZ");
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
	ANKI_CHECK(getResourceManager().loadResource("programs/LightShading.ankiprog", m_prog));

	ShaderProgramResourceConstantValueInitList<4> consts(m_prog);
	consts.add("CLUSTER_COUNT_X", U32(m_clusterCounts[0]))
		.add("CLUSTER_COUNT_Y", U32(m_clusterCounts[1]))
		.add("CLUSTER_COUNT", U32(m_clusterCount))
		.add("IR_MIPMAP_COUNT", U32(m_r->getIndirect().getReflectionTextureMipmapCount()));

	m_prog->getOrCreateVariant(consts.get(), m_progVariant);

	//
	// Create framebuffer
	//
	m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		SamplingFilter::LINEAR,
		1,
		"lightp"));

	FramebufferInitInfo fbInit("lightp");
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

Error LightShading::binLights(RenderingContext& ctx)
{
	updateCommonBlock(ctx);

	ANKI_CHECK(m_lightBin->bin(ctx.m_renderQueue->m_viewMatrix,
		ctx.m_renderQueue->m_projectionMatrix,
		ctx.m_renderQueue->m_viewProjectionMatrix,
		ctx.m_renderQueue->m_cameraTransform,
		*ctx.m_renderQueue,
		getFrameAllocator(),
		m_maxLightIds,
		true,
		ctx.m_lightShading.m_pointLightsToken,
		ctx.m_lightShading.m_spotLightsToken,
		&ctx.m_lightShading.m_probesToken,
		ctx.m_lightShading.m_decalsToken,
		ctx.m_lightShading.m_clustersToken,
		ctx.m_lightShading.m_lightIndicesToken,
		ctx.m_lightShading.m_diffDecalTex,
		ctx.m_lightShading.m_normRoughnessDecalTex));

	return ErrorCode::NONE;
}

void LightShading::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fb);
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindShaderProgram(m_progVariant->getProgram());

	cmdb->bindTexture(1, 0, m_r->getGBuffer().m_rt0);
	cmdb->bindTexture(1, 1, m_r->getGBuffer().m_rt1);
	cmdb->bindTexture(1, 2, m_r->getGBuffer().m_rt2);
	cmdb->bindTexture(1, 3, m_r->getGBuffer().m_depthRt, DepthStencilAspectBit::DEPTH);
	cmdb->informTextureCurrentUsage(m_r->getSsao().getRt(), TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->bindTexture(1, 4, m_r->getSsao().getRt());

	cmdb->bindTexture(0, 0, m_r->getShadowMapping().m_spotTex);
	cmdb->bindTexture(0, 1, m_r->getShadowMapping().m_omniTexArray);
	cmdb->bindTexture(0, 2, m_r->getIndirect().getReflectionTexture());
	cmdb->bindTexture(0, 3, m_r->getIndirect().getIrradianceTexture());
	cmdb->bindTextureAndSampler(
		0, 4, m_r->getIndirect().getIntegrationLut(), m_r->getIndirect().getIntegrationLutSampler());
	cmdb->bindTexture(
		0, 5, (ctx.m_lightShading.m_diffDecalTex) ? ctx.m_lightShading.m_diffDecalTex : m_r->getDummyTexture());
	cmdb->bindTexture(0,
		6,
		(ctx.m_lightShading.m_normRoughnessDecalTex) ? ctx.m_lightShading.m_normRoughnessDecalTex
													 : m_r->getDummyTexture());

	bindUniforms(cmdb, 0, 0, ctx.m_lightShading.m_commonToken);
	bindUniforms(cmdb, 0, 1, ctx.m_lightShading.m_pointLightsToken);
	bindUniforms(cmdb, 0, 2, ctx.m_lightShading.m_spotLightsToken);
	bindUniforms(cmdb, 0, 3, ctx.m_lightShading.m_probesToken);
	bindUniforms(cmdb, 0, 4, ctx.m_lightShading.m_decalsToken);

	bindStorage(cmdb, 0, 0, ctx.m_lightShading.m_clustersToken);
	bindStorage(cmdb, 0, 1, ctx.m_lightShading.m_lightIndicesToken);

	cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
	cmdb->endRenderPass();
}

void LightShading::updateCommonBlock(RenderingContext& ctx)
{
	ShaderCommonUniforms* blk =
		allocateUniforms<ShaderCommonUniforms*>(sizeof(ShaderCommonUniforms), ctx.m_lightShading.m_commonToken);

	// Start writing
	blk->m_projectionParams = ctx.m_unprojParams;
	blk->m_nearFarClustererMagicPad1 = Vec4(ctx.m_renderQueue->m_cameraNear,
		ctx.m_renderQueue->m_cameraFar,
		m_lightBin->getClusterer().getShaderMagicValue(),
		0.0);

	blk->m_invViewRotation = Mat3x4(ctx.m_renderQueue->m_viewMatrix.getInverse().getRotationPart());

	blk->m_rendererSizeTimePad1 = Vec4(m_r->getWidth(), m_r->getHeight(), HighRezTimer::getCurrentTime(), 0.0);

	blk->m_tileCount = UVec4(m_clusterCounts[0], m_clusterCounts[1], m_clusterCounts[2], m_clusterCount);

	blk->m_invViewProjMat = ctx.m_viewProjMatJitter.getInverse();
	blk->m_prevViewProjMat = ctx.m_prevViewProjMat;
	blk->m_invProjMat = ctx.m_projMatJitter.getInverse();
}

void LightShading::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

} // end namespace anki
