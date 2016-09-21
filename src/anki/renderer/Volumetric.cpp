// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Volumetric.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/HalfDepth.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

Volumetric::~Volumetric()
{
}

Error Volumetric::init(const ConfigSet& config)
{
	// Create frag shader
	ANKI_CHECK(getResourceManager().loadResource("shaders/Volumetric.frag.glsl", m_frag));

	// Create ppline
	PipelineInitInfo init;
	init.m_inputAssembler.m_topology = PrimitiveTopology::TRIANGLE_STRIP;
	init.m_depthStencil.m_depthWriteEnabled = false;
	init.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;
	init.m_depthStencil.m_format = MS_DEPTH_ATTACHMENT_PIXEL_FORMAT;
	init.m_color.m_attachmentCount = 1;
	init.m_color.m_attachments[0].m_format = IS_COLOR_ATTACHMENT_PIXEL_FORMAT;
	init.m_color.m_attachments[0].m_srcBlendMethod = BlendMethod::ONE;
	init.m_color.m_attachments[0].m_dstBlendMethod = BlendMethod::ONE;
	init.m_shaders[ShaderType::VERTEX] = m_r->getDrawQuadVertexShader();
	init.m_shaders[ShaderType::FRAGMENT] = m_frag->getGrShader();

	m_ppline = getGrManager().newInstance<Pipeline>(init);

	// Create the resource group
	ResourceGroupInitInfo rcInit;
	rcInit.m_textures[0].m_texture = m_r->getHalfDepth().m_depthRt;
	rcInit.m_textures[0].m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;
	rcInit.m_uniformBuffers[0].m_uploadedMemory = true;
	rcInit.m_uniformBuffers[0].m_usage = BufferUsageBit::UNIFORM_FRAGMENT;

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	return ErrorCode::NONE;
}

void Volumetric::run(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	const Frustum& frc = ctx.m_frustumComponent->getFrustum();

	// Update uniforms
	TransientMemoryInfo dyn;
	Vec4* uniforms = static_cast<Vec4*>(getGrManager().allocateFrameTransientMemory(
		sizeof(Vec4) * 2, BufferUsageBit::UNIFORM_ALL, dyn.m_uniformBuffers[0]));

	computeLinearizeDepthOptimal(frc.getNear(), frc.getFar(), uniforms[0].x(), uniforms[0].y());

	uniforms[1] = Vec4(m_fogColor, m_fogFactor);

	// Draw
	cmdb->bindPipeline(m_ppline);
	cmdb->bindResourceGroup(m_rcGroup, 0, &dyn);
	m_r->drawQuad(cmdb);
}

} // end namespace anki
