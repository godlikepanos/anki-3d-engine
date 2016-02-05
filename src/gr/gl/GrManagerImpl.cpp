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

	m_manager = nullptr;
}

//==============================================================================
GrAllocator<U8> GrManagerImpl::getAllocator() const
{
	return m_manager->getAllocator();
}

//==============================================================================
void GrManagerImpl::create(GrManagerInitInfo& init)
{
	// Create thread
	m_thread =
		m_manager->getAllocator().newInstance<RenderingThread>(m_manager);

	// Start it
	m_thread->start(
		init.m_interface, init.m_registerDebugMessages, *init.m_config);
	m_thread->syncClientServer();
}

} // end namespace anki
