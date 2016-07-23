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
class GlState;
class TransientMemoryManager;

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

	TransientMemoryManager& getTransientMemoryManager()
	{
		ANKI_ASSERT(m_transManager);
		return *m_transManager;
	}

	const TransientMemoryManager& getTransientMemoryManager() const
	{
		ANKI_ASSERT(m_transManager);
		return *m_transManager;
	}

	GrAllocator<U8> getAllocator() const;

	void swapBuffers();

	void pinContextToCurrentThread(Bool pin);

private:
	GrManager* m_manager;
	GlState* m_state = nullptr;
	RenderingThread* m_thread = nullptr;
	WindowingBackend* m_backend = nullptr; ///< The backend of the backend.
	TransientMemoryManager* m_transManager = nullptr;

	ANKI_USE_RESULT Error createBackend(GrManagerInitInfo& init);
	void destroyBackend();
};
/// @}

} // end namespace anki
