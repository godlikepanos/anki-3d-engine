// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Volumetric.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/Sm.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/LightBin.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

Volumetric::~Volumetric()
{
}

Error Volumetric::init(const ConfigSet& config)
{
	U width = m_r->getWidth() / VOLUMETRIC_FRACTION;
	U height = m_r->getHeight() / VOLUMETRIC_FRACTION;

	// Create RTs
	m_r->createRenderTarget(width,
		height,
		IS_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE | TextureUsageBit::CLEAR,
		SamplingFilter::LINEAR,
		1,
		m_rt);

	m_r->clearRenderTarget(m_rt, ClearValue(), TextureUsageBit::SAMPLED_FRAGMENT);

	// Create shaders
	ANKI_CHECK(m_r->createShader("shaders/Volumetric.frag.glsl",
		m_frag,
		"#define RPASS_SIZE uvec2(%uu, %uu)\n"
		"#define CLUSTER_COUNT uvec3(%uu, %uu, %uu)\n",
		width,
		height,
		m_r->getIs().getLightBin().getClusterer().getClusterCountX(),
		m_r->getIs().getLightBin().getClusterer().getClusterCountY(),
		m_r->getIs().getLightBin().getClusterer().getClusterCountZ()));

	// Create pplines
	ColorStateInfo state;
	state.m_attachmentCount = 1;
	state.m_attachments[0].m_format = IS_COLOR_ATTACHMENT_PIXEL_FORMAT;
	state.m_attachments[0].m_srcBlendMethod = BlendMethod::SRC_ALPHA;
	state.m_attachments[0].m_dstBlendMethod = BlendMethod::ONE_MINUS_SRC_ALPHA;

	m_r->createDrawQuadPipeline(m_frag->getGrShader(), state, m_ppline);

	// Create the resource groups
	ResourceGroupInitInfo rcInit;
	rcInit.m_textures[0].m_texture = m_r->getDepthDownscale().m_qd.m_depthRt;
	rcInit.m_textures[0].m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;
	rcInit.m_textures[1].m_texture = m_r->getSm().getSpotTextureArray();
	rcInit.m_textures[2].m_texture = m_r->getSm().getOmniTextureArray();
	rcInit.m_uniformBuffers[0].m_uploadedMemory = true;
	rcInit.m_uniformBuffers[0].m_usage = BufferUsageBit::UNIFORM_FRAGMENT;
	rcInit.m_uniformBuffers[1].m_uploadedMemory = true;
	rcInit.m_uniformBuffers[1].m_usage = BufferUsageBit::UNIFORM_FRAGMENT;
	rcInit.m_uniformBuffers[2].m_uploadedMemory = true;
	rcInit.m_uniformBuffers[2].m_usage = BufferUsageBit::UNIFORM_FRAGMENT;
	rcInit.m_uniformBuffers[3].m_uploadedMemory = true;
	rcInit.m_uniformBuffers[3].m_usage = BufferUsageBit::UNIFORM_FRAGMENT;
	rcInit.m_storageBuffers[0].m_uploadedMemory = true;
	rcInit.m_storageBuffers[0].m_usage = BufferUsageBit::STORAGE_FRAGMENT_READ;
	rcInit.m_storageBuffers[1].m_uploadedMemory = true;
	rcInit.m_storageBuffers[1].m_usage = BufferUsageBit::STORAGE_FRAGMENT_READ;
	m_rc = getGrManager().newInstance<ResourceGroup>(rcInit);

	// Create FBs
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_colorAttachments[0].m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

void Volumetric::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void Volumetric::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void Volumetric::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	const Frustum& frc = ctx.m_frustumComponent->getFrustum();

	// Update uniforms
	TransientMemoryInfo dyn = ctx.m_is.m_dynBufferInfo;
	Vec4* uniforms = static_cast<Vec4*>(getGrManager().allocateFrameTransientMemory(
		sizeof(Vec4) * 2, BufferUsageBit::UNIFORM_ALL, dyn.m_uniformBuffers[3]));
	computeLinearizeDepthOptimal(frc.getNear(), frc.getFar(), uniforms[0].x(), uniforms[0].y());

	uniforms[1] = Vec4(m_fogColor, m_fogFactor);

	// pass 0
	cmdb->setViewport(0, 0, m_r->getWidth() / VOLUMETRIC_FRACTION, m_r->getHeight() / VOLUMETRIC_FRACTION);
	cmdb->beginRenderPass(m_fb);
	cmdb->bindPipeline(m_ppline);
	cmdb->bindResourceGroup(m_rc, 0, &dyn);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

} // end namespace anki
