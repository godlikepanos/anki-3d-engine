// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Fs.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Ms.h"
#include "anki/renderer/Is.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"

namespace anki {

//==============================================================================
Fs::~Fs()
{}

//==============================================================================
Error Fs::init(const ConfigSet&)
{
	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_r->getIs().getRt();
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::LOAD;
	fbInit.m_depthStencilAttachment.m_texture = m_r->getMs().getDepthRt();
	fbInit.m_depthStencilAttachment.m_loadOperation =
		AttachmentLoadOperation::LOAD;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Init the global resources
	for(U i = 0; i < m_globalResources.getSize(); ++i)
	{
		ResourceGroupInitializer init;
		init.m_textures[0].m_texture = m_r->getMs().getDepthRt();
		init.m_textures[1].m_texture =
			m_r->getIs().getSm().getSpotTextureArray();
		init.m_textures[2].m_texture =
			m_r->getIs().getSm().getOmniTextureArray();

		init.m_storageBuffers[0].m_buffer =
			m_r->getIs().getCommonVarsBuffer(i);
		init.m_storageBuffers[1].m_buffer =
			m_r->getIs().getPointLightsBuffer(i);
		init.m_storageBuffers[2].m_buffer =
			m_r->getIs().getSpotLightsBuffer(i);
		init.m_storageBuffers[3].m_buffer = m_r->getIs().getClusterBuffer(i);
		init.m_storageBuffers[4].m_buffer =
			m_r->getIs().getLightIndicesBuffer(i);

		m_globalResources[i] = getGrManager().newInstance<ResourceGroup>(init);
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error Fs::run(CommandBufferPtr& cmdb)
{
	cmdb->bindFramebuffer(m_fb);

	FrustumComponent& camFr = m_r->getActiveFrustumComponent();

	cmdb->bindResourceGroup(m_globalResources[getGlobalTimestamp() % 3], 1);

	SArray<CommandBufferPtr> cmdbs(&cmdb, 1);
	ANKI_CHECK(m_r->getSceneDrawer().render(
		camFr, RenderingStage::BLEND, Pass::MS_FS, cmdbs));

	return ErrorCode::NONE;
}

} // end namespace anki
