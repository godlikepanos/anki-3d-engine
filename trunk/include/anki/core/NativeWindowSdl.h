#ifndef ANKI_CORE_NATIVE_WINDOW_SDL_H
#define ANKI_CORE_NATIVE_WINDOW_SDL_H

#include "anki/core/NativeWindow.h"
#include <SDL.h>

namespace anki {

/// Native window implementation for SDL
struct NativeWindowImpl
{
	SDL_Window* window = nullptr;
	SDL_GLContext ctx = 0;
};

} // end namespace anki

#endif

