#include "anki/input/Input.h"
#include "anki/core/Logger.h"
#include "anki/input/InputX11.h"
#if ANKI_WINDOW_BACKEND_GLXX11
#	include "anki/core/NativeWindowGlxX11.h"
#elif ANKI_WINDOW_BACKEND_EGLX11
#	include "anki/core/NativeWindowEglX11.h"
#else
#	error "See file"
#endif
#include <X11/XKBlib.h>
#include <cstring>

#define DEBUG_EVENTS 0

#if DEBUG_EVENTS
#	define DBG_LOGI(x_) ANKI_LOGI(x_)
#else
#	define DBG_LOGI(x_) ((void)0)
#endif

// Undef stupid X11 defines
#undef Bool

namespace anki {

//==============================================================================
struct X11KeyCodeToAnki
{
	U32 x;
	KeyCode ak;
};

static const X11KeyCodeToAnki x2a[] = {
	{XK_Return, KC_RETURN},
	{XK_Escape, KC_ESCAPE},
	{XK_BackSpace, KC_BACKSPACE},
	{XK_Tab, KC_TAB},
	{XK_space, KC_SPACE},
	{XK_exclam, KC_EXCLAIM},
	{XK_quotedbl, KC_QUOTEDBL},
	{XK_numbersign, KC_HASH},
	{XK_percent, KC_PERCENT},
	{XK_dollar, KC_DOLLAR},
	{XK_ampersand, KC_AMPERSAND},
	{XK_apostrophe, KC_QUOTE},
	{XK_parenleft, KC_LPAREN},
	{XK_parenright, KC_RPAREN},
	{XK_asterisk, KC_ASTERISK},
	{XK_plus, KC_PLUS},
	{XK_comma, KC_COMMA},
	{XK_minus, KC_MINUS},
	{XK_period, KC_PERIOD},
	{XK_slash, KC_SLASH},
	{XK_0, KC_0},
	{XK_1, KC_1},
	{XK_2, KC_2},
	{XK_3, KC_3},
	{XK_4, KC_4},
	{XK_5, KC_5},
	{XK_6, KC_6},
	{XK_7, KC_7},
	{XK_8, KC_8},
	{XK_9, KC_9},
	{XK_colon, KC_COLON},
	{XK_semicolon, KC_SEMICOLON},
	{XK_less, KC_LESS},
	{XK_equal, KC_EQUALS},
	{XK_greater, KC_GREATER},
	{XK_question, KC_QUESTION},
	{XK_at, KC_AT},
	{XK_bracketleft, KC_LBRACKET},
	{XK_backslash, KC_BACKSLASH},
	{XK_bracketright, KC_RBRACKET},
	/*{XK_caret, KC_CARET},*/
	{XK_underscore, KC_UNDERSCORE},
	{XK_grave, KC_BACKQUOTE},
	{XK_a, KC_A},
	{XK_b, KC_B},
	{XK_c, KC_C},
	{XK_d, KC_D},
	{XK_e, KC_E},
	{XK_f, KC_F},
	{XK_g, KC_G},
	{XK_h, KC_H},
	{XK_i, KC_I},
	{XK_j, KC_J},
	{XK_k, KC_K},
	{XK_l, KC_L},
	{XK_m, KC_M},
	{XK_n, KC_N},
	{XK_o, KC_O},
	{XK_p, KC_P},
	{XK_q, KC_Q},
	{XK_r, KC_R},
	{XK_s, KC_S},
	{XK_t, KC_T},
	{XK_u, KC_U},
	{XK_v, KC_V},
	{XK_w, KC_W},
	{XK_x, KC_X},
	{XK_y, KC_Y},
	{XK_z, KC_Z},
	{XK_Delete, KC_DELETE},

	{XK_Up,  KC_UP},
	{XK_Down, KC_DOWN},
	{XK_Left, KC_LEFT},
	{XK_Right, KC_RIGHT}
};

#define XKEYCODE2ANKI(k_) nativeKeyToAnki[k_ & 0xFF]

//==============================================================================
static Bool eventsPending(Display* display)
{
	XFlush(display);
	if(XEventsQueued(display, QueuedAlready)) 
	{
		return true;
	}

	static struct timeval zero_time;
	int x11_fd;
	fd_set fdset;

	x11_fd = ConnectionNumber(display);
	FD_ZERO(&fdset);
	FD_SET(x11_fd, &fdset);
	if(select(x11_fd + 1, &fdset, NULL, NULL, &zero_time) == 1) 
	{
		return XPending(display);
	}

	return false;
}

//==============================================================================
Input::~Input()
{}

//==============================================================================
void Input::init(NativeWindow* nativeWindow_)
{
	ANKI_ASSERT(nativeWindow == nullptr);
	nativeWindow = nativeWindow_;
	
	NativeWindowImpl& nwin = nativeWindow->getNative();

	XSelectInput(nwin.xDisplay, nwin.xWindow, 
		ExposureMask | ButtonPressMask | KeyPressMask | KeyReleaseMask 
		| PointerMotionMask | FocusChangeMask | EnterWindowMask 
		| LeaveWindowMask);

	memset(&nativeKeyToAnki[0], sizeof(nativeKeyToAnki), KC_UNKNOWN);
	for(const X11KeyCodeToAnki& a : x2a)
	{
		// Convert X11 keycode to something else
		U32 somethingElse = a.x & 0xFF;

		nativeKeyToAnki[somethingElse] = a.ak;
	}

	reset();

	// Init native
	impl.reset(new InputImpl);
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

	NativeWindowImpl& win = nativeWindow->getNative();
	Display* disp = win.xDisplay;
	//DBG_LOGI("----------------------");
	while(eventsPending(disp))
	{
		XEvent event;
		::KeyCode keycode = 0;
		KeySym keysym;
		XNextEvent(disp, &event);
skipXNextEvent:

		switch(event.type)
		{
		case KeyPress:
			keysym = XLookupKeysym(&event.xkey, 0);
			keycode = event.xkey.keycode;
			keys[XKEYCODE2ANKI(keysym)] = 1;
			DBG_LOGI("Key pressed: 0x" << std::hex << (U32)keysym);
			break;
		case KeyRelease:
			keycode = event.xkey.keycode;
			keysym = XLookupKeysym(&event.xkey, 0);
			if(eventsPending(disp))
			{
				XEvent event1;
				XNextEvent(disp, &event1);

				if(event1.type == KeyPress && event1.xkey.keycode == keycode)
				{
					// Repeat
					//DBG_LOGI("Key autorepeat: 0x" << std::hex << (U32)keysym);
					//++keys[XKEYCODE2ANKI(keysym)];
				}
				else
				{
					DBG_LOGI("Key released: 0x" << std::hex << (U32)keysym);
					keys[XKEYCODE2ANKI(keysym)] = 0;
					event = event1;
					goto skipXNextEvent;
				}
			}
			else
			{
				DBG_LOGI("Key released #2: 0x" << std::hex << (U32)keysym);
				keys[XKEYCODE2ANKI(keysym)] = 0;
			}
			break;
		case MotionNotify:
			mousePosNdc.x() = 
				(F32)event.xmotion.x / nativeWindow->getWidth() * 2.0 - 1.0;
			mousePosNdc.y() = 
				-((F32)event.xmotion.y / nativeWindow->getHeight() * 2.0 - 1.0);

			DBG_LOGI("MotionNotify: " << event.xmotion.x << " " 
				<< event.xmotion.y << " (" << mousePosNdc << ")");
			break;
		case EnterNotify:
			DBG_LOGI("EnterNotify: ");
			break;
		case LeaveNotify:
			DBG_LOGI("LeaveNotify: ");
			break;
		default:
			DBG_LOGI("Unknown X event");
			break;
		}
	}
}

//==============================================================================
void Input::moveMouse(const Vec2& pos)
{
	DBG_LOGI("Moving to: " << pos.x() << " " << pos.y());

	if(pos != mousePosNdc)
	{
		NativeWindowImpl& nwi = nativeWindow->getNative();

		XWarpPointer(nwi.xDisplay, None, nwi.xWindow, 0, 0, 0, 0, 
			(F32)nativeWindow->getWidth() * (pos.x() / 2.0 + 0.5), 
			(F32)nativeWindow->getHeight() * (-pos.y() / 2.0 + 0.5));

		XSync(nwi.xDisplay, False);
	}
}

//==============================================================================
void Input::hideCursor(Bool hide)
{
	Display* dis = nativeWindow->getNative().xDisplay;
	Window win = nativeWindow->getNative().xWindow;

	if(impl->emptyCursor == None)
	{
		char data[1] = {0};
		XColor color;
		Pixmap pixmap;

		color.red = color.green = color.blue = 0;
		pixmap = XCreateBitmapFromData(dis, DefaultRootWindow(dis), data, 1, 1);
		if(pixmap) 
		{
			impl->emptyCursor = XCreatePixmapCursor(dis, pixmap, pixmap,
				&color, &color, 0, 0);
			XFreePixmap(dis, pixmap);
		}	
	}

	XDefineCursor(dis, win, impl->emptyCursor);
	XFlush(dis);
}

} // end namespace anki
