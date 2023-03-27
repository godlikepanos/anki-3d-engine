// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ShadowmapsResolve.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/PackVisibleClusteredObjects.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

ShadowmapsResolve::~ShadowmapsResolve()
{
}

Error ShadowmapsResolve::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize shadow resolve pass");
	}

	return Error::kNone;
}

Error ShadowmapsResolve::initInternal()
{
	m_quarterRez = ConfigSet::getSingleton().getRSmResolveQuarterRez();
	const U32 width = m_r->getInternalResolution().x() / (m_quarterRez + 1);
	const U32 height = m_r->getInternalResolution().y() / (m_quarterRez + 1);

	ANKI_R_LOGV("Initializing shadowmaps resolve. Resolution %ux%u", width, height);

	m_rtDescr = m_r->create2DRenderTargetDescription(width, height, Format::kR8G8B8A8_Unorm, "SM resolve");
	m_rtDescr.bake();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	// Prog
	ANKI_CHECK(ResourceManager::getSingleton().loadResource((ConfigSet::getSingleton().getRPreferCompute())
																? "ShaderBinaries/ShadowmapsResolveCompute.ankiprogbin"
																: "ShaderBinaries/ShadowmapsResolveRaster.ankiprogbin",
															m_prog));
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("kFramebufferSize", UVec2(width, height));
	variantInitInfo.addConstant("kTileCount", m_r->getTileCounts());
	variantInitInfo.addConstant("kZSplitCount", m_r->getZSplitCount());
	variantInitInfo.addConstant("kTileSize", m_r->getTileSize());
	variantInitInfo.addMutation("PCF", ConfigSet::getSingleton().getRShadowMappingPcf());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	return Error::kNone;
}

void ShadowmapsResolve::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	if(ConfigSet::getSingleton().getRPreferCompute())
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("ResolveShadows");

		rpass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			run(rgraphCtx);
		});

		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kImageComputeWrite);
		rpass.newTextureDependency((m_quarterRez) ? m_r->getDepthDownscale().getHiZRt()
												  : m_r->getGBuffer().getDepthRt(),
								   TextureUsageBit::kSampledCompute, TextureSurfaceInfo(0, 0, 0, 0));
		rpass.newTextureDependency(m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::kSampledCompute);

		rpass.newBufferDependency(m_r->getClusterBinning().getClustersRenderGraphHandle(),
								  BufferUsageBit::kStorageComputeRead);
	}
	else
	{
		GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("ResolveShadows");
		rpass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_rt});

		rpass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			run(rgraphCtx);
		});

		rpass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite);
		rpass.newTextureDependency((m_quarterRez) ? m_r->getDepthDownscale().getHiZRt()
												  : m_r->getGBuffer().getDepthRt(),
								   TextureUsageBit::kSampledFragment, TextureSurfaceInfo(0, 0, 0, 0));
		rpass.newTextureDependency(m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::kSampledFragment);

		rpass.newBufferDependency(m_r->getClusterBinning().getClustersRenderGraphHandle(),
								  BufferUsageBit::kStorageFragmentRead);
	}
}

void ShadowmapsResolve::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grProg);

	bindUniforms(cmdb, 0, 0, m_r->getClusterBinning().getClusteredUniformsRebarToken());
	m_r->getPackVisibleClusteredObjects().bindClusteredObjectBuffer(cmdb, 0, 1, ClusteredObjectType::kPointLight);
	m_r->getPackVisibleClusteredObjects().bindClusteredObjectBuffer(cmdb, 0, 2, ClusteredObjectType::kSpotLight);
	rgraphCtx.bindColorTexture(0, 3, m_r->getShadowMapping().getShadowmapRt());
	bindStorage(cmdb, 0, 4, m_r->getClusterBinning().getClustersRebarToken());

	cmdb->bindSampler(0, 5, m_r->getSamplers().m_trilinearClamp);
	cmdb->bindSampler(0, 6, m_r->getSamplers().m_trilinearClampShadow);
	cmdb->bindSampler(0, 7, m_r->getSamplers().m_trilinearRepeat);

	if(m_quarterRez)
	{
		rgraphCtx.bindTexture(0, 8, m_r->getDepthDownscale().getHiZRt(),
							  TextureSubresourceInfo(TextureSurfaceInfo(0, 0, 0, 0)));
	}
	else
	{
		rgraphCtx.bindTexture(0, 8, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
	}
	cmdb->bindTexture(0, 9, m_noiseImage->getTextureView());

	if(ConfigSet::getSingleton().getRPreferCompute())
	{
		rgraphCtx.bindImage(0, 10, m_runCtx.m_rt, TextureSubresourceInfo());
		dispatchPPCompute(cmdb, 8, 8, m_rtDescr.m_width, m_rtDescr.m_height);
	}
	else
	{
		cmdb->setViewport(0, 0, m_rtDescr.m_width, m_rtDescr.m_height);
		cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
	}
}

} // end namespace anki
