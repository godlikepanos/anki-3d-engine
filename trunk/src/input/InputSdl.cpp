// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/input/Input.h"
#include "anki/input/InputSdl.h"
#include "anki/core/NativeWindowSdl.h"
#include "anki/core/Logger.h"
#include <SDL.h>

namespace anki {

//==============================================================================
void Input::init(NativeWindow* nativeWindow)
{
	ANKI_ASSERT(nativeWindow);
	m_nativeWindow = nativeWindow;

	// Init native
	m_impl = m_nativeWindow->_getAllocator().newInstance<InputImpl>();

	//impl
	m_impl->m_sdlToAnki[SDLK_RETURN] = KeyCode::RETURN;
	m_impl->m_sdlToAnki[SDLK_ESCAPE] = KeyCode::ESCAPE;
	m_impl->m_sdlToAnki[SDLK_BACKSPACE] = KeyCode::BACKSPACE;
	m_impl->m_sdlToAnki[SDLK_TAB] = KeyCode::TAB;
	m_impl->m_sdlToAnki[SDLK_SPACE] = KeyCode::SPACE;
	m_impl->m_sdlToAnki[SDLK_EXCLAIM] = KeyCode::EXCLAIM;
	m_impl->m_sdlToAnki[SDLK_QUOTEDBL] = KeyCode::QUOTEDBL;
	m_impl->m_sdlToAnki[SDLK_HASH] = KeyCode::HASH;
	m_impl->m_sdlToAnki[SDLK_PERCENT] = KeyCode::PERCENT;
	m_impl->m_sdlToAnki[SDLK_DOLLAR] = KeyCode::DOLLAR;
	m_impl->m_sdlToAnki[SDLK_AMPERSAND] = KeyCode::AMPERSAND;
	m_impl->m_sdlToAnki[SDLK_QUOTE] = KeyCode::QUOTE;
	m_impl->m_sdlToAnki[SDLK_LEFTPAREN] = KeyCode::LEFTPAREN;
	m_impl->m_sdlToAnki[SDLK_RIGHTPAREN] = KeyCode::RIGHTPAREN;
	m_impl->m_sdlToAnki[SDLK_ASTERISK] = KeyCode::ASTERISK;
	m_impl->m_sdlToAnki[SDLK_PLUS] = KeyCode::PLUS;
	m_impl->m_sdlToAnki[SDLK_COMMA] = KeyCode::COMMA;
	m_impl->m_sdlToAnki[SDLK_MINUS] = KeyCode::MINUS;
	m_impl->m_sdlToAnki[SDLK_PERIOD] = KeyCode::PERIOD;
	m_impl->m_sdlToAnki[SDLK_SLASH] = KeyCode::SLASH;
	m_impl->m_sdlToAnki[SDLK_0] = KeyCode::_0;
	m_impl->m_sdlToAnki[SDLK_1] = KeyCode::_1;
	m_impl->m_sdlToAnki[SDLK_2] = KeyCode::_2;
	m_impl->m_sdlToAnki[SDLK_3] = KeyCode::_3;
	m_impl->m_sdlToAnki[SDLK_4] = KeyCode::_4;
	m_impl->m_sdlToAnki[SDLK_5] = KeyCode::_5;
	m_impl->m_sdlToAnki[SDLK_6] = KeyCode::_6;
	m_impl->m_sdlToAnki[SDLK_7] = KeyCode::_7;
	m_impl->m_sdlToAnki[SDLK_8] = KeyCode::_8;
	m_impl->m_sdlToAnki[SDLK_9] = KeyCode::_9;
	m_impl->m_sdlToAnki[SDLK_COLON] = KeyCode::COLON;
	m_impl->m_sdlToAnki[SDLK_SEMICOLON] = KeyCode::SEMICOLON;
	m_impl->m_sdlToAnki[SDLK_LESS] = KeyCode::LESS;
	m_impl->m_sdlToAnki[SDLK_EQUALS] = KeyCode::EQUALS;
	m_impl->m_sdlToAnki[SDLK_GREATER] = KeyCode::GREATER;
	m_impl->m_sdlToAnki[SDLK_QUESTION] = KeyCode::QUESTION;
	m_impl->m_sdlToAnki[SDLK_AT] = KeyCode::AT;
	m_impl->m_sdlToAnki[SDLK_LEFTBRACKET] = KeyCode::LEFTBRACKET;
	m_impl->m_sdlToAnki[SDLK_BACKSLASH] = KeyCode::BACKSLASH;
	m_impl->m_sdlToAnki[SDLK_RIGHTBRACKET] = KeyCode::RIGHTBRACKET;
	m_impl->m_sdlToAnki[SDLK_CARET] = KeyCode::CARET;
	m_impl->m_sdlToAnki[SDLK_UNDERSCORE] = KeyCode::UNDERSCORE;
	m_impl->m_sdlToAnki[SDLK_BACKQUOTE] = KeyCode::BACKQUOTE;
	m_impl->m_sdlToAnki[SDLK_a] = KeyCode::A;
	m_impl->m_sdlToAnki[SDLK_b] = KeyCode::B;
	m_impl->m_sdlToAnki[SDLK_c] = KeyCode::C;
	m_impl->m_sdlToAnki[SDLK_d] = KeyCode::D;
	m_impl->m_sdlToAnki[SDLK_e] = KeyCode::E;
	m_impl->m_sdlToAnki[SDLK_f] = KeyCode::F;
	m_impl->m_sdlToAnki[SDLK_g] = KeyCode::G;
	m_impl->m_sdlToAnki[SDLK_h] = KeyCode::H;
	m_impl->m_sdlToAnki[SDLK_i] = KeyCode::I;
	m_impl->m_sdlToAnki[SDLK_j] = KeyCode::J;
	m_impl->m_sdlToAnki[SDLK_k] = KeyCode::K;
	m_impl->m_sdlToAnki[SDLK_l] = KeyCode::L;
	m_impl->m_sdlToAnki[SDLK_m] = KeyCode::M;
	m_impl->m_sdlToAnki[SDLK_n] = KeyCode::N;
	m_impl->m_sdlToAnki[SDLK_o] = KeyCode::O;
	m_impl->m_sdlToAnki[SDLK_p] = KeyCode::P;
	m_impl->m_sdlToAnki[SDLK_q] = KeyCode::Q;
	m_impl->m_sdlToAnki[SDLK_r] = KeyCode::R;
	m_impl->m_sdlToAnki[SDLK_s] = KeyCode::S;
	m_impl->m_sdlToAnki[SDLK_y] = KeyCode::T;
	m_impl->m_sdlToAnki[SDLK_u] = KeyCode::U;
	m_impl->m_sdlToAnki[SDLK_v] = KeyCode::V;
	m_impl->m_sdlToAnki[SDLK_w] = KeyCode::W;
	m_impl->m_sdlToAnki[SDLK_x] = KeyCode::X;
	m_impl->m_sdlToAnki[SDLK_y] = KeyCode::Y;
	m_impl->m_sdlToAnki[SDLK_z] = KeyCode::Z;
	m_impl->m_sdlToAnki[SDLK_CAPSLOCK] = KeyCode::CAPSLOCK;
	m_impl->m_sdlToAnki[SDLK_F1] = KeyCode::F1;
	m_impl->m_sdlToAnki[SDLK_F2] = KeyCode::F2;
	m_impl->m_sdlToAnki[SDLK_F3] = KeyCode::F3;
	m_impl->m_sdlToAnki[SDLK_F4] = KeyCode::F4;
	m_impl->m_sdlToAnki[SDLK_F5] = KeyCode::F5;
	m_impl->m_sdlToAnki[SDLK_F6] = KeyCode::F6;
	m_impl->m_sdlToAnki[SDLK_F7] = KeyCode::F7;
	m_impl->m_sdlToAnki[SDLK_F8] = KeyCode::F8;
	m_impl->m_sdlToAnki[SDLK_F9] = KeyCode::F9;
	m_impl->m_sdlToAnki[SDLK_F10] = KeyCode::F10;
	m_impl->m_sdlToAnki[SDLK_F11] = KeyCode::F11;
	m_impl->m_sdlToAnki[SDLK_F12] = KeyCode::F12;
	m_impl->m_sdlToAnki[SDLK_PRINTSCREEN] = KeyCode::PRINTSCREEN;
	m_impl->m_sdlToAnki[SDLK_SCROLLLOCK] = KeyCode::SCROLLLOCK;
	m_impl->m_sdlToAnki[SDLK_PAUSE] = KeyCode::PAUSE;
	m_impl->m_sdlToAnki[SDLK_INSERT] = KeyCode::INSERT;
	m_impl->m_sdlToAnki[SDLK_HOME] = KeyCode::HOME;
	m_impl->m_sdlToAnki[SDLK_PAGEUP] = KeyCode::PAGEUP;
	m_impl->m_sdlToAnki[SDLK_DELETE] = KeyCode::DELETE;
	m_impl->m_sdlToAnki[SDLK_END] = KeyCode::END;
	m_impl->m_sdlToAnki[SDLK_PAGEDOWN] = KeyCode::PAGEDOWN;
	m_impl->m_sdlToAnki[SDLK_RIGHT] = KeyCode::RIGHT;
	m_impl->m_sdlToAnki[SDLK_LEFT] = KeyCode::LEFT;
	m_impl->m_sdlToAnki[SDLK_DOWN] = KeyCode::DOWN;
	m_impl->m_sdlToAnki[SDLK_UP] = KeyCode::UP;
	m_impl->m_sdlToAnki[SDLK_NUMLOCKCLEAR] = KeyCode::NUMLOCKCLEAR;
	m_impl->m_sdlToAnki[SDLK_KP_DIVIDE] = KeyCode::KP_DIVIDE;
	m_impl->m_sdlToAnki[SDLK_KP_MULTIPLY] = KeyCode::KP_MULTIPLY;
	m_impl->m_sdlToAnki[SDLK_KP_MINUS] = KeyCode::KP_MINUS;
	m_impl->m_sdlToAnki[SDLK_KP_PLUS] = KeyCode::KP_PLUS;
	m_impl->m_sdlToAnki[SDLK_KP_ENTER] = KeyCode::KP_ENTER;
	m_impl->m_sdlToAnki[SDLK_KP_1] = KeyCode::KP_1;
	m_impl->m_sdlToAnki[SDLK_KP_2] = KeyCode::KP_2;
	m_impl->m_sdlToAnki[SDLK_KP_3] = KeyCode::KP_3;
	m_impl->m_sdlToAnki[SDLK_KP_4] = KeyCode::KP_4;
	m_impl->m_sdlToAnki[SDLK_KP_5] = KeyCode::KP_5;
	m_impl->m_sdlToAnki[SDLK_KP_6] = KeyCode::KP_6;
	m_impl->m_sdlToAnki[SDLK_KP_7] = KeyCode::KP_7;
	m_impl->m_sdlToAnki[SDLK_KP_8] = KeyCode::KP_8;
	m_impl->m_sdlToAnki[SDLK_KP_9] = KeyCode::KP_9;
	m_impl->m_sdlToAnki[SDLK_KP_0] = KeyCode::KP_0;
	m_impl->m_sdlToAnki[SDLK_KP_PERIOD] = KeyCode::KP_PERIOD;
	m_impl->m_sdlToAnki[SDLK_APPLICATION] = KeyCode::APPLICATION;
	m_impl->m_sdlToAnki[SDLK_POWER] = KeyCode::POWER;
	m_impl->m_sdlToAnki[SDLK_KP_EQUALS] = KeyCode::KP_EQUALS;
	m_impl->m_sdlToAnki[SDLK_F13] = KeyCode::F13;
	m_impl->m_sdlToAnki[SDLK_F14] = KeyCode::F14;
	m_impl->m_sdlToAnki[SDLK_F15] = KeyCode::F15;
	m_impl->m_sdlToAnki[SDLK_F16] = KeyCode::F16;
	m_impl->m_sdlToAnki[SDLK_F17] = KeyCode::F17;
	m_impl->m_sdlToAnki[SDLK_F18] = KeyCode::F18;
	m_impl->m_sdlToAnki[SDLK_F19] = KeyCode::F19;
	m_impl->m_sdlToAnki[SDLK_F20] = KeyCode::F20;
	m_impl->m_sdlToAnki[SDLK_F21] = KeyCode::F21;
	m_impl->m_sdlToAnki[SDLK_F22] = KeyCode::F22;
	m_impl->m_sdlToAnki[SDLK_F23] = KeyCode::F23;
	m_impl->m_sdlToAnki[SDLK_F24] = KeyCode::F24;
	m_impl->m_sdlToAnki[SDLK_EXECUTE] = KeyCode::EXECUTE;
	m_impl->m_sdlToAnki[SDLK_HELP] = KeyCode::HELP;
	m_impl->m_sdlToAnki[SDLK_MENU] = KeyCode::MENU;
	m_impl->m_sdlToAnki[SDLK_SELECT] = KeyCode::SELECT;
	m_impl->m_sdlToAnki[SDLK_STOP] = KeyCode::STOP;
	m_impl->m_sdlToAnki[SDLK_AGAIN] = KeyCode::AGAIN;
	m_impl->m_sdlToAnki[SDLK_UNDO] = KeyCode::UNDO;
	m_impl->m_sdlToAnki[SDLK_CUT] = KeyCode::CUT;
	m_impl->m_sdlToAnki[SDLK_COPY] = KeyCode::COPY;
	m_impl->m_sdlToAnki[SDLK_PASTE] = KeyCode::PASTE;
	m_impl->m_sdlToAnki[SDLK_FIND] = KeyCode::FIND;
	m_impl->m_sdlToAnki[SDLK_MUTE] = KeyCode::MUTE;
	m_impl->m_sdlToAnki[SDLK_VOLUMEUP] = KeyCode::VOLUMEUP;
	m_impl->m_sdlToAnki[SDLK_VOLUMEDOWN] = KeyCode::VOLUMEDOWN;
	m_impl->m_sdlToAnki[SDLK_KP_COMMA] = KeyCode::KP_COMMA;
	m_impl->m_sdlToAnki[SDLK_KP_EQUALSAS400] = KeyCode::KP_EQUALSAS400;
	m_impl->m_sdlToAnki[SDLK_ALTERASE] = KeyCode::ALTERASE;
	m_impl->m_sdlToAnki[SDLK_SYSREQ] = KeyCode::SYSREQ;
	m_impl->m_sdlToAnki[SDLK_CANCEL] = KeyCode::CANCEL;
	m_impl->m_sdlToAnki[SDLK_CLEAR] = KeyCode::CLEAR;
	m_impl->m_sdlToAnki[SDLK_PRIOR] = KeyCode::PRIOR;
	m_impl->m_sdlToAnki[SDLK_RETURN2] = KeyCode::RETURN2;
	m_impl->m_sdlToAnki[SDLK_SEPARATOR] = KeyCode::SEPARATOR;
	m_impl->m_sdlToAnki[SDLK_OUT] = KeyCode::OUT;
	m_impl->m_sdlToAnki[SDLK_OPER] = KeyCode::OPER;
	m_impl->m_sdlToAnki[SDLK_CLEARAGAIN] = KeyCode::CLEARAGAIN;
	m_impl->m_sdlToAnki[SDLK_CRSEL] = KeyCode::CRSEL;
	m_impl->m_sdlToAnki[SDLK_EXSEL] = KeyCode::EXSEL;
	m_impl->m_sdlToAnki[SDLK_KP_00] = KeyCode::KP_00;
	m_impl->m_sdlToAnki[SDLK_KP_000] = KeyCode::KP_000;
	m_impl->m_sdlToAnki[SDLK_THOUSANDSSEPARATOR] = KeyCode::THOUSANDSSEPARATOR;
	m_impl->m_sdlToAnki[SDLK_DECIMALSEPARATOR] = KeyCode::DECIMALSEPARATOR;
	m_impl->m_sdlToAnki[SDLK_CURRENCYUNIT] = KeyCode::CURRENCYUNIT;
	m_impl->m_sdlToAnki[SDLK_CURRENCYSUBUNIT] = KeyCode::CURRENCYSUBUNIT;
	m_impl->m_sdlToAnki[SDLK_KP_LEFTPAREN] = KeyCode::KP_LEFTPAREN;
	m_impl->m_sdlToAnki[SDLK_KP_RIGHTPAREN] = KeyCode::KP_RIGHTPAREN;
	m_impl->m_sdlToAnki[SDLK_KP_LEFTBRACE] = KeyCode::KP_LEFTBRACE;
	m_impl->m_sdlToAnki[SDLK_KP_RIGHTBRACE] = KeyCode::KP_RIGHTBRACE;
	m_impl->m_sdlToAnki[SDLK_KP_TAB] = KeyCode::KP_TAB;
	m_impl->m_sdlToAnki[SDLK_KP_BACKSPACE] = KeyCode::KP_BACKSPACE;
	m_impl->m_sdlToAnki[SDLK_KP_A] = KeyCode::KP_A;
	m_impl->m_sdlToAnki[SDLK_KP_B] = KeyCode::KP_B;
	m_impl->m_sdlToAnki[SDLK_KP_C] = KeyCode::KP_C;
	m_impl->m_sdlToAnki[SDLK_KP_D] = KeyCode::KP_D;
	m_impl->m_sdlToAnki[SDLK_KP_E] = KeyCode::KP_E;
	m_impl->m_sdlToAnki[SDLK_KP_F] = KeyCode::KP_F;
	m_impl->m_sdlToAnki[SDLK_KP_XOR] = KeyCode::KP_XOR;
	m_impl->m_sdlToAnki[SDLK_KP_POWER] = KeyCode::KP_POWER;
	m_impl->m_sdlToAnki[SDLK_KP_PERCENT] = KeyCode::KP_PERCENT;
	m_impl->m_sdlToAnki[SDLK_KP_LESS] = KeyCode::KP_LESS;
	m_impl->m_sdlToAnki[SDLK_KP_GREATER] = KeyCode::KP_GREATER;
	m_impl->m_sdlToAnki[SDLK_KP_AMPERSAND] = KeyCode::KP_AMPERSAND;
	m_impl->m_sdlToAnki[SDLK_KP_DBLAMPERSAND] = KeyCode::KP_DBLAMPERSAND;
	m_impl->m_sdlToAnki[SDLK_KP_VERTICALBAR] = KeyCode::KP_VERTICALBAR;
	m_impl->m_sdlToAnki[SDLK_KP_DBLVERTICALBAR] = KeyCode::KP_DBLVERTICALBAR;
	m_impl->m_sdlToAnki[SDLK_KP_COLON] = KeyCode::KP_COLON;
	m_impl->m_sdlToAnki[SDLK_KP_HASH] = KeyCode::KP_HASH;
	m_impl->m_sdlToAnki[SDLK_KP_SPACE] = KeyCode::KP_SPACE;
	m_impl->m_sdlToAnki[SDLK_KP_AT] = KeyCode::KP_AT;
	m_impl->m_sdlToAnki[SDLK_KP_EXCLAM] = KeyCode::KP_EXCLAM;
	m_impl->m_sdlToAnki[SDLK_KP_MEMSTORE] = KeyCode::KP_MEMSTORE;
	m_impl->m_sdlToAnki[SDLK_KP_MEMRECALL] = KeyCode::KP_MEMRECALL;
	m_impl->m_sdlToAnki[SDLK_KP_MEMCLEAR] = KeyCode::KP_MEMCLEAR;
	m_impl->m_sdlToAnki[SDLK_KP_MEMADD] = KeyCode::KP_MEMADD;
	m_impl->m_sdlToAnki[SDLK_KP_MEMSUBTRACT] = KeyCode::KP_MEMSUBTRACT;
	m_impl->m_sdlToAnki[SDLK_KP_MEMMULTIPLY] = KeyCode::KP_MEMMULTIPLY;
	m_impl->m_sdlToAnki[SDLK_KP_MEMDIVIDE] = KeyCode::KP_MEMDIVIDE;
	m_impl->m_sdlToAnki[SDLK_KP_PLUSMINUS] = KeyCode::KP_PLUSMINUS;
	m_impl->m_sdlToAnki[SDLK_KP_CLEAR] = KeyCode::KP_CLEAR;
	m_impl->m_sdlToAnki[SDLK_KP_CLEARENTRY] = KeyCode::KP_CLEARENTRY;
	m_impl->m_sdlToAnki[SDLK_KP_BINARY] = KeyCode::KP_BINARY;
	m_impl->m_sdlToAnki[SDLK_KP_OCTAL] = KeyCode::KP_OCTAL;
	m_impl->m_sdlToAnki[SDLK_KP_DECIMAL] = KeyCode::KP_DECIMAL;
	m_impl->m_sdlToAnki[SDLK_KP_HEXADECIMAL] = KeyCode::KP_HEXADECIMAL;
	m_impl->m_sdlToAnki[SDLK_LCTRL] = KeyCode::LCTRL;
	m_impl->m_sdlToAnki[SDLK_LSHIFT] = KeyCode::LSHIFT;
	m_impl->m_sdlToAnki[SDLK_LALT] = KeyCode::LALT;
	m_impl->m_sdlToAnki[SDLK_LGUI] = KeyCode::LGUI;
	m_impl->m_sdlToAnki[SDLK_RCTRL] = KeyCode::RCTRL;
	m_impl->m_sdlToAnki[SDLK_RSHIFT] = KeyCode::RSHIFT;
	m_impl->m_sdlToAnki[SDLK_RALT] = KeyCode::RALT;
	m_impl->m_sdlToAnki[SDLK_RGUI] = KeyCode::RGUI;
	m_impl->m_sdlToAnki[SDLK_MODE] = KeyCode::MODE;
	m_impl->m_sdlToAnki[SDLK_AUDIONEXT] = KeyCode::AUDIONEXT;
	m_impl->m_sdlToAnki[SDLK_AUDIOPREV] = KeyCode::AUDIOPREV;
	m_impl->m_sdlToAnki[SDLK_AUDIOSTOP] = KeyCode::AUDIOSTOP;
	m_impl->m_sdlToAnki[SDLK_AUDIOPLAY] = KeyCode::AUDIOPLAY;
	m_impl->m_sdlToAnki[SDLK_AUDIOMUTE] = KeyCode::AUDIOMUTE;
	m_impl->m_sdlToAnki[SDLK_MEDIASELECT] = KeyCode::MEDIASELECT;
	m_impl->m_sdlToAnki[SDLK_WWW] = KeyCode::WWW;
	m_impl->m_sdlToAnki[SDLK_MAIL] = KeyCode::MAIL;
	m_impl->m_sdlToAnki[SDLK_CALCULATOR] = KeyCode::CALCULATOR;
	m_impl->m_sdlToAnki[SDLK_COMPUTER] = KeyCode::COMPUTER;
	m_impl->m_sdlToAnki[SDLK_AC_SEARCH] = KeyCode::AC_SEARCH;
	m_impl->m_sdlToAnki[SDLK_AC_HOME] = KeyCode::AC_HOME;
	m_impl->m_sdlToAnki[SDLK_AC_BACK] = KeyCode::AC_BACK;
	m_impl->m_sdlToAnki[SDLK_AC_FORWARD] = KeyCode::AC_FORWARD;
	m_impl->m_sdlToAnki[SDLK_AC_STOP] = KeyCode::AC_STOP;
	m_impl->m_sdlToAnki[SDLK_AC_REFRESH] = KeyCode::AC_REFRESH;
	m_impl->m_sdlToAnki[SDLK_AC_BOOKMARKS] = KeyCode::AC_BOOKMARKS;
	m_impl->m_sdlToAnki[SDLK_BRIGHTNESSDOWN] = KeyCode::BRIGHTNESSDOWN;
	m_impl->m_sdlToAnki[SDLK_BRIGHTNESSUP] = KeyCode::BRIGHTNESSUP;
	m_impl->m_sdlToAnki[SDLK_DISPLAYSWITCH] = KeyCode::DISPLAYSWITCH;
	m_impl->m_sdlToAnki[SDLK_KBDILLUMTOGGLE] = KeyCode::KBDILLUMTOGGLE;
	m_impl->m_sdlToAnki[SDLK_KBDILLUMDOWN] = KeyCode::KBDILLUMDOWN;
	m_impl->m_sdlToAnki[SDLK_KBDILLUMUP] = KeyCode::KBDILLUMUP;
	m_impl->m_sdlToAnki[SDLK_EJECT] = KeyCode::EJECT;
	m_impl->m_sdlToAnki[SDLK_SLEEP] = KeyCode::SLEEP;

	// Call once to clear first events
	handleEvents();
}

//==============================================================================
void Input::destroy()
{
	if(m_impl != nullptr)
	{
		m_nativeWindow->_getAllocator().deleteInstance(m_impl);
	}
}

//==============================================================================
void Input::handleEvents()
{
	ANKI_ASSERT(m_nativeWindow != nullptr);

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
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_KEYDOWN:
			akkey = m_impl->m_sdlToAnki[event.key.keysym.sym];
			m_keys[static_cast<U>(akkey)] = 1;
			break;
		case SDL_KEYUP:
			akkey = m_impl->m_sdlToAnki[event.key.keysym.sym];
			m_keys[static_cast<U>(akkey)] = 0;
			break;
		case SDL_MOUSEBUTTONDOWN:
			//XXX
			break;
		case SDL_MOUSEBUTTONUP:
			//XXX
			break;
		case SDL_MOUSEMOTION:
			m_mousePosNdc.x() = 
				(F32)event.button.x / m_nativeWindow->getWidth() * 2.0 - 1.0;
			m_mousePosNdc.y() = 
				-((F32)event.button.y 
				/ m_nativeWindow->getHeight() * 2.0 - 1.0);
			break;
		case SDL_QUIT:
			addEvent(WINDOW_CLOSED_EVENT);
			break;
		}
	} // end while events

	// Lock mouse
	if(m_lockCurs)
	{
		moveCursor(Vec2(0.0));
	}
}

//==============================================================================
void Input::moveCursor(const Vec2& pos)
{
	if(pos != m_mousePosNdc)
	{
		SDL_WarpMouseInWindow(
			m_nativeWindow->getNative().m_window,
			m_nativeWindow->getWidth() * (pos.x() * 0.5 + 0.5), 
			m_nativeWindow->getHeight() * (pos.y() * 0.5 + 0.5));
	}
}

//==============================================================================
void Input::hideCursor(Bool hide)
{
	SDL_ShowCursor(!hide);
}

} // end namespace anki
