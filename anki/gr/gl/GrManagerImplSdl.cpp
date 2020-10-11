// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file Contains the SDL specific bits of the GL implamentation

#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/core/NativeWindow.h>
#include <anki/core/NativeWindowSdl.h>
#include <anki/core/Config.h>
#include <SDL.h>
#include <GL/glew.h>

namespace anki
{

/// SDL implementation of WindowingBackend
class WindowingBackend
{
public:
	SDL_GLContext m_context = nullptr;
	SDL_Window* m_window = nullptr;

	~WindowingBackend()
	{
		if(m_context)
		{
			SDL_GL_DeleteContext(m_context);
		}
	}

	Error createContext(GrManagerInitInfo& init)
	{
		m_window = init.m_window->getNative().m_window;

		ANKI_GL_LOGI("Creating GL %u.%u context...", U(init.m_config->getNumber("gr.glmajor")),
					 U(init.m_config->getNumber("gr.glminor")));

		if(init.m_config->getNumber("gr_debugContext"))
		{
			if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG))
			{
				ANKI_GL_LOGE("SDL_GL_SetAttribute() failed");
				return Error::FUNCTION_FAILED;
			}
		}

		if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, init.m_config->getNumber("gr.glmajor"))
		   || SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, init.m_config->getNumber("gr.glminor"))
		   || SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))
		{
			ANKI_GL_LOGE("SDL_GL_SetAttribute() failed");
			return Error::FUNCTION_FAILED;
		}

		// Create context
		m_context = SDL_GL_CreateContext(m_window);
		if(m_context == nullptr)
		{
			ANKI_GL_LOGE("SDL_GL_CreateContext() failed: %s", SDL_GetError());
			return Error::FUNCTION_FAILED;
		}

		// GLEW
		glewExperimental = GL_TRUE;
		if(glewInit() != GLEW_OK)
		{
			ANKI_GL_LOGE("GLEW initialization failed");
			return Error::FUNCTION_FAILED;
		}
		glGetError();

		return Error::NONE;
	}

	void pinContextToCurrentThread(Bool pin)
	{
		SDL_GLContext pinCtx = (pin) ? m_context : nullptr;
		SDL_GL_MakeCurrent(m_window, pinCtx);
	}

	void swapBuffers()
	{
		SDL_GL_SwapWindow(m_window);
	}
};

Error GrManagerImpl::createBackend(GrManagerInitInfo& init)
{
	ANKI_ASSERT(m_backend == nullptr);

	m_backend = getAllocator().newInstance<WindowingBackend>();
	return m_backend->createContext(init);
}

void GrManagerImpl::destroyBackend()
{
	if(m_backend)
	{
		getAllocator().deleteInstance(m_backend);
	}
}

void GrManagerImpl::swapBuffers()
{
	ANKI_ASSERT(m_backend);
	m_backend->swapBuffers();
}

void GrManagerImpl::pinContextToCurrentThread(Bool pin)
{
	ANKI_ASSERT(m_backend);
	m_backend->pinContextToCurrentThread(pin);
}

} // end namespace anki
