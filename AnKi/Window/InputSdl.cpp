// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Window/Input.h>
#include <AnKi/Window/InputSdl.h>
#include <AnKi/Window/NativeWindowSdl.h>
#include <AnKi/Util/Logger.h>
#include <SDL3/SDL.h>

namespace anki {

static MouseButton sdlMouseButtonToAnKi(const U32 sdl)
{
	MouseButton out = MouseButton::kCount;

	switch(sdl)
	{
	case SDL_BUTTON_LEFT:
		out = MouseButton::kLeft;
		break;
	case SDL_BUTTON_RIGHT:
		out = MouseButton::kRight;
		break;
	case SDL_BUTTON_MIDDLE:
		out = MouseButton::kMiddle;
		break;
	}

	return out;
}

static KeyCode sdlKeytoAnKi(SDL_Keycode sdlk)
{
	KeyCode akk = KeyCode::kUnknown;
	switch(sdlk)
	{
#define ANKI_KEY_CODE(ak, sdl) \
	case SDLK_##sdl: \
		akk = KeyCode::k##ak; \
		break;
#include <AnKi/Window/KeyCode.def.h>
#undef ANKI_KEY_CODE
	}

	ANKI_ASSERT(akk != KeyCode::kUnknown);
	return akk;
}

template<>
template<>
Input& MakeSingletonPtr<Input>::allocateSingleton<>()
{
	ANKI_ASSERT(m_global == nullptr);
	m_global = new InputSdl;

#if ANKI_ASSERTIONS_ENABLED
	++g_singletonsAllocated;
#endif

	return *m_global;
}

template<>
void MakeSingletonPtr<Input>::freeSingleton()
{
	if(m_global)
	{
		delete static_cast<InputSdl*>(m_global);
		m_global = nullptr;
#if ANKI_ASSERTIONS_ENABLED
		--g_singletonsAllocated;
#endif
	}
}

Error Input::init()
{
	return static_cast<InputSdl*>(this)->initInternal();
}

Error Input::handleEvents()
{
	InputSdl* self = static_cast<InputSdl*>(this);
	return self->handleEventsInternal();
}

void Input::moveCursor(const Vec2& pos)
{
	if(pos != m_mousePosNdc)
	{
		const F32 x = F32(NativeWindow::getSingleton().getWidth()) * (pos.x() * 0.5f + 0.5f);
		const F32 y = F32(NativeWindow::getSingleton().getHeight()) * (-pos.y() * 0.5f + 0.5f);

		SDL_WarpMouseInWindow(static_cast<NativeWindowSdl&>(NativeWindow::getSingleton()).m_sdlWindow, x, y);

		// SDL doesn't generate a SDL_MOUSEMOTION event if the cursor is outside the window. Push that event
		SDL_Event event;
		event.type = SDL_EVENT_MOUSE_MOTION;
		event.button.x = x;
		event.button.y = y;

		SDL_PushEvent(&event);
	}
}

void Input::hideCursor(Bool hide)
{
	if(hide)
	{
		if(!SDL_HideCursor())
		{
			ANKI_WIND_LOGE("SDL_HideCursor() failed: %s", SDL_GetError());
		}
	}
	else
	{
		if(!SDL_ShowCursor())
		{
			ANKI_WIND_LOGE("SDL_ShowCursor() failed: %s", SDL_GetError());
		}
	}
}

Bool Input::hasTouchDevice() const
{
	return false;
}

Error InputSdl::initInternal()
{
	// Call once to clear first events
	return handleEvents();
}

Error InputSdl::handleEventsInternal()
{
	m_textInput[0] = '\0';

	// add the times a key is being pressed
	for(auto& k : m_keys)
	{
		if(k)
		{
			++k;
		}
	}
	for(auto& k : m_mouseBtns)
	{
		if(k)
		{
			++k;
		}
	}

	SDL_Event event;
	KeyCode akkey;
	if(!SDL_StartTextInput(static_cast<NativeWindowSdl&>(NativeWindow::getSingleton()).m_sdlWindow))
	{
		ANKI_WIND_LOGE("SDL_StartTextInput() failed: %s", SDL_GetError());
	}

	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_EVENT_KEY_DOWN:
			akkey = sdlKeytoAnKi(event.key.key);
			m_keys[akkey] = 1;
			break;
		case SDL_EVENT_KEY_UP:
			akkey = sdlKeytoAnKi(event.key.key);
			m_keys[akkey] = 0;
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		{
			MouseButton mb = sdlMouseButtonToAnKi(event.button.button);
			if(mb != MouseButton::kCount)
			{
				m_mouseBtns[mb] = 1;
			}
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			MouseButton mb = sdlMouseButtonToAnKi(event.button.button);
			if(mb != MouseButton::kCount)
			{
				m_mouseBtns[mb] = 0;
			}
			break;
		}
		case SDL_EVENT_MOUSE_WHEEL:
			m_mouseBtns[MouseButton::kScrollUp] = event.wheel.y > 0.0f;
			m_mouseBtns[MouseButton::kScrollDown] = event.wheel.y < 0.0f;
			break;
		case SDL_EVENT_MOUSE_MOTION:
			m_mousePosWin.x() = U32(event.button.x);
			m_mousePosWin.y() = U32(event.button.y);
			m_mousePosNdc.x() = F32(event.button.x) / F32(NativeWindow::getSingleton().getWidth()) * 2.0f - 1.0f;
			m_mousePosNdc.y() = -(F32(event.button.y) / F32(NativeWindow::getSingleton().getHeight()) * 2.0f - 1.0f);
			break;
		case SDL_EVENT_QUIT:
			addEvent(InputEvent::kWindowClosed);
			break;
		case SDL_EVENT_TEXT_INPUT:
			std::strncpy(&m_textInput[0], event.text.text, m_textInput.getSize() - 1);
			break;
		}
	} // end while events

	// Lock mouse
	if(m_lockCurs)
	{
		moveCursor(Vec2(0.0));
	}

	return Error::kNone;
}

} // end namespace anki
