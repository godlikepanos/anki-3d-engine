// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/GlState.h>
#include <anki/core/Config.h>
#include <anki/core/NativeWindow.h>

namespace anki
{

GrManagerImpl::GrManagerImpl()
{
}

GrManagerImpl::~GrManagerImpl()
{
	m_fakeDefaultFb.reset(nullptr);
	m_fakeFbTex.reset(nullptr);

	if(m_thread)
	{
		m_thread->stop();
		m_alloc.deleteInstance(m_thread);
		m_thread = nullptr;
	}

	if(m_state)
	{
		m_alloc.deleteInstance(m_state);
	}

	destroyBackend();
}

Error GrManagerImpl::init(GrManagerInitInfo& init, GrAllocator<U8> alloc)
{
	m_alloc = alloc;
	m_cacheDir.create(m_alloc, init.m_cacheDirectory);

	m_debugMarkers = init.m_config->getNumber("window.debugMarkers");

	// Init the backend of the backend
	ANKI_CHECK(createBackend(init));

	// First create the state
	m_state = m_alloc.newInstance<GlState>(this);
	m_state->initMainThread(*init.m_config);

	// Create thread
	m_thread = m_alloc.newInstance<RenderingThread>(this);

	// Start it
	m_thread->start();
	m_thread->syncClientServer();

	// Misc
	m_capabilities.m_gpuVendor = m_state->m_gpu;
	m_capabilities.m_uniformBufferBindOffsetAlignment = m_state->m_uboAlignment;
	m_capabilities.m_uniformBufferMaxRange = m_state->m_uniBlockMaxSize;
	m_capabilities.m_storageBufferBindOffsetAlignment = m_state->m_ssboAlignment;
	m_capabilities.m_storageBufferMaxRange = m_state->m_storageBlockMaxSize;
	m_capabilities.m_textureBufferBindOffsetAlignment = m_state->m_tboAlignment;
	m_capabilities.m_textureBufferMaxRange = m_state->m_tboMaxRange;
	m_capabilities.m_majorApiVersion = U(init.m_config->getNumber("gr.glmajor"));
	m_capabilities.m_minorApiVersion = U(init.m_config->getNumber("gr.glmajor"));

	initFakeDefaultFb(init);

	return Error::NONE;
}

void GrManagerImpl::initFakeDefaultFb(GrManagerInitInfo& init)
{
	U32 defaultFbWidth = init.m_window->getWidth();
	U32 defaultFbHeight = init.m_window->getHeight();

	TextureInitInfo texinit("FB Tex");
	texinit.m_width = defaultFbWidth;
	texinit.m_height = defaultFbHeight;
	texinit.m_format = Format::R8G8B8A8_UNORM;
	texinit.m_usage =
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::IMAGE_COMPUTE_WRITE | TextureUsageBit::PRESENT;
	m_fakeFbTex = newTexture(texinit);

	TextureViewPtr view = newTextureView(TextureViewInitInfo(m_fakeFbTex, "FB view"));

	FramebufferInitInfo fbinit("Dflt FB");
	fbinit.m_colorAttachmentCount = 1;
	fbinit.m_colorAttachments[0].m_textureView = view;
	m_fakeDefaultFb = newFramebuffer(fbinit);
}

} // end namespace anki
