// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Renderer.h>

namespace anki
{

//==============================================================================
Error DownscaleBlur::initSubpass(U idx, const UVec2& inputTexSize)
{
	Subpass& pass = m_passes[idx];

	PipelineInitInfo ppinit;
	ppinit.m_color.m_attachmentCount = 1;
	ppinit.m_color.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;
	ppinit.m_depthStencil.m_depthWriteEnabled = false;
	ppinit.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;

	StringAuto pps(getAllocator());

	// vert shader
	pps.sprintf("#define UV_OFFSET vec2(%f, %f)\n",
		1.0 / inputTexSize.x(),
		1.0 / inputTexSize.y());

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		pass.m_vert, "shaders/Quad.vert.glsl", pps.toCString(), "r_"));

	ppinit.m_shaders[ShaderType::VERTEX] = pass.m_vert->getGrShader();

	// frag shader
	pps.destroy();
	pps.sprintf("#define TEXTURE_SIZE vec2(%f, %f)\n"
				"#define TEXTURE_MIPMAP float(%u)\n",
		F32(inputTexSize.x()),
		F32(inputTexSize.y()),
		idx);

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		pass.m_frag, "shaders/DownscaleBlur.frag.glsl", pps.toCString(), "r_"));

	ppinit.m_shaders[ShaderType::FRAGMENT] = pass.m_frag->getGrShader();

	// ppline
	pass.m_ppline = getGrManager().newInstance<Pipeline>(ppinit);

	// FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_r->getIs().getRt();
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	fbInit.m_colorAttachments[0].m_mipmap = idx + 1;
	pass.m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Resources
	ResourceGroupInitInfo rcinit;
	rcinit.m_textures[0].m_texture = m_r->getIs().getRt();
	pass.m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcinit);

	return ErrorCode::NONE;
}

//==============================================================================
Error DownscaleBlur::init(const ConfigSet& initializer)
{
	UVec2 size(m_r->getWidth(), m_r->getHeight());
	for(U i = 0; i < m_passes.getSize(); ++i)
	{
		ANKI_CHECK(initSubpass(i, size));
		size /= 2;
	}

	return ErrorCode::NONE;
}

//==============================================================================
void DownscaleBlur::run(RenderingContext& ctx)
{
	CommandBufferPtr cmdb = ctx.m_commandBuffer;
	UVec2 size(m_r->getWidth(), m_r->getHeight());
	for(U i = 0; i < m_passes.getSize(); ++i)
	{
		size /= 2;
		Subpass& pass = m_passes[i];

		cmdb->bindFramebuffer(pass.m_fb);
		cmdb->setViewport(0, 0, size.x(), size.y());
		cmdb->bindPipeline(pass.m_ppline);
		cmdb->bindResourceGroup(pass.m_rcGroup, 0, nullptr);

		m_r->drawQuad(cmdb);
	}
}

} // end namespace anki
