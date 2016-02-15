// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>

namespace anki
{

// Forward
class RenderingThread;

/// @addtogroup opengl
/// @{

/// Graphics manager backend specific.
class GrManagerImpl
{
public:
	GrManagerImpl(GrManager* manager)
		: m_manager(manager)
	{
		ANKI_ASSERT(manager);
	}

	~GrManagerImpl();

	void init(GrManagerInitInfo& init);

	const RenderingThread& getRenderingThread() const
	{
		return *m_thread;
	}

	RenderingThread& getRenderingThread()
	{
		return *m_thread;
	}

	GrAllocator<U8> getAllocator() const;

private:
	GrManager* m_manager;
	RenderingThread* m_thread = nullptr;
};
/// @}

} // end namespace anki
