// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/Common.h>

namespace anki
{

// Forward
class RenderingThread;
class WindowingBackend;
class GlState;

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
		ANKI_ASSERT(m_thread);
		return *m_thread;
	}

	RenderingThread& getRenderingThread()
	{
		ANKI_ASSERT(m_thread);
		return *m_thread;
	}

	GlState& getState()
	{
		ANKI_ASSERT(m_state);
		return *m_state;
	}

	const GlState& getState() const
	{
		ANKI_ASSERT(m_state);
		return *m_state;
	}

	GrAllocator<U8> getAllocator() const;

	void swapBuffers();

	void pinContextToCurrentThread(Bool pin);

private:
	GrManager* m_manager;
	GlState* m_state = nullptr;
	RenderingThread* m_thread = nullptr;
	WindowingBackend* m_backend = nullptr; ///< The backend of the backend.

	ANKI_USE_RESULT Error createBackend(GrManagerInitInfo& init);
	void destroyBackend();
};
/// @}

} // end namespace anki
