// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/LightShading.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/Indirect.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/LightBin.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/ForwardShading.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/Reflections.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/HighRezTimer.h>
#include <anki/collision/Functions.h>

namespace anki
{

/// @note Should match the shader
struct ShaderCommonUniforms
{
	Vec4 m_unprojectionParams;
	Vec4 m_rendererSizeTimeNear;
	Vec4 m_cameraPosFar;
	ClustererShaderMagicValues m_clustererMagicValues;
	UVec4 m_tileCount;
	Mat4 m_viewMat;
	Mat4 m_invViewMat;
	Mat4 m_projMat;
	Mat4 m_invProjMat;
	Mat4 m_viewProjMat;
	Mat4 m_invViewProjMat;
	Mat4 m_prevViewProjMat;
	Mat4 m_prevViewProjMatMulInvViewProjMat;
};

LightShading::LightShading(Renderer* r)
	: RendererObject(r)
{
}

LightShading::~LightShading()
{
	getAllocator().deleteInstance(m_lightBin);
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
		return Error::USER_DATA;
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

	// Load shaders and programs
	ANKI_CHECK(getResourceManager().loadResource("programs/LightShading.ankiprog", m_prog));

	ShaderProgramResourceConstantValueInitList<5> consts(m_prog);
	consts.add("CLUSTER_COUNT_X", U32(m_clusterCounts[0]))
		.add("CLUSTER_COUNT_Y", U32(m_clusterCounts[1]))
		.add("CLUSTER_COUNT_Z", U32(m_clusterCounts[2]))
		.add("CLUSTER_COUNT", U32(m_clusterCount))
		.add("IR_MIPMAP_COUNT", U32(m_r->getIndirect().getReflectionTextureMipmapCount()));

	m_prog->getOrCreateVariant(consts.get(), m_progVariant);

	// Create RT descr
	m_rtDescr = m_r->create2DRenderTargetDescription(m_r->getWidth(),
		m_r->getHeight(),
		LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		"Light Shading");
	m_rtDescr.bake();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fbDescr.bake();

	return Error::NONE;
}

Error LightShading::binLights(RenderingContext& ctx)
{
	updateCommonBlock(ctx);

	ANKI_CHECK(m_lightBin->bin(ctx.m_renderQueue->m_viewMatrix,
		ctx.m_renderQueue->m_projectionMatrix,
		ctx.m_renderQueue->m_viewProjectionMatrix,
		ctx.m_renderQueue->m_cameraTransform,
		*ctx.m_renderQueue,
		ctx.m_tempAllocator,
		m_maxLightIds,
		true,
		m_runCtx.m_resources));

	return Error::NONE;
}

void LightShading::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const LightShadingResources& rsrc = m_runCtx.m_resources;

	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindShaderProgram(m_progVariant->getProgram());

	// Bind textures
	rgraphCtx.bindColorTextureAndSampler(0, 0, m_r->getGBuffer().getColorRt(0), m_r->getNearestSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getGBuffer().getColorRt(1), m_r->getNearestSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 2, m_r->getGBuffer().getColorRt(2), m_r->getNearestSampler());
	rgraphCtx.bindTextureAndSampler(0,
		3,
		m_r->getGBuffer().getDepthRt(),
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
		m_r->getNearestSampler());

	rgraphCtx.bindColorTextureAndSampler(0, 4, m_r->getReflections().getRt(), m_r->getNearestSampler());

	rgraphCtx.bindColorTextureAndSampler(0, 5, m_r->getShadowMapping().getShadowmapRt(), m_r->getLinearSampler());

	// Bind uniforms
	bindUniforms(cmdb, 0, 0, rsrc.m_commonUniformsToken);
	bindUniforms(cmdb, 0, 1, rsrc.m_pointLightsToken);
	bindUniforms(cmdb, 0, 2, rsrc.m_spotLightsToken);

	// Bind storage
	bindStorage(cmdb, 0, 0, rsrc.m_clustersToken);
	bindStorage(cmdb, 0, 1, rsrc.m_lightIndicesToken);

	cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);

	// Apply the forward shading result
	m_r->getForwardShading().drawUpscale(ctx, rgraphCtx);
}

void LightShading::updateCommonBlock(RenderingContext& ctx)
{
	ShaderCommonUniforms* blk = allocateUniforms<ShaderCommonUniforms*>(
		sizeof(ShaderCommonUniforms), m_runCtx.m_resources.m_commonUniformsToken);

	// Start writing
	blk->m_unprojectionParams = ctx.m_unprojParams;

	blk->m_rendererSizeTimeNear =
		Vec4(m_r->getWidth(), m_r->getHeight(), HighRezTimer::getCurrentTime(), ctx.m_renderQueue->m_cameraNear);

	blk->m_tileCount = UVec4(m_clusterCounts[0], m_clusterCounts[1], m_clusterCounts[2], m_clusterCount);

	blk->m_cameraPosFar =
		Vec4(ctx.m_renderQueue->m_cameraTransform.getTranslationPart().xyz(), ctx.m_renderQueue->m_cameraFar);

	blk->m_clustererMagicValues = m_lightBin->getClusterer().getShaderMagicValues();

	// Matrices
	blk->m_viewMat = ctx.m_renderQueue->m_viewMatrix;
	blk->m_invViewMat = ctx.m_renderQueue->m_viewMatrix.getInverse();

	blk->m_projMat = ctx.m_projMatJitter;
	blk->m_invProjMat = ctx.m_projMatJitter.getInverse();

	blk->m_viewProjMat = ctx.m_viewProjMatJitter;
	blk->m_invViewProjMat = ctx.m_viewProjMatJitter.getInverse();

	blk->m_prevViewProjMat = ctx.m_prevViewProjMat;

	blk->m_prevViewProjMatMulInvViewProjMat = ctx.m_prevViewProjMat * ctx.m_viewProjMatJitter.getInverse();
}

void LightShading::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Light Shading");

	pass.setWork(runCallback, this, 0);
	pass.setFramebufferInfo(m_fbDescr, {{m_runCtx.m_rt}}, {});

	pass.newConsumer({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	pass.newConsumer({m_r->getGBuffer().getColorRt(0), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newConsumer({m_r->getGBuffer().getColorRt(1), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newConsumer({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newConsumer({m_r->getGBuffer().getDepthRt(),
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});
	pass.newConsumer({m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newConsumer({m_r->getReflections().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});

	// For forward shading
	pass.newConsumer({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_FRAGMENT, HIZ_HALF_DEPTH});
	pass.newConsumer({m_r->getForwardShading().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});

	pass.newProducer({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
}

} // end namespace anki
