// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrManager.h>
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
class GrManagerImpl final : public GrManager
{
public:
	/// Dummy framebuffer texture that emulates functionality vulkan can but GL can't
	TexturePtr m_fakeFbTex;
	FramebufferPtr m_fakeDefaultFb;

	GrManagerImpl();

	~GrManagerImpl();

	ANKI_USE_RESULT Error init(GrManagerInitInfo& init, GrAllocator<U8> alloc);

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

	void swapBuffers();

	void pinContextToCurrentThread(Bool pin);

	Bool debugMarkersEnabled() const
	{
		return m_debugMarkers;
	}

private:
	GlState* m_state = nullptr;
	RenderingThread* m_thread = nullptr;
	WindowingBackend* m_backend = nullptr; ///< The backend of the backend.
	Bool m_debugMarkers = false;

	ANKI_USE_RESULT Error createBackend(GrManagerInitInfo& init);
	void destroyBackend();

	void initFakeDefaultFb(GrManagerInitInfo& init);
};
/// @}

} // end namespace anki
