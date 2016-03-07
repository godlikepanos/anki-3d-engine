// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/RenderingThread.h>

namespace anki
{

//==============================================================================
GrManagerImpl::~GrManagerImpl()
{
	if(m_thread)
	{
		m_thread->stop();
		m_manager->getAllocator().deleteInstance(m_thread);
		m_thread = nullptr;
	}

	destroyBackend();
	m_manager = nullptr;
}

//==============================================================================
GrAllocator<U8> GrManagerImpl::getAllocator() const
{
	return m_manager->getAllocator();
}

//==============================================================================
Error GrManagerImpl::init(GrManagerInitInfo& init)
{
	// Init the backend of the backend
	ANKI_CHECK(createBackend(init));

	// Create thread
	m_thread =
		m_manager->getAllocator().newInstance<RenderingThread>(m_manager);

	// Start it
	m_thread->start(init.m_debugContext, *init.m_config);
	m_thread->syncClientServer();

	return ErrorCode::NONE;
}

} // end namespace anki
