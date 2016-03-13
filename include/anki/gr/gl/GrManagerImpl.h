// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>

namespace anki
{

// Forward
class RenderingThread;
class WindowingBackend;

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

	ANKI_USE_RESULT Error init(GrManagerInitInfo& init);

	const RenderingThread& getRenderingThread() const
	{
		return *m_thread;
	}

	RenderingThread& getRenderingThread()
	{
		return *m_thread;
	}

	GrAllocator<U8> getAllocator() const;

	void swapBuffers();

	void pinContextToCurrentThread(Bool pin);

private:
	GrManager* m_manager;
	RenderingThread* m_thread = nullptr;
	WindowingBackend* m_backend = nullptr; ///< The backend of the backend.

	ANKI_USE_RESULT Error createBackend(GrManagerInitInfo& init);
	void destroyBackend();
};
/// @}

} // end namespace anki
