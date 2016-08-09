// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Volumetric.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

//==============================================================================
Volumetric::~Volumetric()
{
}

//==============================================================================
Error Volumetric::init(const ConfigSet& config)
{
	// Create frag shader
	ANKI_CHECK(getResourceManager().loadResource(
		"shaders/Volumetric.frag.glsl", m_frag));

	// Create ppline
	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_srcBlendMethod = BlendMethod::ONE;
	colorState.m_attachments[0].m_dstBlendMethod = BlendMethod::ONE;

	m_r->createDrawQuadPipeline(m_frag->getGrShader(), colorState, m_ppline);

	// Create the resource group
	ResourceGroupInitInfo rcInit;
	rcInit.m_textures[0].m_texture = m_r->getMs().getDepthRt();
	rcInit.m_textures[0].m_usage = TextureUsageBit::SAMPLED_FRAGMENT
		| TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;
	rcInit.m_uniformBuffers[0].m_uploadedMemory = true;
	rcInit.m_uniformBuffers[0].m_usage = BufferUsageBit::UNIFORM_FRAGMENT;

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	return ErrorCode::NONE;
}

//==============================================================================
void Volumetric::run(RenderingContext& ctx)
{
	const Frustum& frc = ctx.m_frustumComponent->getFrustum();
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Update uniforms
	TransientMemoryInfo dyn;
	Vec4* uniforms = static_cast<Vec4*>(
		getGrManager().allocateFrameTransientMemory(sizeof(Vec4) * 2,
			BufferUsageBit::UNIFORM_ALL,
			dyn.m_uniformBuffers[0]));

	computeLinearizeDepthOptimal(
		frc.getNear(), frc.getFar(), uniforms[0].x(), uniforms[0].y());

	uniforms[1] = Vec4(m_fogColor, m_fogFactor);

	// Draw
	cmdb->bindPipeline(m_ppline);
	cmdb->bindResourceGroup(m_rcGroup, 0, &dyn);
	m_r->drawQuad(cmdb);
}

} // end namespace anki
