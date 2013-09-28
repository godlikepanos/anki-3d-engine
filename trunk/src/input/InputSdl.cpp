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
void Input::init(NativeWindow* nativeWindow_)
{
	ANKI_ASSERT(nativeWindow_);
	nativeWindow = nativeWindow_;

	// Init native
	impl.reset(new InputImpl);

	//impl
	impl->sdlKeyToAnki[SDLK_RETURN] = KC_RETURN;
	impl->sdlKeyToAnki[SDLK_ESCAPE] = KC_ESCAPE;
	impl->sdlKeyToAnki[SDLK_BACKSPACE] = KC_BACKSPACE;
	impl->sdlKeyToAnki[SDLK_TAB] = KC_TAB;
	impl->sdlKeyToAnki[SDLK_SPACE] = KC_SPACE;
	impl->sdlKeyToAnki[SDLK_EXCLAIM] = KC_EXCLAIM;
	impl->sdlKeyToAnki[SDLK_QUOTEDBL] = KC_QUOTEDBL;
	impl->sdlKeyToAnki[SDLK_HASH] = KC_HASH;
	impl->sdlKeyToAnki[SDLK_PERCENT] = KC_PERCENT;
	impl->sdlKeyToAnki[SDLK_DOLLAR] = KC_DOLLAR;
	impl->sdlKeyToAnki[SDLK_AMPERSAND] = KC_AMPERSAND;
	impl->sdlKeyToAnki[SDLK_QUOTE] = KC_QUOTE;
	impl->sdlKeyToAnki[SDLK_LEFTPAREN] = KC_LEFTPAREN;
	impl->sdlKeyToAnki[SDLK_RIGHTPAREN] = KC_RIGHTPAREN;
	impl->sdlKeyToAnki[SDLK_ASTERISK] = KC_ASTERISK;
	impl->sdlKeyToAnki[SDLK_PLUS] = KC_PLUS;
	impl->sdlKeyToAnki[SDLK_COMMA] = KC_COMMA;
	impl->sdlKeyToAnki[SDLK_MINUS] = KC_MINUS;
	impl->sdlKeyToAnki[SDLK_PERIOD] = KC_PERIOD;
	impl->sdlKeyToAnki[SDLK_SLASH] = KC_SLASH;
	impl->sdlKeyToAnki[SDLK_0] = KC_0;
	impl->sdlKeyToAnki[SDLK_1] = KC_1;
	impl->sdlKeyToAnki[SDLK_2] = KC_2;
	impl->sdlKeyToAnki[SDLK_3] = KC_3;
	impl->sdlKeyToAnki[SDLK_4] = KC_4;
	impl->sdlKeyToAnki[SDLK_5] = KC_5;
	impl->sdlKeyToAnki[SDLK_6] = KC_6;
	impl->sdlKeyToAnki[SDLK_7] = KC_7;
	impl->sdlKeyToAnki[SDLK_8] = KC_8;
	impl->sdlKeyToAnki[SDLK_9] = KC_9;
	impl->sdlKeyToAnki[SDLK_COLON] = KC_COLON;
	impl->sdlKeyToAnki[SDLK_SEMICOLON] = KC_SEMICOLON;
	impl->sdlKeyToAnki[SDLK_LESS] = KC_LESS;
	impl->sdlKeyToAnki[SDLK_EQUALS] = KC_EQUALS;
	impl->sdlKeyToAnki[SDLK_GREATER] = KC_GREATER;
	impl->sdlKeyToAnki[SDLK_QUESTION] = KC_QUESTION;
	impl->sdlKeyToAnki[SDLK_AT] = KC_AT;
	impl->sdlKeyToAnki[SDLK_LEFTBRACKET] = KC_LEFTBRACKET;
	impl->sdlKeyToAnki[SDLK_BACKSLASH] = KC_BACKSLASH;
	impl->sdlKeyToAnki[SDLK_RIGHTBRACKET] = KC_RIGHTBRACKET;
	impl->sdlKeyToAnki[SDLK_CARET] = KC_CARET;
	impl->sdlKeyToAnki[SDLK_UNDERSCORE] = KC_UNDERSCORE;
	impl->sdlKeyToAnki[SDLK_BACKQUOTE] = KC_BACKQUOTE;
	impl->sdlKeyToAnki[SDLK_a] = KC_A;
	impl->sdlKeyToAnki[SDLK_b] = KC_B;
	impl->sdlKeyToAnki[SDLK_c] = KC_C;
	impl->sdlKeyToAnki[SDLK_d] = KC_D;
	impl->sdlKeyToAnki[SDLK_e] = KC_E;
	impl->sdlKeyToAnki[SDLK_f] = KC_F;
	impl->sdlKeyToAnki[SDLK_g] = KC_G;
	impl->sdlKeyToAnki[SDLK_h] = KC_H;
	impl->sdlKeyToAnki[SDLK_i] = KC_I;
	impl->sdlKeyToAnki[SDLK_j] = KC_J;
	impl->sdlKeyToAnki[SDLK_k] = KC_K;
	impl->sdlKeyToAnki[SDLK_l] = KC_L;
	impl->sdlKeyToAnki[SDLK_m] = KC_M;
	impl->sdlKeyToAnki[SDLK_n] = KC_N;
	impl->sdlKeyToAnki[SDLK_o] = KC_O;
	impl->sdlKeyToAnki[SDLK_p] = KC_P;
	impl->sdlKeyToAnki[SDLK_q] = KC_Q;
	impl->sdlKeyToAnki[SDLK_r] = KC_R;
	impl->sdlKeyToAnki[SDLK_s] = KC_S;
	impl->sdlKeyToAnki[SDLK_y] = KC_T;
	impl->sdlKeyToAnki[SDLK_u] = KC_U;
	impl->sdlKeyToAnki[SDLK_v] = KC_V;
	impl->sdlKeyToAnki[SDLK_w] = KC_W;
	impl->sdlKeyToAnki[SDLK_x] = KC_X;
	impl->sdlKeyToAnki[SDLK_y] = KC_Y;
	impl->sdlKeyToAnki[SDLK_z] = KC_Z;
	impl->sdlKeyToAnki[SDLK_CAPSLOCK] = KC_CAPSLOCK;
	impl->sdlKeyToAnki[SDLK_F1] = KC_F1;
	impl->sdlKeyToAnki[SDLK_F2] = KC_F2;
	impl->sdlKeyToAnki[SDLK_F3] = KC_F3;
	impl->sdlKeyToAnki[SDLK_F4] = KC_F4;
	impl->sdlKeyToAnki[SDLK_F5] = KC_F5;
	impl->sdlKeyToAnki[SDLK_F6] = KC_F6;
	impl->sdlKeyToAnki[SDLK_F7] = KC_F7;
	impl->sdlKeyToAnki[SDLK_F8] = KC_F8;
	impl->sdlKeyToAnki[SDLK_F9] = KC_F9;
	impl->sdlKeyToAnki[SDLK_F10] = KC_F10;
	impl->sdlKeyToAnki[SDLK_F11] = KC_F11;
	impl->sdlKeyToAnki[SDLK_F12] = KC_F12;
	impl->sdlKeyToAnki[SDLK_PRINTSCREEN] = KC_PRINTSCREEN;
	impl->sdlKeyToAnki[SDLK_SCROLLLOCK] = KC_SCROLLLOCK;
	impl->sdlKeyToAnki[SDLK_PAUSE] = KC_PAUSE;
	impl->sdlKeyToAnki[SDLK_INSERT] = KC_INSERT;
	impl->sdlKeyToAnki[SDLK_HOME] = KC_HOME;
	impl->sdlKeyToAnki[SDLK_PAGEUP] = KC_PAGEUP;
	impl->sdlKeyToAnki[SDLK_DELETE] = KC_DELETE;
	impl->sdlKeyToAnki[SDLK_END] = KC_END;
	impl->sdlKeyToAnki[SDLK_PAGEDOWN] = KC_PAGEDOWN;
	impl->sdlKeyToAnki[SDLK_RIGHT] = KC_RIGHT;
	impl->sdlKeyToAnki[SDLK_LEFT] = KC_LEFT;
	impl->sdlKeyToAnki[SDLK_DOWN] = KC_DOWN;
	impl->sdlKeyToAnki[SDLK_UP] = KC_UP;
	impl->sdlKeyToAnki[SDLK_NUMLOCKCLEAR] = KC_NUMLOCKCLEAR;
	impl->sdlKeyToAnki[SDLK_KP_DIVIDE] = KC_KP_DIVIDE;
	impl->sdlKeyToAnki[SDLK_KP_MULTIPLY] = KC_KP_MULTIPLY;
	impl->sdlKeyToAnki[SDLK_KP_MINUS] = KC_KP_MINUS;
	impl->sdlKeyToAnki[SDLK_KP_PLUS] = KC_KP_PLUS;
	impl->sdlKeyToAnki[SDLK_KP_ENTER] = KC_KP_ENTER;
	impl->sdlKeyToAnki[SDLK_KP_1] = KC_KP_1;
	impl->sdlKeyToAnki[SDLK_KP_2] = KC_KP_2;
	impl->sdlKeyToAnki[SDLK_KP_3] = KC_KP_3;
	impl->sdlKeyToAnki[SDLK_KP_4] = KC_KP_4;
	impl->sdlKeyToAnki[SDLK_KP_5] = KC_KP_5;
	impl->sdlKeyToAnki[SDLK_KP_6] = KC_KP_6;
	impl->sdlKeyToAnki[SDLK_KP_7] = KC_KP_7;
	impl->sdlKeyToAnki[SDLK_KP_8] = KC_KP_8;
	impl->sdlKeyToAnki[SDLK_KP_9] = KC_KP_9;
	impl->sdlKeyToAnki[SDLK_KP_0] = KC_KP_0;
	impl->sdlKeyToAnki[SDLK_KP_PERIOD] = KC_KP_PERIOD;
	impl->sdlKeyToAnki[SDLK_APPLICATION] = KC_APPLICATION;
	impl->sdlKeyToAnki[SDLK_POWER] = KC_POWER;
	impl->sdlKeyToAnki[SDLK_KP_EQUALS] = KC_KP_EQUALS;
	impl->sdlKeyToAnki[SDLK_F13] = KC_F13;
	impl->sdlKeyToAnki[SDLK_F14] = KC_F14;
	impl->sdlKeyToAnki[SDLK_F15] = KC_F15;
	impl->sdlKeyToAnki[SDLK_F16] = KC_F16;
	impl->sdlKeyToAnki[SDLK_F17] = KC_F17;
	impl->sdlKeyToAnki[SDLK_F18] = KC_F18;
	impl->sdlKeyToAnki[SDLK_F19] = KC_F19;
	impl->sdlKeyToAnki[SDLK_F20] = KC_F20;
	impl->sdlKeyToAnki[SDLK_F21] = KC_F21;
	impl->sdlKeyToAnki[SDLK_F22] = KC_F22;
	impl->sdlKeyToAnki[SDLK_F23] = KC_F23;
	impl->sdlKeyToAnki[SDLK_F24] = KC_F24;
	impl->sdlKeyToAnki[SDLK_EXECUTE] = KC_EXECUTE;
	impl->sdlKeyToAnki[SDLK_HELP] = KC_HELP;
	impl->sdlKeyToAnki[SDLK_MENU] = KC_MENU;
	impl->sdlKeyToAnki[SDLK_SELECT] = KC_SELECT;
	impl->sdlKeyToAnki[SDLK_STOP] = KC_STOP;
	impl->sdlKeyToAnki[SDLK_AGAIN] = KC_AGAIN;
	impl->sdlKeyToAnki[SDLK_UNDO] = KC_UNDO;
	impl->sdlKeyToAnki[SDLK_CUT] = KC_CUT;
	impl->sdlKeyToAnki[SDLK_COPY] = KC_COPY;
	impl->sdlKeyToAnki[SDLK_PASTE] = KC_PASTE;
	impl->sdlKeyToAnki[SDLK_FIND] = KC_FIND;
	impl->sdlKeyToAnki[SDLK_MUTE] = KC_MUTE;
	impl->sdlKeyToAnki[SDLK_VOLUMEUP] = KC_VOLUMEUP;
	impl->sdlKeyToAnki[SDLK_VOLUMEDOWN] = KC_VOLUMEDOWN;
	impl->sdlKeyToAnki[SDLK_KP_COMMA] = KC_KP_COMMA;
	impl->sdlKeyToAnki[SDLK_KP_EQUALSAS400] = KC_KP_EQUALSAS400;
	impl->sdlKeyToAnki[SDLK_ALTERASE] = KC_ALTERASE;
	impl->sdlKeyToAnki[SDLK_SYSREQ] = KC_SYSREQ;
	impl->sdlKeyToAnki[SDLK_CANCEL] = KC_CANCEL;
	impl->sdlKeyToAnki[SDLK_CLEAR] = KC_CLEAR;
	impl->sdlKeyToAnki[SDLK_PRIOR] = KC_PRIOR;
	impl->sdlKeyToAnki[SDLK_RETURN2] = KC_RETURN2;
	impl->sdlKeyToAnki[SDLK_SEPARATOR] = KC_SEPARATOR;
	impl->sdlKeyToAnki[SDLK_OUT] = KC_OUT;
	impl->sdlKeyToAnki[SDLK_OPER] = KC_OPER;
	impl->sdlKeyToAnki[SDLK_CLEARAGAIN] = KC_CLEARAGAIN;
	impl->sdlKeyToAnki[SDLK_CRSEL] = KC_CRSEL;
	impl->sdlKeyToAnki[SDLK_EXSEL] = KC_EXSEL;
	impl->sdlKeyToAnki[SDLK_KP_00] = KC_KP_00;
	impl->sdlKeyToAnki[SDLK_KP_000] = KC_KP_000;
	impl->sdlKeyToAnki[SDLK_THOUSANDSSEPARATOR] = KC_THOUSANDSSEPARATOR;
	impl->sdlKeyToAnki[SDLK_DECIMALSEPARATOR] = KC_DECIMALSEPARATOR;
	impl->sdlKeyToAnki[SDLK_CURRENCYUNIT] = KC_CURRENCYUNIT;
	impl->sdlKeyToAnki[SDLK_CURRENCYSUBUNIT] = KC_CURRENCYSUBUNIT;
	impl->sdlKeyToAnki[SDLK_KP_LEFTPAREN] = KC_KP_LEFTPAREN;
	impl->sdlKeyToAnki[SDLK_KP_RIGHTPAREN] = KC_KP_RIGHTPAREN;
	impl->sdlKeyToAnki[SDLK_KP_LEFTBRACE] = KC_KP_LEFTBRACE;
	impl->sdlKeyToAnki[SDLK_KP_RIGHTBRACE] = KC_KP_RIGHTBRACE;
	impl->sdlKeyToAnki[SDLK_KP_TAB] = KC_KP_TAB;
	impl->sdlKeyToAnki[SDLK_KP_BACKSPACE] = KC_KP_BACKSPACE;
	impl->sdlKeyToAnki[SDLK_KP_A] = KC_KP_A;
	impl->sdlKeyToAnki[SDLK_KP_B] = KC_KP_B;
	impl->sdlKeyToAnki[SDLK_KP_C] = KC_KP_C;
	impl->sdlKeyToAnki[SDLK_KP_D] = KC_KP_D;
	impl->sdlKeyToAnki[SDLK_KP_E] = KC_KP_E;
	impl->sdlKeyToAnki[SDLK_KP_F] = KC_KP_F;
	impl->sdlKeyToAnki[SDLK_KP_XOR] = KC_KP_XOR;
	impl->sdlKeyToAnki[SDLK_KP_POWER] = KC_KP_POWER;
	impl->sdlKeyToAnki[SDLK_KP_PERCENT] = KC_KP_PERCENT;
	impl->sdlKeyToAnki[SDLK_KP_LESS] = KC_KP_LESS;
	impl->sdlKeyToAnki[SDLK_KP_GREATER] = KC_KP_GREATER;
	impl->sdlKeyToAnki[SDLK_KP_AMPERSAND] = KC_KP_AMPERSAND;
	impl->sdlKeyToAnki[SDLK_KP_DBLAMPERSAND] = KC_KP_DBLAMPERSAND;
	impl->sdlKeyToAnki[SDLK_KP_VERTICALBAR] = KC_KP_VERTICALBAR;
	impl->sdlKeyToAnki[SDLK_KP_DBLVERTICALBAR] = KC_KP_DBLVERTICALBAR;
	impl->sdlKeyToAnki[SDLK_KP_COLON] = KC_KP_COLON;
	impl->sdlKeyToAnki[SDLK_KP_HASH] = KC_KP_HASH;
	impl->sdlKeyToAnki[SDLK_KP_SPACE] = KC_KP_SPACE;
	impl->sdlKeyToAnki[SDLK_KP_AT] = KC_KP_AT;
	impl->sdlKeyToAnki[SDLK_KP_EXCLAM] = KC_KP_EXCLAM;
	impl->sdlKeyToAnki[SDLK_KP_MEMSTORE] = KC_KP_MEMSTORE;
	impl->sdlKeyToAnki[SDLK_KP_MEMRECALL] = KC_KP_MEMRECALL;
	impl->sdlKeyToAnki[SDLK_KP_MEMCLEAR] = KC_KP_MEMCLEAR;
	impl->sdlKeyToAnki[SDLK_KP_MEMADD] = KC_KP_MEMADD;
	impl->sdlKeyToAnki[SDLK_KP_MEMSUBTRACT] = KC_KP_MEMSUBTRACT;
	impl->sdlKeyToAnki[SDLK_KP_MEMMULTIPLY] = KC_KP_MEMMULTIPLY;
	impl->sdlKeyToAnki[SDLK_KP_MEMDIVIDE] = KC_KP_MEMDIVIDE;
	impl->sdlKeyToAnki[SDLK_KP_PLUSMINUS] = KC_KP_PLUSMINUS;
	impl->sdlKeyToAnki[SDLK_KP_CLEAR] = KC_KP_CLEAR;
	impl->sdlKeyToAnki[SDLK_KP_CLEARENTRY] = KC_KP_CLEARENTRY;
	impl->sdlKeyToAnki[SDLK_KP_BINARY] = KC_KP_BINARY;
	impl->sdlKeyToAnki[SDLK_KP_OCTAL] = KC_KP_OCTAL;
	impl->sdlKeyToAnki[SDLK_KP_DECIMAL] = KC_KP_DECIMAL;
	impl->sdlKeyToAnki[SDLK_KP_HEXADECIMAL] = KC_KP_HEXADECIMAL;
	impl->sdlKeyToAnki[SDLK_LCTRL] = KC_LCTRL;
	impl->sdlKeyToAnki[SDLK_LSHIFT] = KC_LSHIFT;
	impl->sdlKeyToAnki[SDLK_LALT] = KC_LALT;
	impl->sdlKeyToAnki[SDLK_LGUI] = KC_LGUI;
	impl->sdlKeyToAnki[SDLK_RCTRL] = KC_RCTRL;
	impl->sdlKeyToAnki[SDLK_RSHIFT] = KC_RSHIFT;
	impl->sdlKeyToAnki[SDLK_RALT] = KC_RALT;
	impl->sdlKeyToAnki[SDLK_RGUI] = KC_RGUI;
	impl->sdlKeyToAnki[SDLK_MODE] = KC_MODE;
	impl->sdlKeyToAnki[SDLK_AUDIONEXT] = KC_AUDIONEXT;
	impl->sdlKeyToAnki[SDLK_AUDIOPREV] = KC_AUDIOPREV;
	impl->sdlKeyToAnki[SDLK_AUDIOSTOP] = KC_AUDIOSTOP;
	impl->sdlKeyToAnki[SDLK_AUDIOPLAY] = KC_AUDIOPLAY;
	impl->sdlKeyToAnki[SDLK_AUDIOMUTE] = KC_AUDIOMUTE;
	impl->sdlKeyToAnki[SDLK_MEDIASELECT] = KC_MEDIASELECT;
	impl->sdlKeyToAnki[SDLK_WWW] = KC_WWW;
	impl->sdlKeyToAnki[SDLK_MAIL] = KC_MAIL;
	impl->sdlKeyToAnki[SDLK_CALCULATOR] = KC_CALCULATOR;
	impl->sdlKeyToAnki[SDLK_COMPUTER] = KC_COMPUTER;
	impl->sdlKeyToAnki[SDLK_AC_SEARCH] = KC_AC_SEARCH;
	impl->sdlKeyToAnki[SDLK_AC_HOME] = KC_AC_HOME;
	impl->sdlKeyToAnki[SDLK_AC_BACK] = KC_AC_BACK;
	impl->sdlKeyToAnki[SDLK_AC_FORWARD] = KC_AC_FORWARD;
	impl->sdlKeyToAnki[SDLK_AC_STOP] = KC_AC_STOP;
	impl->sdlKeyToAnki[SDLK_AC_REFRESH] = KC_AC_REFRESH;
	impl->sdlKeyToAnki[SDLK_AC_BOOKMARKS] = KC_AC_BOOKMARKS;
	impl->sdlKeyToAnki[SDLK_BRIGHTNESSDOWN] = KC_BRIGHTNESSDOWN;
	impl->sdlKeyToAnki[SDLK_BRIGHTNESSUP] = KC_BRIGHTNESSUP;
	impl->sdlKeyToAnki[SDLK_DISPLAYSWITCH] = KC_DISPLAYSWITCH;
	impl->sdlKeyToAnki[SDLK_KBDILLUMTOGGLE] = KC_KBDILLUMTOGGLE;
	impl->sdlKeyToAnki[SDLK_KBDILLUMDOWN] = KC_KBDILLUMDOWN;
	impl->sdlKeyToAnki[SDLK_KBDILLUMUP] = KC_KBDILLUMUP;
	impl->sdlKeyToAnki[SDLK_EJECT] = KC_EJECT;
	impl->sdlKeyToAnki[SDLK_SLEEP] = KC_SLEEP;

	// Call once to clear first events
	handleEvents();
}

//==============================================================================
void Input::handleEvents()
{
	ANKI_ASSERT(nativeWindow != nullptr);

	// add the times a key is being pressed
	for(auto& k : keys)
	{
		if(k)
		{
			++k;
		}
	}
	for(auto& k : mouseBtns)
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
			akkey = impl->sdlKeyToAnki[event.key.keysym.sym];
			keys[akkey] = 1;
			break;
		case SDL_KEYUP:
			akkey = impl->sdlKeyToAnki[event.key.keysym.sym];
			keys[akkey] = 0;
			break;
		case SDL_MOUSEBUTTONDOWN:
			//XXX
			break;
		case SDL_MOUSEBUTTONUP:
			//XXX
			break;
		case SDL_MOUSEMOTION:
			mousePosNdc.x() = 
				(F32)event.button.x / nativeWindow->getWidth() * 2.0 - 1.0;
			mousePosNdc.y() = 
				-((F32)event.button.y / nativeWindow->getHeight() * 2.0 - 1.0);
			break;
		case SDL_QUIT:
			addEvent(WINDOW_CLOSED_EVENT);
			break;
		}
	} // end while events

	// Lock mouse
	if(lockCurs)
	{
		moveCursor(Vec2(0.0));
	}
}

//==============================================================================
void Input::moveCursor(const Vec2& pos)
{
	if(pos != mousePosNdc)
	{
		SDL_WarpMouseInWindow(
			nativeWindow->getNative().window,
			nativeWindow->getWidth() * (pos.x() * 0.5 + 0.5), 
			nativeWindow->getHeight() * (pos.y() * 0.5 + 0.5));
	}
}

//==============================================================================
void Input::hideCursor(Bool hide)
{
	SDL_ShowCursor(!hide);
}

} // end namespace anki
