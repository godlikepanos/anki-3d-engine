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

// GamepadButton mirrors SDL_GamepadButton so the translation is a cast. Verify the ends and a few in between
static_assert(U32(GamepadButton::kCount) == U32(SDL_GAMEPAD_BUTTON_COUNT));
static_assert(U32(GamepadButton::kSouth) == U32(SDL_GAMEPAD_BUTTON_SOUTH));
static_assert(U32(GamepadButton::kDpadRight) == U32(SDL_GAMEPAD_BUTTON_DPAD_RIGHT));
static_assert(U32(GamepadButton::kTouchpad) == U32(SDL_GAMEPAD_BUTTON_TOUCHPAD));
static_assert(U32(GamepadButton::kMisc6) == U32(SDL_GAMEPAD_BUTTON_MISC6));

// A stick doesn't rest exactly at zero. SDL documents the resting point as being within ~8000 of it, which is about a quarter of the range
constexpr F32 kGamepadStickDeadzone = 0.25f;

// Read a stick as a vector in [-1, 1] with +Y being up
static Vec2 readGamepadStick(SDL_Gamepad* gamepad, SDL_GamepadAxis xAxis, SDL_GamepadAxis yAxis)
{
	ANKI_ASSERT(gamepad);

	// SDL reports +Y downwards. Also the negative end of the range is -32768 so the division needs a clamp
	Vec2 stick(F32(SDL_GetGamepadAxis(gamepad, xAxis)), -F32(SDL_GetGamepadAxis(gamepad, yAxis)));
	stick = (stick / 32767.0f).clamp(-1.0f, 1.0f);

	// Deadzone the magnitude and not each axis on its own, otherwise the dead region is a square and diagonals snap to the axes
	const F32 len = stick.length();
	if(len <= kGamepadStickDeadzone)
	{
		return Vec2(0.0f);
	}

	// Rescale so the magnitude ramps from 0 at the deadzone edge up to 1, otherwise it jumps the moment the stick leaves the deadzone
	return stick.normalize() * min((len - kGamepadStickDeadzone) / (1.0f - kGamepadStickDeadzone), 1.0f);
}

// Read a trigger in [0, 1]. Unlike the sticks, SDL never reports a negative value for these
static F32 readGamepadTrigger(SDL_Gamepad* gamepad, SDL_GamepadAxis axis)
{
	ANKI_ASSERT(gamepad);
	return clamp(F32(SDL_GetGamepadAxis(gamepad, axis)) / 32767.0f, 0.0f, 1.0f);
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
	InputSdl& self = *static_cast<InputSdl*>(this);
	return self.handleEventsInternal();
}

Bool Input::hasTouchDevice() const
{
	return false;
}

Bool Input::hasGamepad() const
{
	const InputSdl& self = *static_cast<const InputSdl*>(this);
	return self.m_gamepad != nullptr;
}

InputSdl::~InputSdl()
{
	closeGamepad();

	for(MouseCursor cursor : EnumIterable<MouseCursor>())
	{
		if(m_cursors[cursor])
		{
			SDL_DestroyCursor(m_cursors[cursor]);
		}
	}
}

void InputSdl::openFirstGamepad()
{
	if(m_gamepad)
	{
		return;
	}

	I32 count;
	SDL_JoystickID* ids = SDL_GetGamepads(&count);
	if(!ids)
	{
		ANKI_WIND_LOGE("SDL_GetGamepads() failed: %s", SDL_GetError());
		return;
	}

	for(I32 i = 0; i < count; ++i)
	{
		m_gamepad = SDL_OpenGamepad(ids[i]);
		if(m_gamepad)
		{
			ANKI_WIND_LOGI("Gamepad connected: %s", SDL_GetGamepadName(m_gamepad));
			break;
		}

		ANKI_WIND_LOGE("SDL_OpenGamepad() failed: %s", SDL_GetError());
	}

	SDL_free(ids);
}

void InputSdl::closeGamepad()
{
	if(m_gamepad)
	{
		SDL_CloseGamepad(m_gamepad);
		m_gamepad = nullptr;
	}

	zeroMemory(m_gamepadBtns);
	zeroMemory(m_gamepadSticks);
	zeroMemory(m_gamepadTriggers);
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
	for(I32& k : m_gamepadBtns)
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
		case SDL_EVENT_GAMEPAD_ADDED:
			// SDL queues one of these for every gamepad that was already connected at SDL_Init, so this is the startup path as well
			openFirstGamepad();
			break;
		case SDL_EVENT_GAMEPAD_REMOVED:
			if(m_gamepad && event.gdevice.which == SDL_GetGamepadID(m_gamepad))
			{
				ANKI_WIND_LOGI("Gamepad removed: %s", SDL_GetGamepadName(m_gamepad));
				closeGamepad();

				// Fall back to another gamepad if one is still around
				openFirstGamepad();
			}
			break;
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			// SDL doesn't repeat these while the button is held so there's no repeat flag to skip, unlike the keyboard
			if(m_gamepad && event.gbutton.which == SDL_GetGamepadID(m_gamepad) && event.gbutton.button < U8(GamepadButton::kCount))
			{
				m_gamepadBtns[GamepadButton(event.gbutton.button)] = (event.gbutton.down) ? 1 : -1;
			}
			break;
		}
	} // end while events

	// Sticks and triggers are polled and not read from events because a held stick emits a flood of motion events but only the latest value
	// matters. SDL_PollEvent() above has already refreshed the state so there's no need for a SDL_UpdateGamepads()
	if(m_gamepad)
	{
		m_gamepadSticks[GamepadStick::kLeft] = readGamepadStick(m_gamepad, SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTY);
		m_gamepadSticks[GamepadStick::kRight] = readGamepadStick(m_gamepad, SDL_GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTY);

		m_gamepadTriggers[GamepadTrigger::kLeft] = readGamepadTrigger(m_gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
		m_gamepadTriggers[GamepadTrigger::kRight] = readGamepadTrigger(m_gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
	}

	if(scrollKeyEvent != MouseButton::kScrollDown)
	{
		m_mouseBtns[MouseButton::kScrollDown] = (m_mouseBtns[MouseButton::kScrollDown] > 0) ? -1 : 0;
	}

	if(scrollKeyEvent != MouseButton::kScrollUp)
	{
		m_mouseBtns[MouseButton::kScrollUp] = (m_mouseBtns[MouseButton::kScrollUp] > 0) ? -1 : 0;
	}

	// Do that at the end to be processed at the next frame so that will give some time for the mouse to move
	if(m_lockMouse.getNonAtomically() || m_requests.m_mousePosNdc != Vec2(kMaxF32))
	{
		Vec2 windPos(F32(NativeWindow::getSingleton().getWidth()), F32(NativeWindow::getSingleton().getHeight()));
		if(m_lockMouse.getNonAtomically())
		{
			windPos *= 0.5f;
		}
		else
		{
			windPos *= Vec2(m_requests.m_mousePosNdc.x, -m_requests.m_mousePosNdc.y) * 0.5f + 0.5f;
		}

		SDL_WarpMouseInWindow(static_cast<NativeWindowSdl&>(NativeWindow::getSingleton()).m_sdlWindow, windPos.x, windPos.y);

		// SDL doesn't generate a SDL_MOUSEMOTION event if the cursor is outside the window. Push that event
		SDL_Event event;
		event.type = SDL_EVENT_MOUSE_MOTION;
		event.button.x = windPos.x;
		event.button.y = windPos.y;

		SDL_PushEvent(&event);

		m_requests.m_mousePosNdc = Vec2(kMaxF32);
	}

	// Cursor change request
	if(m_requests.m_mouseCursor != MouseCursor::kCount)
	{
		SDL_SetCursor(m_cursors[m_requests.m_mouseCursor]);
		m_requests.m_mouseCursor = MouseCursor::kCount;
	}

	// Hide cursor request
	if(m_requests.m_hideCursor.getNonAtomically() >= 0)
	{
		const Bool hide = m_requests.m_hideCursor.getNonAtomically() != 0;

		if(hide)
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

		m_requests.m_hideCursor.setNonAtomically(-1);
	}

	return Error::kNone;
}

} // end namespace anki
