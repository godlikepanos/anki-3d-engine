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

void Input::moveMouseNdc(const Vec2& pos)
{
	if(pos != m_mousePosNdc)
	{
		const F32 x = F32(NativeWindow::getSingleton().getWidth()) * (pos.x * 0.5f + 0.5f);
		const F32 y = F32(NativeWindow::getSingleton().getHeight()) * (-pos.y * 0.5f + 0.5f);

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
	InputSdl* self = static_cast<InputSdl*>(this);
	self->m_crntHideCursor = hide;
}

Bool Input::hasTouchDevice() const
{
	return false;
}

void Input::setMouseCursor(MouseCursor cursor)
{
	ANKI_ASSERT(cursor < MouseCursor::kCount);
	InputSdl* self = static_cast<InputSdl*>(this);
	self->m_crntMouseCursor = cursor;
}

InputSdl::~InputSdl()
{
	for(MouseCursor cursor : EnumIterable<MouseCursor>())
	{
		if(m_cursors[cursor])
		{
			SDL_DestroyCursor(m_cursors[cursor]);
		}
	}
}

Error InputSdl::initInternal()
{
	m_cursors[MouseCursor::kArrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	m_cursors[MouseCursor::kTextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
	m_cursors[MouseCursor::kResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
	m_cursors[MouseCursor::kResizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
	m_cursors[MouseCursor::kResizeEW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
	m_cursors[MouseCursor::kResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE);
	m_cursors[MouseCursor::kResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
	m_cursors[MouseCursor::kHand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
	m_cursors[MouseCursor::kWait] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
	m_cursors[MouseCursor::kProgress] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_PROGRESS);
	m_cursors[MouseCursor::kNotAllowed] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NOT_ALLOWED);

	for(MouseCursor cursor : EnumIterable<MouseCursor>())
	{
		if(!m_cursors[cursor])
		{
			ANKI_WIND_LOGE("Failed to create cursor: %u", U32(cursor));
			return Error::kFunctionFailed;
		}
	}

	// Call once to clear first events
	return handleEvents();
}

Error InputSdl::handleEventsInternal()
{
	m_textInput[0] = '\0';

	// add the times a key is being pressed
	for(I32& k : m_keys)
	{
		if(k > 0)
		{
			++k;
		}
		else if(k < 0)
		{
			k = 0;
		}
	}
	for(I32& k : m_mouseBtns)
	{
		if(k > 0)
		{
			++k;
		}
		else if(k < 0)
		{
			k = 0;
		}
	}

	m_prevMousePosNdc = m_mousePosNdc;

	SDL_Event event = {};
	KeyCode akkey = KeyCode::kCount;
	if(!SDL_StartTextInput(static_cast<NativeWindowSdl&>(NativeWindow::getSingleton()).m_sdlWindow))
	{
		ANKI_WIND_LOGE("SDL_StartTextInput() failed: %s", SDL_GetError());
	}

	MouseButton scrollKeyEvent = MouseButton::kCount;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_EVENT_KEY_DOWN:
			// key.repeat adds a delay but we only want the 1st time the key is pressed
			if(!event.key.repeat)
			{
				akkey = sdlKeytoAnKi(event.key.key);
				m_keys[akkey] = 1;
			}
			break;
		case SDL_EVENT_KEY_UP:
			akkey = sdlKeytoAnKi(event.key.key);
			m_keys[akkey] = -1;
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
				m_mouseBtns[mb] = -1;
			}
			break;
		}
		case SDL_EVENT_MOUSE_WHEEL:
		{
			const MouseButton btn = (event.wheel.y > 0.0f) ? MouseButton::kScrollUp : MouseButton::kScrollDown;
			m_mouseBtns[btn] = max(m_mouseBtns[btn] + 1, 1);
			scrollKeyEvent = btn;
			break;
		}
		case SDL_EVENT_MOUSE_MOTION:
			m_mousePosNdc.x = F32(event.button.x) / F32(NativeWindow::getSingleton().getWidth()) * 2.0f - 1.0f;
			m_mousePosNdc.y = -(F32(event.button.y) / F32(NativeWindow::getSingleton().getHeight()) * 2.0f - 1.0f);
			break;
		case SDL_EVENT_QUIT:
			ANKI_WIND_LOGI("Recieved SDL_EVENT_QUIT");
			addEvent(InputEvent::kWindowClosed);
			break;
		case SDL_EVENT_TEXT_INPUT:
			std::strncpy(&m_textInput[0], event.text.text, m_textInput.getSize() - 1);
			break;
		}
	} // end while events

	if(scrollKeyEvent != MouseButton::kScrollDown)
	{
		m_mouseBtns[MouseButton::kScrollDown] = (m_mouseBtns[MouseButton::kScrollDown] > 0) ? -1 : 0;
	}

	if(scrollKeyEvent != MouseButton::kScrollUp)
	{
		m_mouseBtns[MouseButton::kScrollUp] = (m_mouseBtns[MouseButton::kScrollUp] > 0) ? -1 : 0;
	}

	// Lock mouse
	if(m_lockCurs)
	{
		moveMouseNdc(Vec2(0.0f));
	}

	if(m_crntMouseCursor != m_prevMouseCursor)
	{
		SDL_SetCursor(m_cursors[m_crntMouseCursor]);
		m_prevMouseCursor = m_crntMouseCursor;
	}

	if(m_crntHideCursor != m_prevHideCursor)
	{
		m_prevHideCursor = m_crntHideCursor;

		if(m_crntHideCursor)
		{
			if(!SDL_HideCursor())
			{
				ANKI_WIND_LOGE("SDL_HideCursor() failed: %s", SDL_GetError());
			}

			if(!SDL_SetWindowRelativeMouseMode(static_cast<NativeWindowSdl&>(NativeWindow::getSingleton()).m_sdlWindow, true))
			{
				ANKI_WIND_LOGE("SDL_SetWindowRelativeMouseMode() failed: %s", SDL_GetError());
			}
		}
		else
		{
			if(!SDL_ShowCursor())
			{
				ANKI_WIND_LOGE("SDL_ShowCursor() failed: %s", SDL_GetError());
			}

			if(!SDL_SetWindowRelativeMouseMode(static_cast<NativeWindowSdl&>(NativeWindow::getSingleton()).m_sdlWindow, false))
			{
				ANKI_WIND_LOGE("SDL_SetWindowRelativeMouseMode() failed: %s", SDL_GetError());
			}
		}
	}

	return Error::kNone;
}

} // end namespace anki
