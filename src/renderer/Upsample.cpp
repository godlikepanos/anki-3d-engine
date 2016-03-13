// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Upsample.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Fs.h>
#include <anki/renderer/Ssao.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

//==============================================================================
Error Upsample::init(const ConfigSet& config)
{
	GrManager& gr = getGrManager();

	// Create RC group
	ResourceGroupInitInfo rcInit;
	SamplerInitInfo sinit;
	sinit.m_repeat = false;

	rcInit.m_textures[0].m_texture = m_r->getMs().getDepthRt();

	sinit.m_minLod = 1.0;
	sinit.m_mipmapFilter = SamplingFilter::NEAREST;
	rcInit.m_textures[1].m_texture = m_r->getMs().getDepthRt();
	rcInit.m_textures[1].m_sampler = gr.newInstance<Sampler>(sinit);

	sinit.m_minLod = 0.0;
	rcInit.m_textures[2].m_texture = m_r->getFs().getRt();
	rcInit.m_textures[2].m_sampler = gr.newInstance<Sampler>(sinit);

	sinit.m_minMagFilter = SamplingFilter::LINEAR;
	rcInit.m_textures[3].m_texture = m_r->getFs().getRt();
	rcInit.m_textures[3].m_sampler = gr.newInstance<Sampler>(sinit);

	if(m_r->getSsaoEnabled())
	{
		rcInit.m_textures[4].m_texture = m_r->getSsao().getRt();
	}

	rcInit.m_uniformBuffers[0].m_dynamic = true;

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	// Shader
	StringAuto pps(getFrameAllocator());
	pps.sprintf("#define TEXTURE_WIDTH %uu\n"
				"#define TEXTURE_HEIGHT %uu\n"
				"#define SSAO_ENABLED %u\n",
		m_r->getWidth() / FS_FRACTION,
		m_r->getHeight() / FS_FRACTION,
		m_r->getSsaoEnabled());

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_frag, "shaders/NearDepthUpscale.frag.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_vert, "shaders/NearDepthUpscale.vert.glsl", pps.toCString(), "r_"));

	// Ppline
	PipelineInitInfo ppinit;

	ppinit.m_depthStencil.m_depthWriteEnabled = false;
	ppinit.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;

	ppinit.m_color.m_attachmentCount = 1;
	ppinit.m_color.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;
	ppinit.m_color.m_attachments[0].m_srcBlendMethod = BlendMethod::ONE;
	ppinit.m_color.m_attachments[0].m_dstBlendMethod = BlendMethod::SRC_ALPHA;

	ppinit.m_shaders[U(ShaderType::VERTEX)] = m_vert->getGrShader();
	ppinit.m_shaders[U(ShaderType::FRAGMENT)] = m_frag->getGrShader();
	m_ppline = gr.newInstance<Pipeline>(ppinit);

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_r->getIs().getRt();
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::LOAD;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

//==============================================================================
void Upsample::run(RenderingContext& ctx)
{
	CommandBufferPtr cmdb = ctx.m_commandBuffer;
	DynamicBufferInfo dyn;

	Vec4* linearDepth =
		static_cast<Vec4*>(getGrManager().allocateFrameHostVisibleMemory(
			sizeof(Vec4), BufferUsage::UNIFORM, dyn.m_uniformBuffers[0]));
	const Frustum& fr = ctx.m_frustumComponent->getFrustum();
	computeLinearizeDepthOptimal(
		fr.getNear(), fr.getFar(), linearDepth->x(), linearDepth->y());

	cmdb->beginRenderPass(m_fb);
	cmdb->bindPipeline(m_ppline);
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindResourceGroup(m_rcGroup, 0, &dyn);

	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

} // end namespace anki
