// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/input/Input.h>
#include <anki/input/InputSdl.h>
#include <anki/core/NativeWindowSdl.h>
#include <anki/util/Logger.h>
#include <SDL.h>

namespace anki
{

static MouseButton sdlMouseButtonToAnKi(const U32 sdl)
{
	MouseButton out = MouseButton::COUNT;

	switch(sdl)
	{
	case SDL_BUTTON_LEFT:
		out = MouseButton::LEFT;
		break;
	case SDL_BUTTON_RIGHT:
		out = MouseButton::RIGHT;
		break;
	case SDL_BUTTON_MIDDLE:
		out = MouseButton::MIDDLE;
		break;
	}

	return out;
}

Error Input::initInternal(NativeWindow* nativeWindow)
{
	ANKI_ASSERT(nativeWindow);
	m_nativeWindow = nativeWindow;

	// Init native
	HeapAllocator<std::pair<const SDL_Keycode, KeyCode>> alloc = m_nativeWindow->_getAllocator();

	m_impl = m_nativeWindow->_getAllocator().newInstance<InputImpl>(alloc);

// impl
#define MAP(sdl, ak) m_impl->m_sdlToAnki[sdl] = KeyCode::ak

	MAP(SDLK_RETURN, RETURN);
	MAP(SDLK_ESCAPE, ESCAPE);
	MAP(SDLK_BACKSPACE, BACKSPACE);
	MAP(SDLK_TAB, TAB);
	MAP(SDLK_SPACE, SPACE);
	MAP(SDLK_EXCLAIM, EXCLAIM);
	MAP(SDLK_QUOTEDBL, QUOTEDBL);
	MAP(SDLK_HASH, HASH);
	MAP(SDLK_PERCENT, PERCENT);
	MAP(SDLK_DOLLAR, DOLLAR);
	MAP(SDLK_AMPERSAND, AMPERSAND);
	MAP(SDLK_QUOTE, QUOTE);
	MAP(SDLK_LEFTPAREN, LEFTPAREN);
	MAP(SDLK_RIGHTPAREN, RIGHTPAREN);
	MAP(SDLK_ASTERISK, ASTERISK);
	MAP(SDLK_PLUS, PLUS);
	MAP(SDLK_COMMA, COMMA);
	MAP(SDLK_MINUS, MINUS);
	MAP(SDLK_PERIOD, PERIOD);
	MAP(SDLK_SLASH, SLASH);
	MAP(SDLK_0, _0);
	MAP(SDLK_1, _1);
	MAP(SDLK_2, _2);
	MAP(SDLK_3, _3);
	MAP(SDLK_4, _4);
	MAP(SDLK_5, _5);
	MAP(SDLK_6, _6);
	MAP(SDLK_7, _7);
	MAP(SDLK_8, _8);
	MAP(SDLK_9, _9);
	MAP(SDLK_COLON, COLON);
	MAP(SDLK_SEMICOLON, SEMICOLON);
	MAP(SDLK_LESS, LESS);
	MAP(SDLK_EQUALS, EQUALS);
	MAP(SDLK_GREATER, GREATER);
	MAP(SDLK_QUESTION, QUESTION);
	MAP(SDLK_AT, AT);
	MAP(SDLK_LEFTBRACKET, LEFTBRACKET);
	MAP(SDLK_BACKSLASH, BACKSLASH);
	MAP(SDLK_RIGHTBRACKET, RIGHTBRACKET);
	MAP(SDLK_CARET, CARET);
	MAP(SDLK_UNDERSCORE, UNDERSCORE);
	MAP(SDLK_BACKQUOTE, BACKQUOTE);
	MAP(SDLK_a, A);
	MAP(SDLK_b, B);
	MAP(SDLK_c, C);
	MAP(SDLK_d, D);
	MAP(SDLK_e, E);
	MAP(SDLK_f, F);
	MAP(SDLK_g, G);
	MAP(SDLK_h, H);
	MAP(SDLK_i, I);
	MAP(SDLK_j, J);
	MAP(SDLK_k, K);
	MAP(SDLK_l, L);
	MAP(SDLK_m, M);
	MAP(SDLK_n, N);
	MAP(SDLK_o, O);
	MAP(SDLK_p, P);
	MAP(SDLK_q, Q);
	MAP(SDLK_r, R);
	MAP(SDLK_s, S);
	MAP(SDLK_t, T);
	MAP(SDLK_u, U);
	MAP(SDLK_v, V);
	MAP(SDLK_w, W);
	MAP(SDLK_x, X);
	MAP(SDLK_y, Y);
	MAP(SDLK_z, Z);
	MAP(SDLK_CAPSLOCK, CAPSLOCK);
	MAP(SDLK_F1, F1);
	MAP(SDLK_F2, F2);
	MAP(SDLK_F3, F3);
	MAP(SDLK_F4, F4);
	MAP(SDLK_F5, F5);
	MAP(SDLK_F6, F6);
	MAP(SDLK_F7, F7);
	MAP(SDLK_F8, F8);
	MAP(SDLK_F9, F9);
	MAP(SDLK_F10, F10);
	MAP(SDLK_F11, F11);
	MAP(SDLK_F12, F12);
	MAP(SDLK_PRINTSCREEN, PRINTSCREEN);
	MAP(SDLK_SCROLLLOCK, SCROLLLOCK);
	MAP(SDLK_PAUSE, PAUSE);
	MAP(SDLK_INSERT, INSERT);
	MAP(SDLK_HOME, HOME);
	MAP(SDLK_PAGEUP, PAGEUP);
	MAP(SDLK_DELETE, DELETE);
	MAP(SDLK_END, END);
	MAP(SDLK_PAGEDOWN, PAGEDOWN);
	MAP(SDLK_RIGHT, RIGHT);
	MAP(SDLK_LEFT, LEFT);
	MAP(SDLK_DOWN, DOWN);
	MAP(SDLK_UP, UP);
	MAP(SDLK_NUMLOCKCLEAR, NUMLOCKCLEAR);
	MAP(SDLK_KP_DIVIDE, KP_DIVIDE);
	MAP(SDLK_KP_MULTIPLY, KP_MULTIPLY);
	MAP(SDLK_KP_MINUS, KP_MINUS);
	MAP(SDLK_KP_PLUS, KP_PLUS);
	MAP(SDLK_KP_ENTER, KP_ENTER);
	MAP(SDLK_KP_1, KP_1);
	MAP(SDLK_KP_2, KP_2);
	MAP(SDLK_KP_3, KP_3);
	MAP(SDLK_KP_4, KP_4);
	MAP(SDLK_KP_5, KP_5);
	MAP(SDLK_KP_6, KP_6);
	MAP(SDLK_KP_7, KP_7);
	MAP(SDLK_KP_8, KP_8);
	MAP(SDLK_KP_9, KP_9);
	MAP(SDLK_KP_0, KP_0);
	MAP(SDLK_KP_PERIOD, KP_PERIOD);
	MAP(SDLK_APPLICATION, APPLICATION);
	MAP(SDLK_POWER, POWER);
	MAP(SDLK_KP_EQUALS, KP_EQUALS);
	MAP(SDLK_F13, F13);
	MAP(SDLK_F14, F14);
	MAP(SDLK_F15, F15);
	MAP(SDLK_F16, F16);
	MAP(SDLK_F17, F17);
	MAP(SDLK_F18, F18);
	MAP(SDLK_F19, F19);
	MAP(SDLK_F20, F20);
	MAP(SDLK_F21, F21);
	MAP(SDLK_F22, F22);
	MAP(SDLK_F23, F23);
	MAP(SDLK_F24, F24);
	MAP(SDLK_EXECUTE, EXECUTE);
	MAP(SDLK_HELP, HELP);
	MAP(SDLK_MENU, MENU);
	MAP(SDLK_SELECT, SELECT);
	MAP(SDLK_STOP, STOP);
	MAP(SDLK_AGAIN, AGAIN);
	MAP(SDLK_UNDO, UNDO);
	MAP(SDLK_CUT, CUT);
	MAP(SDLK_COPY, COPY);
	MAP(SDLK_PASTE, PASTE);
	MAP(SDLK_FIND, FIND);
	MAP(SDLK_MUTE, MUTE);
	MAP(SDLK_VOLUMEUP, VOLUMEUP);
	MAP(SDLK_VOLUMEDOWN, VOLUMEDOWN);
	MAP(SDLK_KP_COMMA, KP_COMMA);
	MAP(SDLK_KP_EQUALSAS400, KP_EQUALSAS400);
	MAP(SDLK_ALTERASE, ALTERASE);
	MAP(SDLK_SYSREQ, SYSREQ);
	MAP(SDLK_CANCEL, CANCEL);
	MAP(SDLK_CLEAR, CLEAR);
	MAP(SDLK_PRIOR, PRIOR);
	MAP(SDLK_RETURN2, RETURN2);
	MAP(SDLK_SEPARATOR, SEPARATOR);
	MAP(SDLK_OUT, OUT);
	MAP(SDLK_OPER, OPER);
	MAP(SDLK_CLEARAGAIN, CLEARAGAIN);
	MAP(SDLK_CRSEL, CRSEL);
	MAP(SDLK_EXSEL, EXSEL);
	MAP(SDLK_KP_00, KP_00);
	MAP(SDLK_KP_000, KP_000);
	MAP(SDLK_THOUSANDSSEPARATOR, THOUSANDSSEPARATOR);
	MAP(SDLK_DECIMALSEPARATOR, DECIMALSEPARATOR);
	MAP(SDLK_CURRENCYUNIT, CURRENCYUNIT);
	MAP(SDLK_CURRENCYSUBUNIT, CURRENCYSUBUNIT);
	MAP(SDLK_KP_LEFTPAREN, KP_LEFTPAREN);
	MAP(SDLK_KP_RIGHTPAREN, KP_RIGHTPAREN);
	MAP(SDLK_KP_LEFTBRACE, KP_LEFTBRACE);
	MAP(SDLK_KP_RIGHTBRACE, KP_RIGHTBRACE);
	MAP(SDLK_KP_TAB, KP_TAB);
	MAP(SDLK_KP_BACKSPACE, KP_BACKSPACE);
	MAP(SDLK_KP_A, KP_A);
	MAP(SDLK_KP_B, KP_B);
	MAP(SDLK_KP_C, KP_C);
	MAP(SDLK_KP_D, KP_D);
	MAP(SDLK_KP_E, KP_E);
	MAP(SDLK_KP_F, KP_F);
	MAP(SDLK_KP_XOR, KP_XOR);
	MAP(SDLK_KP_POWER, KP_POWER);
	MAP(SDLK_KP_PERCENT, KP_PERCENT);
	MAP(SDLK_KP_LESS, KP_LESS);
	MAP(SDLK_KP_GREATER, KP_GREATER);
	MAP(SDLK_KP_AMPERSAND, KP_AMPERSAND);
	MAP(SDLK_KP_DBLAMPERSAND, KP_DBLAMPERSAND);
	MAP(SDLK_KP_VERTICALBAR, KP_VERTICALBAR);
	MAP(SDLK_KP_DBLVERTICALBAR, KP_DBLVERTICALBAR);
	MAP(SDLK_KP_COLON, KP_COLON);
	MAP(SDLK_KP_HASH, KP_HASH);
	MAP(SDLK_KP_SPACE, KP_SPACE);
	MAP(SDLK_KP_AT, KP_AT);
	MAP(SDLK_KP_EXCLAM, KP_EXCLAM);
	MAP(SDLK_KP_MEMSTORE, KP_MEMSTORE);
	MAP(SDLK_KP_MEMRECALL, KP_MEMRECALL);
	MAP(SDLK_KP_MEMCLEAR, KP_MEMCLEAR);
	MAP(SDLK_KP_MEMADD, KP_MEMADD);
	MAP(SDLK_KP_MEMSUBTRACT, KP_MEMSUBTRACT);
	MAP(SDLK_KP_MEMMULTIPLY, KP_MEMMULTIPLY);
	MAP(SDLK_KP_MEMDIVIDE, KP_MEMDIVIDE);
	MAP(SDLK_KP_PLUSMINUS, KP_PLUSMINUS);
	MAP(SDLK_KP_CLEAR, KP_CLEAR);
	MAP(SDLK_KP_CLEARENTRY, KP_CLEARENTRY);
	MAP(SDLK_KP_BINARY, KP_BINARY);
	MAP(SDLK_KP_OCTAL, KP_OCTAL);
	MAP(SDLK_KP_DECIMAL, KP_DECIMAL);
	MAP(SDLK_KP_HEXADECIMAL, KP_HEXADECIMAL);
	MAP(SDLK_LCTRL, LCTRL);
	MAP(SDLK_LSHIFT, LSHIFT);
	MAP(SDLK_LALT, LALT);
	MAP(SDLK_LGUI, LGUI);
	MAP(SDLK_RCTRL, RCTRL);
	MAP(SDLK_RSHIFT, RSHIFT);
	MAP(SDLK_RALT, RALT);
	MAP(SDLK_RGUI, RGUI);
	MAP(SDLK_MODE, MODE);
	MAP(SDLK_AUDIONEXT, AUDIONEXT);
	MAP(SDLK_AUDIOPREV, AUDIOPREV);
	MAP(SDLK_AUDIOSTOP, AUDIOSTOP);
	MAP(SDLK_AUDIOPLAY, AUDIOPLAY);
	MAP(SDLK_AUDIOMUTE, AUDIOMUTE);
	MAP(SDLK_MEDIASELECT, MEDIASELECT);
	MAP(SDLK_WWW, WWW);
	MAP(SDLK_MAIL, MAIL);
	MAP(SDLK_CALCULATOR, CALCULATOR);
	MAP(SDLK_COMPUTER, COMPUTER);
	MAP(SDLK_AC_SEARCH, AC_SEARCH);
	MAP(SDLK_AC_HOME, AC_HOME);
	MAP(SDLK_AC_BACK, AC_BACK);
	MAP(SDLK_AC_FORWARD, AC_FORWARD);
	MAP(SDLK_AC_STOP, AC_STOP);
	MAP(SDLK_AC_REFRESH, AC_REFRESH);
	MAP(SDLK_AC_BOOKMARKS, AC_BOOKMARKS);
	MAP(SDLK_BRIGHTNESSDOWN, BRIGHTNESSDOWN);
	MAP(SDLK_BRIGHTNESSUP, BRIGHTNESSUP);
	MAP(SDLK_DISPLAYSWITCH, DISPLAYSWITCH);
	MAP(SDLK_KBDILLUMTOGGLE, KBDILLUMTOGGLE);
	MAP(SDLK_KBDILLUMDOWN, KBDILLUMDOWN);
	MAP(SDLK_KBDILLUMUP, KBDILLUMUP);
	MAP(SDLK_EJECT, EJECT);
	MAP(SDLK_SLEEP, SLEEP);

#undef MAP

	// Call once to clear first events
	return handleEvents();
}

void Input::destroy()
{
	if(m_impl != nullptr)
	{
		m_nativeWindow->_getAllocator().deleteInstance(m_impl);
		m_impl = nullptr;
	}
	m_nativeWindow = nullptr;
}

Error Input::handleEvents()
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
			akkey = m_impl->m_sdlToAnki[event.key.keysym.sym];
			m_keys[akkey] = 1;
			break;
		case SDL_KEYUP:
			akkey = m_impl->m_sdlToAnki[event.key.keysym.sym];
			m_keys[akkey] = 0;
			break;
		case SDL_MOUSEBUTTONDOWN:
		{
			MouseButton mb = sdlMouseButtonToAnKi(event.button.button);
			if(mb != MouseButton::COUNT)
			{
				m_mouseBtns[mb] = 1;
			}
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			MouseButton mb = sdlMouseButtonToAnKi(event.button.button);
			if(mb != MouseButton::COUNT)
			{
				m_mouseBtns[mb] = 0;
			}
			break;
		}
		case SDL_MOUSEWHEEL:
			m_mouseBtns[MouseButton::SCROLL_UP] = event.wheel.y > 0;
			m_mouseBtns[MouseButton::SCROLL_DOWN] = event.wheel.y < 0;
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

	return Error::NONE;
}

void Input::moveCursor(const Vec2& pos)
{
	if(pos != m_mousePosNdc)
	{
		const I32 x = I32(F32(m_nativeWindow->getWidth()) * (pos.x() * 0.5f + 0.5f));
		const I32 y = I32(F32(m_nativeWindow->getHeight()) * (-pos.y() * 0.5f + 0.5f));

		SDL_WarpMouseInWindow(m_nativeWindow->getNative().m_window, x, y);

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

} // end namespace anki
