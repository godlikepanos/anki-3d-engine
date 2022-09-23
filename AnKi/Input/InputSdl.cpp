// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Input/Input.h>
#include <AnKi/Input/InputSdl.h>
#include <AnKi/Core/NativeWindowSdl.h>
#include <AnKi/Util/Logger.h>
#include <SDL.h>

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

Error Input::newInstance(AllocAlignedCallback allocCallback, void* allocCallbackUserData, NativeWindow* nativeWindow,
						 Input*& input)
{
	ANKI_ASSERT(allocCallback && nativeWindow);

	HeapAllocator<U8> alloc(allocCallback, allocCallbackUserData, "Input");
	InputSdl* sdlinput = alloc.newInstance<InputSdl>(alloc);

	sdlinput->m_alloc = alloc;
	sdlinput->m_nativeWindow = nativeWindow;

	const Error err = sdlinput->init();
	if(err)
	{
		sdlinput->~InputSdl();
		alloc.getMemoryPool().free(sdlinput);
		input = nullptr;
		return err;
	}
	else
	{
		input = sdlinput;
		return Error::kNone;
	}
}

void Input::deleteInstance(Input* input)
{
	if(input)
	{
		InputSdl* self = static_cast<InputSdl*>(input);
		HeapAllocator<U8> alloc = self->m_alloc;
		alloc.deleteInstance(self);
	}
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
		const I32 x = I32(F32(m_nativeWindow->getWidth()) * (pos.x() * 0.5f + 0.5f));
		const I32 y = I32(F32(m_nativeWindow->getHeight()) * (-pos.y() * 0.5f + 0.5f));

		SDL_WarpMouseInWindow(static_cast<NativeWindowSdl*>(m_nativeWindow)->m_window, x, y);

		// SDL doesn't generate a SDL_MOUSEMOTION event if the cursor is outside the window. Push that event
		SDL_Event event;
		event.type = SDL_MOUSEMOTION;
		event.button.x = x;
		event.button.y = y;

		SDL_PushEvent(&event);
	}
}

void Input::hideCursor(Bool hide)
{
	SDL_ShowCursor(!hide);
}

Bool Input::hasTouchDevice() const
{
	return false;
}

Error InputSdl::init()
{
	ANKI_ASSERT(m_nativeWindow);

#define ANKI_KEY_CODE(ak, sdl) m_sdlToAnki[SDLK_##sdl] = KeyCode::k##ak;
#include <AnKi/Input/KeyCode.defs.h>
#undef ANKI_KEY_CODE

	// Call once to clear first events
	return handleEvents();
}

Error InputSdl::handleEventsInternal()
{
	ANKI_ASSERT(m_nativeWindow != nullptr);

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
	SDL_StartTextInput();
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_KEYDOWN:
			akkey = m_sdlToAnki[event.key.keysym.sym];
			m_keys[akkey] = 1;
			break;
		case SDL_KEYUP:
			akkey = m_sdlToAnki[event.key.keysym.sym];
			m_keys[akkey] = 0;
			break;
		case SDL_MOUSEBUTTONDOWN:
		{
			MouseButton mb = sdlMouseButtonToAnKi(event.button.button);
			if(mb != MouseButton::kCount)
			{
				m_mouseBtns[mb] = 1;
			}
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			MouseButton mb = sdlMouseButtonToAnKi(event.button.button);
			if(mb != MouseButton::kCount)
			{
				m_mouseBtns[mb] = 0;
			}
			break;
		}
		case SDL_MOUSEWHEEL:
			m_mouseBtns[MouseButton::kScrollUp] = event.wheel.y > 0;
			m_mouseBtns[MouseButton::kScrollDown] = event.wheel.y < 0;
			break;
		case SDL_MOUSEMOTION:
			m_mousePosWin.x() = event.button.x;
			m_mousePosWin.y() = event.button.y;
			m_mousePosNdc.x() = F32(event.button.x) / F32(m_nativeWindow->getWidth()) * 2.0f - 1.0f;
			m_mousePosNdc.y() = -(F32(event.button.y) / F32(m_nativeWindow->getHeight()) * 2.0f - 1.0f);
			break;
		case SDL_QUIT:
			addEvent(InputEvent::WINDOW_CLOSED);
			break;
		case SDL_TEXTINPUT:
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
