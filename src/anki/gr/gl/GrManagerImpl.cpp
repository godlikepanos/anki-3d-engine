// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/GlState.h>
#include <anki/core/Config.h>

namespace anki
{

GrManagerImpl::~GrManagerImpl()
{
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
	m_capabilities.m_shaderSubgroups = !!(m_state->m_extensions & GlExtensions::ARB_SHADER_BALLOT);
	m_capabilities.m_uniformBufferBindOffsetAlignment = m_state->m_uboAlignment;
	m_capabilities.m_uniformBufferMaxRange = m_state->m_uniBlockMaxSize;
	m_capabilities.m_storageBufferBindOffsetAlignment = m_state->m_ssboAlignment;
	m_capabilities.m_storageBufferMaxRange = m_state->m_storageBlockMaxSize;
	m_capabilities.m_textureBufferBindOffsetAlignment = m_state->m_tboAlignment;
	m_capabilities.m_textureBufferMaxRange = m_state->m_tboMaxRange;

	return Error::NONE;
}

} // end namespace anki
