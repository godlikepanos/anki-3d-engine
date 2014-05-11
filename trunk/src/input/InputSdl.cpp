#include "anki/input/Input.h"
#include "anki/input/InputSdl.h"
#include "anki/core/NativeWindowSdl.h"
#include "anki/core/Logger.h"
#include <SDL.h>

namespace anki {

//==============================================================================
Input::~Input()
{}

//==============================================================================
void Input::init(NativeWindow* nativeWindow)
{
	ANKI_ASSERT(nativeWindow);
	m_nativeWindow = nativeWindow;

	// Init native
	m_impl.reset(new InputImpl);

	//impl
	m_impl->m_sdlKeyToAnki[SDLK_RETURN] = KC_RETURN;
	m_impl->m_sdlKeyToAnki[SDLK_ESCAPE] = KC_ESCAPE;
	m_impl->m_sdlKeyToAnki[SDLK_BACKSPACE] = KC_BACKSPACE;
	m_impl->m_sdlKeyToAnki[SDLK_TAB] = KC_TAB;
	m_impl->m_sdlKeyToAnki[SDLK_SPACE] = KC_SPACE;
	m_impl->m_sdlKeyToAnki[SDLK_EXCLAIM] = KC_EXCLAIM;
	m_impl->m_sdlKeyToAnki[SDLK_QUOTEDBL] = KC_QUOTEDBL;
	m_impl->m_sdlKeyToAnki[SDLK_HASH] = KC_HASH;
	m_impl->m_sdlKeyToAnki[SDLK_PERCENT] = KC_PERCENT;
	m_impl->m_sdlKeyToAnki[SDLK_DOLLAR] = KC_DOLLAR;
	m_impl->m_sdlKeyToAnki[SDLK_AMPERSAND] = KC_AMPERSAND;
	m_impl->m_sdlKeyToAnki[SDLK_QUOTE] = KC_QUOTE;
	m_impl->m_sdlKeyToAnki[SDLK_LEFTPAREN] = KC_LEFTPAREN;
	m_impl->m_sdlKeyToAnki[SDLK_RIGHTPAREN] = KC_RIGHTPAREN;
	m_impl->m_sdlKeyToAnki[SDLK_ASTERISK] = KC_ASTERISK;
	m_impl->m_sdlKeyToAnki[SDLK_PLUS] = KC_PLUS;
	m_impl->m_sdlKeyToAnki[SDLK_COMMA] = KC_COMMA;
	m_impl->m_sdlKeyToAnki[SDLK_MINUS] = KC_MINUS;
	m_impl->m_sdlKeyToAnki[SDLK_PERIOD] = KC_PERIOD;
	m_impl->m_sdlKeyToAnki[SDLK_SLASH] = KC_SLASH;
	m_impl->m_sdlKeyToAnki[SDLK_0] = KC_0;
	m_impl->m_sdlKeyToAnki[SDLK_1] = KC_1;
	m_impl->m_sdlKeyToAnki[SDLK_2] = KC_2;
	m_impl->m_sdlKeyToAnki[SDLK_3] = KC_3;
	m_impl->m_sdlKeyToAnki[SDLK_4] = KC_4;
	m_impl->m_sdlKeyToAnki[SDLK_5] = KC_5;
	m_impl->m_sdlKeyToAnki[SDLK_6] = KC_6;
	m_impl->m_sdlKeyToAnki[SDLK_7] = KC_7;
	m_impl->m_sdlKeyToAnki[SDLK_8] = KC_8;
	m_impl->m_sdlKeyToAnki[SDLK_9] = KC_9;
	m_impl->m_sdlKeyToAnki[SDLK_COLON] = KC_COLON;
	m_impl->m_sdlKeyToAnki[SDLK_SEMICOLON] = KC_SEMICOLON;
	m_impl->m_sdlKeyToAnki[SDLK_LESS] = KC_LESS;
	m_impl->m_sdlKeyToAnki[SDLK_EQUALS] = KC_EQUALS;
	m_impl->m_sdlKeyToAnki[SDLK_GREATER] = KC_GREATER;
	m_impl->m_sdlKeyToAnki[SDLK_QUESTION] = KC_QUESTION;
	m_impl->m_sdlKeyToAnki[SDLK_AT] = KC_AT;
	m_impl->m_sdlKeyToAnki[SDLK_LEFTBRACKET] = KC_LEFTBRACKET;
	m_impl->m_sdlKeyToAnki[SDLK_BACKSLASH] = KC_BACKSLASH;
	m_impl->m_sdlKeyToAnki[SDLK_RIGHTBRACKET] = KC_RIGHTBRACKET;
	m_impl->m_sdlKeyToAnki[SDLK_CARET] = KC_CARET;
	m_impl->m_sdlKeyToAnki[SDLK_UNDERSCORE] = KC_UNDERSCORE;
	m_impl->m_sdlKeyToAnki[SDLK_BACKQUOTE] = KC_BACKQUOTE;
	m_impl->m_sdlKeyToAnki[SDLK_a] = KC_A;
	m_impl->m_sdlKeyToAnki[SDLK_b] = KC_B;
	m_impl->m_sdlKeyToAnki[SDLK_c] = KC_C;
	m_impl->m_sdlKeyToAnki[SDLK_d] = KC_D;
	m_impl->m_sdlKeyToAnki[SDLK_e] = KC_E;
	m_impl->m_sdlKeyToAnki[SDLK_f] = KC_F;
	m_impl->m_sdlKeyToAnki[SDLK_g] = KC_G;
	m_impl->m_sdlKeyToAnki[SDLK_h] = KC_H;
	m_impl->m_sdlKeyToAnki[SDLK_i] = KC_I;
	m_impl->m_sdlKeyToAnki[SDLK_j] = KC_J;
	m_impl->m_sdlKeyToAnki[SDLK_k] = KC_K;
	m_impl->m_sdlKeyToAnki[SDLK_l] = KC_L;
	m_impl->m_sdlKeyToAnki[SDLK_m] = KC_M;
	m_impl->m_sdlKeyToAnki[SDLK_n] = KC_N;
	m_impl->m_sdlKeyToAnki[SDLK_o] = KC_O;
	m_impl->m_sdlKeyToAnki[SDLK_p] = KC_P;
	m_impl->m_sdlKeyToAnki[SDLK_q] = KC_Q;
	m_impl->m_sdlKeyToAnki[SDLK_r] = KC_R;
	m_impl->m_sdlKeyToAnki[SDLK_s] = KC_S;
	m_impl->m_sdlKeyToAnki[SDLK_y] = KC_T;
	m_impl->m_sdlKeyToAnki[SDLK_u] = KC_U;
	m_impl->m_sdlKeyToAnki[SDLK_v] = KC_V;
	m_impl->m_sdlKeyToAnki[SDLK_w] = KC_W;
	m_impl->m_sdlKeyToAnki[SDLK_x] = KC_X;
	m_impl->m_sdlKeyToAnki[SDLK_y] = KC_Y;
	m_impl->m_sdlKeyToAnki[SDLK_z] = KC_Z;
	m_impl->m_sdlKeyToAnki[SDLK_CAPSLOCK] = KC_CAPSLOCK;
	m_impl->m_sdlKeyToAnki[SDLK_F1] = KC_F1;
	m_impl->m_sdlKeyToAnki[SDLK_F2] = KC_F2;
	m_impl->m_sdlKeyToAnki[SDLK_F3] = KC_F3;
	m_impl->m_sdlKeyToAnki[SDLK_F4] = KC_F4;
	m_impl->m_sdlKeyToAnki[SDLK_F5] = KC_F5;
	m_impl->m_sdlKeyToAnki[SDLK_F6] = KC_F6;
	m_impl->m_sdlKeyToAnki[SDLK_F7] = KC_F7;
	m_impl->m_sdlKeyToAnki[SDLK_F8] = KC_F8;
	m_impl->m_sdlKeyToAnki[SDLK_F9] = KC_F9;
	m_impl->m_sdlKeyToAnki[SDLK_F10] = KC_F10;
	m_impl->m_sdlKeyToAnki[SDLK_F11] = KC_F11;
	m_impl->m_sdlKeyToAnki[SDLK_F12] = KC_F12;
	m_impl->m_sdlKeyToAnki[SDLK_PRINTSCREEN] = KC_PRINTSCREEN;
	m_impl->m_sdlKeyToAnki[SDLK_SCROLLLOCK] = KC_SCROLLLOCK;
	m_impl->m_sdlKeyToAnki[SDLK_PAUSE] = KC_PAUSE;
	m_impl->m_sdlKeyToAnki[SDLK_INSERT] = KC_INSERT;
	m_impl->m_sdlKeyToAnki[SDLK_HOME] = KC_HOME;
	m_impl->m_sdlKeyToAnki[SDLK_PAGEUP] = KC_PAGEUP;
	m_impl->m_sdlKeyToAnki[SDLK_DELETE] = KC_DELETE;
	m_impl->m_sdlKeyToAnki[SDLK_END] = KC_END;
	m_impl->m_sdlKeyToAnki[SDLK_PAGEDOWN] = KC_PAGEDOWN;
	m_impl->m_sdlKeyToAnki[SDLK_RIGHT] = KC_RIGHT;
	m_impl->m_sdlKeyToAnki[SDLK_LEFT] = KC_LEFT;
	m_impl->m_sdlKeyToAnki[SDLK_DOWN] = KC_DOWN;
	m_impl->m_sdlKeyToAnki[SDLK_UP] = KC_UP;
	m_impl->m_sdlKeyToAnki[SDLK_NUMLOCKCLEAR] = KC_NUMLOCKCLEAR;
	m_impl->m_sdlKeyToAnki[SDLK_KP_DIVIDE] = KC_KP_DIVIDE;
	m_impl->m_sdlKeyToAnki[SDLK_KP_MULTIPLY] = KC_KP_MULTIPLY;
	m_impl->m_sdlKeyToAnki[SDLK_KP_MINUS] = KC_KP_MINUS;
	m_impl->m_sdlKeyToAnki[SDLK_KP_PLUS] = KC_KP_PLUS;
	m_impl->m_sdlKeyToAnki[SDLK_KP_ENTER] = KC_KP_ENTER;
	m_impl->m_sdlKeyToAnki[SDLK_KP_1] = KC_KP_1;
	m_impl->m_sdlKeyToAnki[SDLK_KP_2] = KC_KP_2;
	m_impl->m_sdlKeyToAnki[SDLK_KP_3] = KC_KP_3;
	m_impl->m_sdlKeyToAnki[SDLK_KP_4] = KC_KP_4;
	m_impl->m_sdlKeyToAnki[SDLK_KP_5] = KC_KP_5;
	m_impl->m_sdlKeyToAnki[SDLK_KP_6] = KC_KP_6;
	m_impl->m_sdlKeyToAnki[SDLK_KP_7] = KC_KP_7;
	m_impl->m_sdlKeyToAnki[SDLK_KP_8] = KC_KP_8;
	m_impl->m_sdlKeyToAnki[SDLK_KP_9] = KC_KP_9;
	m_impl->m_sdlKeyToAnki[SDLK_KP_0] = KC_KP_0;
	m_impl->m_sdlKeyToAnki[SDLK_KP_PERIOD] = KC_KP_PERIOD;
	m_impl->m_sdlKeyToAnki[SDLK_APPLICATION] = KC_APPLICATION;
	m_impl->m_sdlKeyToAnki[SDLK_POWER] = KC_POWER;
	m_impl->m_sdlKeyToAnki[SDLK_KP_EQUALS] = KC_KP_EQUALS;
	m_impl->m_sdlKeyToAnki[SDLK_F13] = KC_F13;
	m_impl->m_sdlKeyToAnki[SDLK_F14] = KC_F14;
	m_impl->m_sdlKeyToAnki[SDLK_F15] = KC_F15;
	m_impl->m_sdlKeyToAnki[SDLK_F16] = KC_F16;
	m_impl->m_sdlKeyToAnki[SDLK_F17] = KC_F17;
	m_impl->m_sdlKeyToAnki[SDLK_F18] = KC_F18;
	m_impl->m_sdlKeyToAnki[SDLK_F19] = KC_F19;
	m_impl->m_sdlKeyToAnki[SDLK_F20] = KC_F20;
	m_impl->m_sdlKeyToAnki[SDLK_F21] = KC_F21;
	m_impl->m_sdlKeyToAnki[SDLK_F22] = KC_F22;
	m_impl->m_sdlKeyToAnki[SDLK_F23] = KC_F23;
	m_impl->m_sdlKeyToAnki[SDLK_F24] = KC_F24;
	m_impl->m_sdlKeyToAnki[SDLK_EXECUTE] = KC_EXECUTE;
	m_impl->m_sdlKeyToAnki[SDLK_HELP] = KC_HELP;
	m_impl->m_sdlKeyToAnki[SDLK_MENU] = KC_MENU;
	m_impl->m_sdlKeyToAnki[SDLK_SELECT] = KC_SELECT;
	m_impl->m_sdlKeyToAnki[SDLK_STOP] = KC_STOP;
	m_impl->m_sdlKeyToAnki[SDLK_AGAIN] = KC_AGAIN;
	m_impl->m_sdlKeyToAnki[SDLK_UNDO] = KC_UNDO;
	m_impl->m_sdlKeyToAnki[SDLK_CUT] = KC_CUT;
	m_impl->m_sdlKeyToAnki[SDLK_COPY] = KC_COPY;
	m_impl->m_sdlKeyToAnki[SDLK_PASTE] = KC_PASTE;
	m_impl->m_sdlKeyToAnki[SDLK_FIND] = KC_FIND;
	m_impl->m_sdlKeyToAnki[SDLK_MUTE] = KC_MUTE;
	m_impl->m_sdlKeyToAnki[SDLK_VOLUMEUP] = KC_VOLUMEUP;
	m_impl->m_sdlKeyToAnki[SDLK_VOLUMEDOWN] = KC_VOLUMEDOWN;
	m_impl->m_sdlKeyToAnki[SDLK_KP_COMMA] = KC_KP_COMMA;
	m_impl->m_sdlKeyToAnki[SDLK_KP_EQUALSAS400] = KC_KP_EQUALSAS400;
	m_impl->m_sdlKeyToAnki[SDLK_ALTERASE] = KC_ALTERASE;
	m_impl->m_sdlKeyToAnki[SDLK_SYSREQ] = KC_SYSREQ;
	m_impl->m_sdlKeyToAnki[SDLK_CANCEL] = KC_CANCEL;
	m_impl->m_sdlKeyToAnki[SDLK_CLEAR] = KC_CLEAR;
	m_impl->m_sdlKeyToAnki[SDLK_PRIOR] = KC_PRIOR;
	m_impl->m_sdlKeyToAnki[SDLK_RETURN2] = KC_RETURN2;
	m_impl->m_sdlKeyToAnki[SDLK_SEPARATOR] = KC_SEPARATOR;
	m_impl->m_sdlKeyToAnki[SDLK_OUT] = KC_OUT;
	m_impl->m_sdlKeyToAnki[SDLK_OPER] = KC_OPER;
	m_impl->m_sdlKeyToAnki[SDLK_CLEARAGAIN] = KC_CLEARAGAIN;
	m_impl->m_sdlKeyToAnki[SDLK_CRSEL] = KC_CRSEL;
	m_impl->m_sdlKeyToAnki[SDLK_EXSEL] = KC_EXSEL;
	m_impl->m_sdlKeyToAnki[SDLK_KP_00] = KC_KP_00;
	m_impl->m_sdlKeyToAnki[SDLK_KP_000] = KC_KP_000;
	m_impl->m_sdlKeyToAnki[SDLK_THOUSANDSSEPARATOR] = KC_THOUSANDSSEPARATOR;
	m_impl->m_sdlKeyToAnki[SDLK_DECIMALSEPARATOR] = KC_DECIMALSEPARATOR;
	m_impl->m_sdlKeyToAnki[SDLK_CURRENCYUNIT] = KC_CURRENCYUNIT;
	m_impl->m_sdlKeyToAnki[SDLK_CURRENCYSUBUNIT] = KC_CURRENCYSUBUNIT;
	m_impl->m_sdlKeyToAnki[SDLK_KP_LEFTPAREN] = KC_KP_LEFTPAREN;
	m_impl->m_sdlKeyToAnki[SDLK_KP_RIGHTPAREN] = KC_KP_RIGHTPAREN;
	m_impl->m_sdlKeyToAnki[SDLK_KP_LEFTBRACE] = KC_KP_LEFTBRACE;
	m_impl->m_sdlKeyToAnki[SDLK_KP_RIGHTBRACE] = KC_KP_RIGHTBRACE;
	m_impl->m_sdlKeyToAnki[SDLK_KP_TAB] = KC_KP_TAB;
	m_impl->m_sdlKeyToAnki[SDLK_KP_BACKSPACE] = KC_KP_BACKSPACE;
	m_impl->m_sdlKeyToAnki[SDLK_KP_A] = KC_KP_A;
	m_impl->m_sdlKeyToAnki[SDLK_KP_B] = KC_KP_B;
	m_impl->m_sdlKeyToAnki[SDLK_KP_C] = KC_KP_C;
	m_impl->m_sdlKeyToAnki[SDLK_KP_D] = KC_KP_D;
	m_impl->m_sdlKeyToAnki[SDLK_KP_E] = KC_KP_E;
	m_impl->m_sdlKeyToAnki[SDLK_KP_F] = KC_KP_F;
	m_impl->m_sdlKeyToAnki[SDLK_KP_XOR] = KC_KP_XOR;
	m_impl->m_sdlKeyToAnki[SDLK_KP_POWER] = KC_KP_POWER;
	m_impl->m_sdlKeyToAnki[SDLK_KP_PERCENT] = KC_KP_PERCENT;
	m_impl->m_sdlKeyToAnki[SDLK_KP_LESS] = KC_KP_LESS;
	m_impl->m_sdlKeyToAnki[SDLK_KP_GREATER] = KC_KP_GREATER;
	m_impl->m_sdlKeyToAnki[SDLK_KP_AMPERSAND] = KC_KP_AMPERSAND;
	m_impl->m_sdlKeyToAnki[SDLK_KP_DBLAMPERSAND] = KC_KP_DBLAMPERSAND;
	m_impl->m_sdlKeyToAnki[SDLK_KP_VERTICALBAR] = KC_KP_VERTICALBAR;
	m_impl->m_sdlKeyToAnki[SDLK_KP_DBLVERTICALBAR] = KC_KP_DBLVERTICALBAR;
	m_impl->m_sdlKeyToAnki[SDLK_KP_COLON] = KC_KP_COLON;
	m_impl->m_sdlKeyToAnki[SDLK_KP_HASH] = KC_KP_HASH;
	m_impl->m_sdlKeyToAnki[SDLK_KP_SPACE] = KC_KP_SPACE;
	m_impl->m_sdlKeyToAnki[SDLK_KP_AT] = KC_KP_AT;
	m_impl->m_sdlKeyToAnki[SDLK_KP_EXCLAM] = KC_KP_EXCLAM;
	m_impl->m_sdlKeyToAnki[SDLK_KP_MEMSTORE] = KC_KP_MEMSTORE;
	m_impl->m_sdlKeyToAnki[SDLK_KP_MEMRECALL] = KC_KP_MEMRECALL;
	m_impl->m_sdlKeyToAnki[SDLK_KP_MEMCLEAR] = KC_KP_MEMCLEAR;
	m_impl->m_sdlKeyToAnki[SDLK_KP_MEMADD] = KC_KP_MEMADD;
	m_impl->m_sdlKeyToAnki[SDLK_KP_MEMSUBTRACT] = KC_KP_MEMSUBTRACT;
	m_impl->m_sdlKeyToAnki[SDLK_KP_MEMMULTIPLY] = KC_KP_MEMMULTIPLY;
	m_impl->m_sdlKeyToAnki[SDLK_KP_MEMDIVIDE] = KC_KP_MEMDIVIDE;
	m_impl->m_sdlKeyToAnki[SDLK_KP_PLUSMINUS] = KC_KP_PLUSMINUS;
	m_impl->m_sdlKeyToAnki[SDLK_KP_CLEAR] = KC_KP_CLEAR;
	m_impl->m_sdlKeyToAnki[SDLK_KP_CLEARENTRY] = KC_KP_CLEARENTRY;
	m_impl->m_sdlKeyToAnki[SDLK_KP_BINARY] = KC_KP_BINARY;
	m_impl->m_sdlKeyToAnki[SDLK_KP_OCTAL] = KC_KP_OCTAL;
	m_impl->m_sdlKeyToAnki[SDLK_KP_DECIMAL] = KC_KP_DECIMAL;
	m_impl->m_sdlKeyToAnki[SDLK_KP_HEXADECIMAL] = KC_KP_HEXADECIMAL;
	m_impl->m_sdlKeyToAnki[SDLK_LCTRL] = KC_LCTRL;
	m_impl->m_sdlKeyToAnki[SDLK_LSHIFT] = KC_LSHIFT;
	m_impl->m_sdlKeyToAnki[SDLK_LALT] = KC_LALT;
	m_impl->m_sdlKeyToAnki[SDLK_LGUI] = KC_LGUI;
	m_impl->m_sdlKeyToAnki[SDLK_RCTRL] = KC_RCTRL;
	m_impl->m_sdlKeyToAnki[SDLK_RSHIFT] = KC_RSHIFT;
	m_impl->m_sdlKeyToAnki[SDLK_RALT] = KC_RALT;
	m_impl->m_sdlKeyToAnki[SDLK_RGUI] = KC_RGUI;
	m_impl->m_sdlKeyToAnki[SDLK_MODE] = KC_MODE;
	m_impl->m_sdlKeyToAnki[SDLK_AUDIONEXT] = KC_AUDIONEXT;
	m_impl->m_sdlKeyToAnki[SDLK_AUDIOPREV] = KC_AUDIOPREV;
	m_impl->m_sdlKeyToAnki[SDLK_AUDIOSTOP] = KC_AUDIOSTOP;
	m_impl->m_sdlKeyToAnki[SDLK_AUDIOPLAY] = KC_AUDIOPLAY;
	m_impl->m_sdlKeyToAnki[SDLK_AUDIOMUTE] = KC_AUDIOMUTE;
	m_impl->m_sdlKeyToAnki[SDLK_MEDIASELECT] = KC_MEDIASELECT;
	m_impl->m_sdlKeyToAnki[SDLK_WWW] = KC_WWW;
	m_impl->m_sdlKeyToAnki[SDLK_MAIL] = KC_MAIL;
	m_impl->m_sdlKeyToAnki[SDLK_CALCULATOR] = KC_CALCULATOR;
	m_impl->m_sdlKeyToAnki[SDLK_COMPUTER] = KC_COMPUTER;
	m_impl->m_sdlKeyToAnki[SDLK_AC_SEARCH] = KC_AC_SEARCH;
	m_impl->m_sdlKeyToAnki[SDLK_AC_HOME] = KC_AC_HOME;
	m_impl->m_sdlKeyToAnki[SDLK_AC_BACK] = KC_AC_BACK;
	m_impl->m_sdlKeyToAnki[SDLK_AC_FORWARD] = KC_AC_FORWARD;
	m_impl->m_sdlKeyToAnki[SDLK_AC_STOP] = KC_AC_STOP;
	m_impl->m_sdlKeyToAnki[SDLK_AC_REFRESH] = KC_AC_REFRESH;
	m_impl->m_sdlKeyToAnki[SDLK_AC_BOOKMARKS] = KC_AC_BOOKMARKS;
	m_impl->m_sdlKeyToAnki[SDLK_BRIGHTNESSDOWN] = KC_BRIGHTNESSDOWN;
	m_impl->m_sdlKeyToAnki[SDLK_BRIGHTNESSUP] = KC_BRIGHTNESSUP;
	m_impl->m_sdlKeyToAnki[SDLK_DISPLAYSWITCH] = KC_DISPLAYSWITCH;
	m_impl->m_sdlKeyToAnki[SDLK_KBDILLUMTOGGLE] = KC_KBDILLUMTOGGLE;
	m_impl->m_sdlKeyToAnki[SDLK_KBDILLUMDOWN] = KC_KBDILLUMDOWN;
	m_impl->m_sdlKeyToAnki[SDLK_KBDILLUMUP] = KC_KBDILLUMUP;
	m_impl->m_sdlKeyToAnki[SDLK_EJECT] = KC_EJECT;
	m_impl->m_sdlKeyToAnki[SDLK_SLEEP] = KC_SLEEP;

	// Call once to clear first events
	handleEvents();
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
			akkey = m_impl->m_sdlKeyToAnki[event.key.keysym.sym];
			m_keys[akkey] = 1;
			break;
		case SDL_KEYUP:
			akkey = m_impl->m_sdlKeyToAnki[event.key.keysym.sym];
			m_keys[akkey] = 0;
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
